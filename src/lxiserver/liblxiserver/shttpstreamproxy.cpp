/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
 *   code@admiraal.dds.nl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "shttpstreamproxy.h"
#include "shttpengine.h"

namespace LXiServer {

class SHttpStreamProxy::SetSourceEvent : public QEvent
{
public:
  inline SetSourceEvent(QAbstractSocket *source)
    : QEvent(setSourceEventType), source(source)
  {
  }

public:
  QAbstractSocket * const source;
};

class SHttpStreamProxy::AddSocketEvent : public QEvent
{
public:
  inline AddSocketEvent(QAbstractSocket *socket)
    : QEvent(addSocketEventType), socket(socket)
  {
  }

public:
  QAbstractSocket * const socket;
};

struct SHttpStreamProxy::Data
{
  struct Socket
  {
    QAbstractSocket           * socket;
    bool                        sendCache;
    bool                        firstData;
  };

  inline Data(void) : mutex(QMutex::Recursive) { }

  QMutex                        mutex;
  QThread                     * mainThread;
  QAbstractSocket             * source;
  QTime                         sourceTimer;
  bool                          sourceFinished;
  QVector<Socket>               sockets;
  QTimer                        socketTimer;

  bool                          caching;
  QByteArray                    cache;
};

const QEvent::Type  SHttpStreamProxy::setSourceEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SHttpStreamProxy::addSocketEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SHttpStreamProxy::disconnectEventType = QEvent::Type(QEvent::registerEventType());

const int           SHttpStreamProxy::inBufferSize  = 262144;
const int           SHttpStreamProxy::outBufferSize = 8388608;

SHttpStreamProxy::SHttpStreamProxy(void)
  : QThread(),
    d(new Data())
{
  d->mainThread = thread();
  d->source = NULL;
  d->sourceFinished = false;
  d->caching = true;

  connect(&d->socketTimer, SIGNAL(timeout()), SLOT(processData()));

  moveToThread(this);
  start();
}

SHttpStreamProxy::~SHttpStreamProxy()
{
  // The event will simply be discarded if the thread isn't running anymore.
  qApp->postEvent(this, new QEvent(disconnectEventType));
  wait();

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SHttpStreamProxy::isConnected(void) const
{
  QMutexLocker l(&d->mutex);

  if (!d->sourceFinished)
    return true;
  else
    return !d->sockets.isEmpty();
}

bool SHttpStreamProxy::setSource(QAbstractSocket *source)
{
  if (currentThread() != d->mainThread)
  {
    qCritical() <<
        "SHttpStreamProxy::setSource() is not called from the correct thread, "
        "make sure Qt::DirectConnection is specified with connect if the slot "
        "is invoked through a signal.";
  }

  if (source)
  {
    source->setParent(NULL);
    source->moveToThread(this);
    qApp->postEvent(this, new SetSourceEvent(source));

    return true;
  }

  disconnectAllSockets();
  return false;
}

bool SHttpStreamProxy::addSocket(QAbstractSocket *socket)
{
  if (currentThread() != d->mainThread)
  {
    qCritical() <<
        "SHttpStreamProxy::addSocket() is not called from the correct thread, "
        "make sure Qt::DirectConnection is specified with connect if the slot "
        "is invoked through a signal.";
  }

  if (d->caching && ((d->source == NULL) || (qAbs(d->sourceTimer.elapsed()) < 10000)))
  {
    socket->setParent(NULL);
    socket->moveToThread(this);
    qApp->postEvent(this, new AddSocketEvent(socket));

    return true;
  }

  return false;
}

void SHttpStreamProxy::run(void)
{
  QThread::exec();
}

void SHttpStreamProxy::customEvent(QEvent *e)
{
  QMutexLocker l(&d->mutex);

  if (e->type() == setSourceEventType)
  {
    const SetSourceEvent * const event = static_cast<const SetSourceEvent *>(e);

    d->source = event->source;
    d->source->setReadBufferSize(inBufferSize * 2);
    connect(d->source, SIGNAL(readyRead()), SLOT(processData()));
    connect(d->source, SIGNAL(disconnected()), SLOT(flushData()));

    d->sourceTimer.start();
    d->socketTimer.start(250);
  }
  else if (e->type() == addSocketEventType)
  {
    const AddSocketEvent * const event = static_cast<const AddSocketEvent *>(e);

    Data::Socket s;
    s.socket = event->socket;
    s.sendCache = true;
    s.firstData = true;

    d->sockets += s;
    connect(s.socket, SIGNAL(bytesWritten(qint64)), SLOT(processData()));
  }
  else if (e->type() == disconnectEventType)
  {
    disconnectAllSockets();
  }
  else
    QThread::customEvent(e);
}

void SHttpStreamProxy::disconnectAllSockets(void)
{
  QMutexLocker l(&d->mutex);

  if (d->source)
  {
    disconnect(d->source, SIGNAL(readyRead()), this, SLOT(processData()));
    disconnect(d->source, SIGNAL(disconnected()), this, SLOT(flushData()));

    if (d->source->state() != QAbstractSocket::UnconnectedState)
    {
      connect(d->source, SIGNAL(disconnected()), d->source, SLOT(deleteLater()));
      d->source->disconnectFromHost();
    }
    else
      d->source->deleteLater();

    d->source->moveToThread(qApp->thread());

    d->source = NULL;

    emit disconnected();
  }

  for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s=d->sockets.erase(s))
  {
    if (s->socket->state() != QAbstractSocket::UnconnectedState)
    {
      disconnect(s->socket, SIGNAL(bytesWritten(qint64)), this, SLOT(processData()));
      connect(s->socket, SIGNAL(disconnected()), s->socket, SLOT(deleteLater()));
      s->socket->disconnectFromHost();
    }
    else
      s->socket->deleteLater();

    s->socket->moveToThread(qApp->thread());
  }

  d->socketTimer.stop();

  quit();
}

void SHttpStreamProxy::processData(void)
{
  QMutexLocker l(&d->mutex);

  if (d->source && !d->sourceFinished)
  {
    while (d->source->bytesAvailable() > 0)
    {
      for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s++)
      if (s->socket->state() == QAbstractSocket::ConnectedState)
      if (s->socket->bytesToWrite() >= outBufferSize)
        return; // Blocked, wait a bit.

      const QByteArray buffer = d->source->read(inBufferSize);

      if (d->caching)
      {
        for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s++)
        if (s->sendCache && (s->socket->state() == QAbstractSocket::ConnectedState))
        {
          //qDebug() << "Reuse" << d->cache.size();
          s->socket->write(d->cache);
          s->sendCache = false;
        }

        if ((d->cache.size() + buffer.size()) < (outBufferSize * 2))
        {
          d->cache.append(buffer);
        }
        else
        {
          d->cache.clear();
          d->caching = false;
        }
      }

      for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); )
      if (s->socket->state() == QAbstractSocket::ConnectedState)
      {
        if (!s->sendCache)
          s->socket->write(buffer);

        if (s->firstData)
        {
          s->socket->flush();
          s->firstData = false;
        }

        s++;
      }
      else
      {
        s->socket->deleteLater();
        s->socket->moveToThread(qApp->thread());
        s = d->sockets.erase(s);
      }
    }
  }
  else // No souce, flush data.
  {
    for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); )
    if (s->socket->state() != QAbstractSocket::ConnectedState)
    {
      s->socket->deleteLater();
      s->socket->moveToThread(qApp->thread());
      s = d->sockets.erase(s);
    }
    else if (s->socket->bytesToWrite() == 0)
    {
      disconnect(s->socket, SIGNAL(bytesWritten(qint64)), this, SLOT(processData()));
      connect(s->socket, SIGNAL(disconnected()), s->socket, SLOT(deleteLater()));
      s->socket->disconnectFromHost();
      s->socket->moveToThread(qApp->thread());
      s = d->sockets.erase(s);
    }
    else
      s++;
  }

  if (d->sockets.isEmpty() && !d->caching)
    disconnectAllSockets();
}

void SHttpStreamProxy::flushData(void)
{
  QMutexLocker l(&d->mutex);

  if (d->source && !d->sourceFinished)
  {
    while (d->source->bytesAvailable() > 0)
    {
      const QByteArray buffer = d->source->read(inBufferSize);

      for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); )
      if (s->socket->state() == QAbstractSocket::ConnectedState)
      {
        if (!s->sendCache)
        for (qint64 i=0; i<buffer.size(); )
        {
          const qint64 r = s->socket->write(buffer.data() + i, buffer.size() - i);
          if (r > 0)
            i += r;
          else
            break;
        }

        s++;
      }
      else
      {
        s->socket->deleteLater();
        s->socket->moveToThread(qApp->thread());
        s = d->sockets.erase(s);
      }
    }

    d->sourceFinished = true;
    d->caching = false;

    if (d->sockets.isEmpty())
      disconnectAllSockets();
  }
}

} // End of namespace
