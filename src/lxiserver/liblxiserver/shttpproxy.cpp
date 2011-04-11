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
    QAbstractSocket           * socket;
    bool                        sendCache;
  };

  QAbstractSocket             * source;
  bool                          sourceFinished;
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
  d->sourceFinished = false;
  d->caching = true;
}

SHttpProxy::~SHttpProxy()
{
  disconnect();

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SHttpProxy::isConnected(void) const
{
  if (d->sourceFinished || (d->source->state() != QAbstractSocket::ConnectedState))
  {
    d->sourceFinished = true;
    return false;
  }

  if (d->caching)
    return true;

  foreach (const Data::Socket &s, d->sockets)
  if (s.socket->state() == QAbstractSocket::ConnectedState)
    return true;

  return false;
}

bool SHttpProxy::setSource(QAbstractSocket *source)
{
  d->source = source;

  connect(d->source, SIGNAL(readyRead()), SLOT(processData()));
  connect(d->source, SIGNAL(disconnected()), SLOT(flushData()));

  connect(&d->socketTimer, SIGNAL(timeout()), SLOT(processData()));
  d->socketTimer.start(250);

  return true;
}

bool SHttpProxy::addSocket(QAbstractSocket *socket)
{
  if (d->caching || d->sockets.isEmpty())
  {
    Data::Socket s;
    s.socket = socket;
    s.sendCache = true;

    d->sockets += s;
    connect(s.socket, SIGNAL(bytesWritten(qint64)), SLOT(processData()));

    return true;
  }

  return false;
}

void SHttpProxy::disconnectAllSockets(void)
{
  disconnect(d->source, SIGNAL(readyRead()), this, SLOT(processData()));
  disconnect(d->source, SIGNAL(disconnected()), this, SLOT(flushData()));

  if (d->source->state() != QAbstractSocket::UnconnectedState)
  {
    connect(d->source, SIGNAL(disconnected()), d->source, SLOT(deleteLater()));
    QTimer::singleShot(30000, d->source, SLOT(deleteLater()));
    d->source->disconnectFromHost();
  }
  else
    d->source->deleteLater();

  d->source = NULL;


  for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s=d->sockets.erase(s))
  if (s->socket->state() != QAbstractSocket::UnconnectedState)
  {
    disconnect(s->socket, SIGNAL(bytesWritten(qint64)), this, SLOT(processData()));
    connect(s->socket, SIGNAL(disconnected()), s->socket, SLOT(deleteLater()));
    QTimer::singleShot(30000, s->socket, SLOT(deleteLater()));
    s->socket->disconnectFromHost();
  }
  else
    s->socket->deleteLater();


  if (!d->sourceFinished)
  {
    emit disconnected();
    d->sourceFinished = true;
  }
}

void SHttpProxy::processData(void)
{
  if (d->source && !d->sourceFinished)
  {
    while (d->source->bytesAvailable() > 0)
    {
      for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s++)
      if (s->socket->state() == QAbstractSocket::ConnectedState)
      if (s->socket->bytesToWrite() >= outBufferSize)
        return; // Blocked, wait a bit.

      const QByteArray buffer = d->source->read(65536);

      if (d->caching)
      {
        for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s++)
        if (s->sendCache && (s->socket->state() == QAbstractSocket::ConnectedState))
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
        s = d->sockets.erase(s);
      }
    }

    if (!isConnected())
      disconnectAllSockets();
  }
}

void SHttpProxy::flushData(void)
{
  if (d->source && !d->sourceFinished)
  {
    while (d->source->bytesAvailable() > 0)
    {
      const QByteArray buffer = d->source->read(65536);

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
        s = d->sockets.erase(s);
      }
    }

    disconnectAllSockets();
  }
}

} // End of namespace
