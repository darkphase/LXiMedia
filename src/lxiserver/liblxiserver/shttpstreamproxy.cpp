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
#if defined(Q_OS_UNIX)
#include <sys/socket.h>
#elif defined(Q_OS_WIN)
#include <ws2tcpip.h>
#endif

namespace LXiServer {

class SHttpStreamProxy::SetSourceEvent : public QEvent
{
public:
  inline SetSourceEvent(QIODevice *source)
    : QEvent(setSourceEventType), source(source)
  {
  }

public:
  QIODevice * const source;
};

class SHttpStreamProxy::AddSocketEvent : public QEvent
{
public:
  inline AddSocketEvent(QIODevice *socket)
    : QEvent(addSocketEventType), socket(socket)
  {
  }

public:
  QIODevice * const socket;
};

struct SHttpStreamProxy::Data
{
  struct Socket
  {
    bool                        isConnected(void) const;

    QIODevice                 * socket;
    QAbstractSocket           * aSocket;
    QLocalSocket              * lSocket;
    qint64                      readPos;
  };

  inline Data(QObject *parent) : mutex(QMutex::NonRecursive), socketTimer(parent) { }

  QMutex                        mutex;
  QThread                     * mainThread;
  QIODevice                   * source;
  QAbstractSocket             * aSource;
  QLocalSocket                * lSource;
  bool                          sourceFinished;
  QVector<Socket>               sockets;
  QTimer                        socketTimer;

  bool                          caching;
  QByteArray                    cache;
  QTime                         cacheTimer;
  static const int              cacheTimeout = 30000;
  static const int              maxCacheSize = 134217728;
};

const QEvent::Type  SHttpStreamProxy::setSourceEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SHttpStreamProxy::addSocketEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SHttpStreamProxy::disconnectEventType = QEvent::Type(QEvent::registerEventType());

const int           SHttpStreamProxy::inBufferSize  = 262144;
const int           SHttpStreamProxy::outBufferSize = 2097152;

SHttpStreamProxy::SHttpStreamProxy(void)
  : QThread(),
    d(new Data(this))
{
  d->mainThread = thread();
  d->source = NULL;
  d->aSource = NULL;
  d->lSource = NULL;
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

bool SHttpStreamProxy::setSource(QIODevice *source)
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

  qApp->postEvent(this, new QEvent(disconnectEventType));
  return false;
}

bool SHttpStreamProxy::addSocket(QIODevice *socket)
{
  if (currentThread() != d->mainThread)
  {
    qCritical() <<
        "SHttpStreamProxy::addSocket() is not called from the correct thread, "
        "make sure Qt::DirectConnection is specified with connect if the slot "
        "is invoked through a signal.";
  }

  if (d->caching && ((d->source == NULL) || (qAbs(d->cacheTimer.elapsed()) < d->cacheTimeout)))
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
  d->cache.reserve(d->maxCacheSize);

  QThread::exec();

  moveToThread(qApp->thread());
}

void SHttpStreamProxy::customEvent(QEvent *e)
{
  QMutexLocker l(&d->mutex);

  if (e->type() == setSourceEventType)
  {
    const SetSourceEvent * const event = static_cast<const SetSourceEvent *>(e);

    d->source = event->source;
    d->aSource = qobject_cast<QAbstractSocket *>(d->source);
    d->lSource = qobject_cast<QLocalSocket *>(d->source);

    if (d->aSource) d->aSource->setReadBufferSize(inBufferSize);
    if (d->lSource) d->lSource->setReadBufferSize(inBufferSize);

    connect(d->source, SIGNAL(readyRead()), SLOT(processData()));
    if (d->source->metaObject()->indexOfSignal("disconnected()") >= 0)
      connect(d->source, SIGNAL(disconnected()), SLOT(flushData()));

    d->cacheTimer.start();
    d->socketTimer.start(250);
  }
  else if (e->type() == addSocketEventType)
  {
    const AddSocketEvent * const event = static_cast<const AddSocketEvent *>(e);

    Data::Socket s;
    s.socket = event->socket;
    s.aSocket = qobject_cast<QAbstractSocket *>(s.socket);
    s.lSocket = qobject_cast<QLocalSocket *>(s.socket);
    s.readPos = 0;

#if (defined(Q_OS_UNIX) || defined(Q_OS_WIN))
    if (s.aSocket)
    {
      const int optval = outBufferSize / 2;
      ::setsockopt(
            s.aSocket->socketDescriptor(), SOL_SOCKET, SO_SNDBUF,
            reinterpret_cast<const char *>(&optval), sizeof(optval));
    }
#endif

    connect(s.socket, SIGNAL(bytesWritten(qint64)), SLOT(processData()));

    d->cacheTimer.start();

    //qDebug() << "SHttpStreamProxy: Connected socket" << s.socket;
    d->sockets += s;
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
  if (d->source)
  {
    disconnect(d->source, SIGNAL(readyRead()), this, SLOT(processData()));
    if (d->source->metaObject()->indexOfSignal("disconnected()") >= 0)
      disconnect(d->source, SIGNAL(disconnected()), this, SLOT(flushData()));

    if (d->aSource && (d->aSource->state() != QAbstractSocket::UnconnectedState))
    {
      connect(d->aSource, SIGNAL(disconnected()), d->aSource, SLOT(deleteLater()));
      d->aSource->disconnectFromHost();
    }
    else if (d->lSource && (d->lSource->state() != QLocalSocket::UnconnectedState))
    {
      connect(d->lSource, SIGNAL(disconnected()), d->lSource, SLOT(deleteLater()));
      d->lSource->disconnectFromServer();
    }
    else
      d->source->deleteLater();

    d->source->moveToThread(qApp->thread());

    d->source = NULL;
    d->sourceFinished = true;
    d->socketTimer.stop();

    //qDebug() << "SHttpStreamProxy: All clients disconnected";
    emit disconnected();
  }

  for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s=d->sockets.erase(s))
  {
    if (s->aSocket && (s->aSocket->state() != QAbstractSocket::UnconnectedState))
    {
      disconnect(s->aSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(processData()));
      connect(s->aSocket, SIGNAL(disconnected()), s->aSocket, SLOT(deleteLater()));
      s->aSocket->disconnectFromHost();
    }
    else if (s->lSocket && (s->lSocket->state() != QLocalSocket::UnconnectedState))
    {
      disconnect(s->lSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(processData()));
      connect(s->lSocket, SIGNAL(disconnected()), s->lSocket, SLOT(deleteLater()));
      s->lSocket->disconnectFromServer();
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
      qint64 minBuf = outBufferSize, maxBuf = 0;
      for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); )
      if (s->isConnected())
      {
        if (s->readPos >= d->cache.size())
        {
          const qint64 bw = s->socket->bytesToWrite();
          minBuf = qMin(minBuf, bw);
          maxBuf = qMax(maxBuf, bw);
        }
        else if (s->socket->bytesToWrite() <= (outBufferSize - inBufferSize))
        {
          //qDebug() << "SHttpStreamProxy: Cached" << s->socket << s->readPos << d->cache.size();

          const QByteArray chunk = d->cache.mid(s->readPos, inBufferSize);
          if (s->socket->write(chunk) == chunk.size())
            s->readPos += chunk.size();
          else
            qDebug() << "SHttpStreamProxy: Error writing data to socket" << s->socket;
        }

        s++;
      }
      else
      {
        s->socket->deleteLater();
        s->socket->moveToThread(qApp->thread());
        s = d->sockets.erase(s);
      }

      if (minBuf <= (outBufferSize - inBufferSize))
      {
        const QByteArray buffer = d->source->read(inBufferSize);

        for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s++)
        if (s->isConnected())
        {
          if (s->readPos >= d->cache.size())
          {
            if (s->socket->bytesToWrite() <= (outBufferSize * 2))
            {
              if (s->socket->write(buffer) == buffer.size())
                s->readPos += buffer.size();
              else
                qDebug() << "SHttpStreamProxy: Error writing data to socket" << s->socket;

              //qDebug() << "SHttpStreamProxy: Direct" << s->socket << s->readPos << (d->cache.size() + buffer.size());
            }
            else if (!d->caching)
            {
              //qDebug() << "SHttpStreamProxy: Disconnecting stalled socket" << s->socket;
              if (s->aSocket) s->aSocket->disconnectFromHost();
              if (s->lSocket) s->lSocket->disconnectFromServer();
            }
          }
        }

        if (d->caching)
        {
          if ((qAbs(d->cacheTimer.elapsed()) < d->cacheTimeout) &&
              ((d->cache.size() + buffer.size()) <= d->maxCacheSize))
          {
            d->cache.append(buffer);
            //qDebug() << "SHttpStreamProxy: Append cache" << d->cache.size() << buffer.size();
          }
          else
          {
            //qDebug() << "SHttpStreamProxy: Flushed reconnect cache" << d->cache.size();
            d->cache.clear();
            d->caching = false;
          }
        }
      }
      else
      {
        if (d->caching && (qAbs(d->cacheTimer.elapsed()) >= d->cacheTimeout))
        {
          //qDebug() << "SHttpStreamProxy: Flushed reconnect cache" << d->cache.size();
          d->cache.clear();
          d->caching = false;
        }

        break;
      }
    }
  }
  else // No souce, flush data.
  {
    for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); )
    if (!s->isConnected())
    {
      s->socket->deleteLater();
      s->socket->moveToThread(qApp->thread());
      s = d->sockets.erase(s);
    }
    else if (s->socket->bytesToWrite() == 0)
    {
      disconnect(s->socket, SIGNAL(bytesWritten(qint64)), this, SLOT(processData()));
      if (s->socket->metaObject()->indexOfSignal("disconnected()") >= 0)
        connect(s->socket, SIGNAL(disconnected()), s->socket, SLOT(deleteLater()));

      if (s->aSocket) s->aSocket->disconnectFromHost();
      if (s->lSocket) s->lSocket->disconnectFromServer();

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
    while ((d->source->bytesAvailable() > 0) && !d->sockets.isEmpty())
    {
      const QByteArray buffer = d->source->read(inBufferSize);

      for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); )
      if (s->isConnected() && (s->socket->bytesToWrite() <= (outBufferSize * 2)))
      {
        if (s->readPos >= d->cache.size())
        {
          if (s->socket->write(buffer) == buffer.size())
            s->readPos += buffer.size();
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


bool SHttpStreamProxy::Data::Socket::isConnected(void) const
{
  if (aSocket)
    return aSocket->state() == QAbstractSocket::ConnectedState;
  else if (lSocket)
    return lSocket->state() == QLocalSocket::ConnectedState;
  else
    return true;
}

} // End of namespace
