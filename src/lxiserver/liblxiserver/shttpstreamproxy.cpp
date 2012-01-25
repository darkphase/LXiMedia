/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

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
  inline SetSourceEvent(QIODevice *source, SHttpEngine *owner)
    : QEvent(setSourceEventType), source(source), owner(owner)
  {
  }

public:
  QIODevice * const source;
  SHttpEngine * const owner;
};

class SHttpStreamProxy::AddSocketEvent : public QEvent
{
public:
  inline AddSocketEvent(QIODevice *socket, SHttpEngine *owner)
    : QEvent(addSocketEventType), socket(socket), owner(owner)
  {
  }

public:
  QIODevice * const socket;
  SHttpEngine * const owner;
};

struct SHttpStreamProxy::Data
{
  struct Socket
  {
    bool                        isConnected(void) const;
    void                        close(void);

    QIODevice                 * socket;
    QAbstractSocket           * aSocket;
    QLocalSocket              * lSocket;
    SHttpEngine               * owner;
    qint64                      readPos;
  };

  inline Data(QObject *parent) : mutex(QMutex::NonRecursive), socketTimer(parent) { }

  QMutex                        mutex;
  QThread                     * mainThread;
  Socket                        source;
  bool                          sourceFinished;
  QVector<Socket>               sockets;
  QTimer                        socketTimer;

  QByteArray                    cache;
  QTime                         cacheTimer;
  bool                          caching;
};

const QEvent::Type  SHttpStreamProxy::setSourceEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SHttpStreamProxy::addSocketEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SHttpStreamProxy::disconnectEventType = QEvent::Type(QEvent::registerEventType());

const int           SHttpStreamProxy::inBufferSize  = 262144;
const int           SHttpStreamProxy::outBufferSize = 33554432;

SHttpStreamProxy::SHttpStreamProxy(void)
  : QThread(),
    d(new Data(this))
{
  d->mainThread = thread();
  d->source.socket = NULL;
  d->source.aSocket = NULL;
  d->source.lSocket = NULL;
  d->source.owner = NULL;
  d->source.readPos = 0;
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

bool SHttpStreamProxy::setSource(QIODevice *source, SHttpEngine *owner)
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
    qApp->postEvent(this, new SetSourceEvent(source, owner));

    return true;
  }

  qApp->postEvent(this, new QEvent(disconnectEventType));
  return false;
}

bool SHttpStreamProxy::addSocket(QIODevice *socket, SHttpEngine *owner)
{
  if (currentThread() != d->mainThread)
  {
    qCritical() <<
        "SHttpStreamProxy::addSocket() is not called from the correct thread, "
        "make sure Qt::DirectConnection is specified with connect if the slot "
        "is invoked through a signal.";
  }

  if (d->caching)
  {
    socket->setParent(NULL);
    socket->moveToThread(this);
    qApp->postEvent(this, new AddSocketEvent(socket, owner));

    return true;
  }

  return false;
}

void SHttpStreamProxy::run(void)
{
  d->cache.reserve(outBufferSize + (outBufferSize / 2));
  d->cacheTimer.start();

  QThread::exec();

  moveToThread(qApp->thread());
}

void SHttpStreamProxy::customEvent(QEvent *e)
{
  QMutexLocker l(&d->mutex);

  if (e->type() == setSourceEventType)
  {
    const SetSourceEvent * const event = static_cast<const SetSourceEvent *>(e);

    d->source.socket = event->source;
    d->source.aSocket = qobject_cast<QAbstractSocket *>(d->source.socket);
    d->source.lSocket = qobject_cast<QLocalSocket *>(d->source.socket);
    d->source.owner = event->owner;

    if (d->source.aSocket) d->source.aSocket->setReadBufferSize(inBufferSize * 2);
    if (d->source.lSocket) d->source.lSocket->setReadBufferSize(inBufferSize * 2);

    connect(d->source.socket, SIGNAL(readyRead()), SLOT(processData()));
    if (d->source.socket->metaObject()->indexOfSignal("disconnected()") >= 0)
      connect(d->source.socket, SIGNAL(disconnected()), SLOT(flushData()));

    d->socketTimer.start(250);
  }
  else if (e->type() == addSocketEventType)
  {
    const AddSocketEvent * const event = static_cast<const AddSocketEvent *>(e);

    Data::Socket s;
    s.socket = event->socket;
    s.aSocket = qobject_cast<QAbstractSocket *>(s.socket);
    s.lSocket = qobject_cast<QLocalSocket *>(s.socket);
    s.owner = event->owner;
    s.readPos = 0;

#if (defined(Q_OS_UNIX) || defined(Q_OS_WIN))
    if (s.aSocket)
    {
      const int optval = inBufferSize * 2;
      ::setsockopt(
            s.aSocket->socketDescriptor(), SOL_SOCKET, SO_SNDBUF,
            reinterpret_cast<const char *>(&optval), sizeof(optval));
    }
#endif

    connect(s.socket, SIGNAL(bytesWritten(qint64)), SLOT(processData()));

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
  if (d->source.socket)
  {
    disconnect(d->source.socket, SIGNAL(readyRead()), this, SLOT(processData()));
    if (d->source.socket->metaObject()->indexOfSignal("disconnected()") >= 0)
      disconnect(d->source.socket, SIGNAL(disconnected()), this, SLOT(flushData()));

    d->source.close();

    d->sourceFinished = true;
    d->socketTimer.stop();

    emit disconnected();
  }

  for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s=d->sockets.erase(s))
    s->socket->close();

  d->socketTimer.stop();

  quit();
}

void SHttpStreamProxy::processData(void)
{
  QMutexLocker l(&d->mutex);

  if (d->source.socket && !d->sourceFinished)
  {
    while (d->source.socket->bytesAvailable() > 0)
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
        s->close();
        s = d->sockets.erase(s);
      }

      if (minBuf <= (outBufferSize - inBufferSize))
      {
        const QByteArray buffer = d->source.socket->read(inBufferSize);

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
            }
            else if (!d->caching)
            {
              qDebug() << "SHttpStreamProxy: Disconnecting stalled socket" << s->socket;
              if (s->aSocket) s->aSocket->disconnectFromHost();
              if (s->lSocket) s->lSocket->disconnectFromServer();
            }
          }
        }

        if (d->caching)
        {
          if ((d->cache.size() + buffer.size()) <= d->cache.capacity())
          {
            d->cache.append(buffer);
            d->cacheTimer.start();
          }
          else
          {
            d->cache.clear();
            d->caching = false;
          }
        }
      }
      else
      {
        if (d->caching && (qAbs(d->cacheTimer.elapsed()) >= 5000))
        {
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
      s->close();
      s = d->sockets.erase(s);
    }
    else if (s->socket->bytesToWrite() == 0)
    {
      disconnect(s->socket, SIGNAL(bytesWritten(qint64)), this, SLOT(processData()));
      if (s->socket->metaObject()->indexOfSignal("disconnected()") >= 0)
        connect(s->socket, SIGNAL(disconnected()), s->socket, SLOT(deleteLater()));

      s->close();
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

  if (d->source.socket && !d->sourceFinished)
  {
    while ((d->source.socket->bytesAvailable() > 0) && !d->sockets.isEmpty())
    {
      const QByteArray buffer = d->source.socket->read(inBufferSize);

      for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); )
      if (s->isConnected() && (s->socket->bytesToWrite() <= (outBufferSize * 3)))
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
        s->close();
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

void SHttpStreamProxy::Data::Socket::close(void)
{
  if (socket)
  {
    if (owner)
      owner->closeSocket(socket);
    else
      socket->deleteLater();

    socket = NULL;
    aSocket = NULL;
    lSocket = NULL;
  }
}

} // End of namespace
