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

#include "shttpproxy.h"
#include "shttpengine.h"

namespace LXiServer {

struct SHttpProxy::Data
{
  struct Socket
  {
    SHttpEngine::SocketPtr      socket;
    bool                        sendCache;
  };

  inline Data(void) : mutex(QMutex::Recursive) { }

  QMutex                        mutex;
  SHttpEngine::SocketPtr        source;
  QVector<Socket>               sockets;
  QTimer                        socketTimer;

  bool                          caching;
  QByteArray                    cache;
};

const int SHttpProxy::outBufferSize = 2097152;

SHttpProxy::SHttpProxy(QObject *parent)
  : QObject(parent),
    d(new Data())
{
  d->caching = true;
}

SHttpProxy::~SHttpProxy()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SHttpProxy::setSource(QIODevice *source)
{
  d->source = source;
  d->source->moveToThread(thread());
  d->source->setParent(this);

  connect(d->source, SIGNAL(readyRead()), SLOT(processData()));

  connect(&d->socketTimer, SIGNAL(timeout()), SLOT(processData()));
  d->socketTimer.start(250);

  return true;
}

bool SHttpProxy::addSocket(QIODevice *socket)
{
  QMutexLocker l(&d->mutex);

  if (d->caching || d->sockets.isEmpty())
  {
    Data::Socket s;
    s.socket = socket;
    s.sendCache = true;

    if (s.socket)
    {
      s.socket->moveToThread(thread());
      s.socket->setParent(this);

      d->sockets += s;

      processData();

      connect(s.socket, SIGNAL(bytesWritten(qint64)), SLOT(processData()));

      return true;
    }
    else
      qFatal("Assigned incompatible socket: %s", socket->metaObject()->className());
  }

  return false;
}

bool SHttpProxy::isConnected(void) const
{
  QMutexLocker l(&d->mutex);

  if (!d->source.isConnected())
  {
    foreach (const Data::Socket &s, d->sockets)
    if (s.socket.isConnected())
    if (s.socket->bytesToWrite() > 0)
      return true;

    return false;
  }

  if (d->caching)
    return true;

  foreach (const Data::Socket &s, d->sockets)
  if (s.socket.isConnected())
    return true;

  return false;
}
void SHttpProxy::processData(void)
{
  QMutexLocker l(&d->mutex);

  while (d->source->bytesAvailable() > 0)
  {
    for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s++)
    if (s->socket.isConnected())
    if (s->socket->bytesToWrite() >= outBufferSize)
      return; // Blocked, wait a bit.

    const QByteArray buffer = d->source->read(65536);

    if (d->caching)
    {
      for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s++)
      if (s->sendCache && s->socket.isConnected())
      {
        //qDebug() << "Reuse" << d->cache.size();
        s->socket->write(d->cache);
        s->sendCache = false;
      }

      if ((d->cache.size() + buffer.size()) < (outBufferSize * 4))
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
    if (s->socket.isConnected())
    {
      if (!s->sendCache && s->socket.isConnected())
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
      s = d->sockets.erase(s);
  }

  if (!isConnected())
    emit disconnected();
}

} // End of namespace
