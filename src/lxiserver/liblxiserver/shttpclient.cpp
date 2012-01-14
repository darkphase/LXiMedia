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

#include "shttpclient.h"
#include "lxiserverprivate.h"
#include <QtNetwork>
#if defined(Q_OS_WIN)
# include <windows.h>
#endif

namespace LXiServer {

struct SHttpClient::Data
{
  struct Request
  {
    inline Request(const QString &host, const QByteArray &message, QObject *receiver, const char *slot, Qt::ConnectionType connectionType)
      : host(host), message(message), receiver(receiver), slot(slot), connectionType(connectionType)
    {
    }

    QString                     host;
    QByteArray                  message;
    QObject                   * receiver;
    const char                * slot;
    Qt::ConnectionType          connectionType;
  };

  QList<Request>                requests;

#if defined(Q_OS_WIN)
  static const int              maxSocketCount = 8;
#else
  static const int              maxSocketCount = 32;
#endif
  QHash<QIODevice *, QString>   socketHost;
  QMultiHash<QString, QIODevice *> socketPool;
  QTimer                        socketPoolTimer;
};

SHttpClient::SHttpClient(QObject *parent)
  : SHttpClientEngine(parent),
    d(new Data())
{
  d->socketPoolTimer.setSingleShot(true);
  connect(&d->socketPoolTimer, SIGNAL(timeout()), SLOT(clearSocketPool()));
}

SHttpClient::~SHttpClient()
{
  clearSocketPool();

  Q_ASSERT(d->socketHost.isEmpty());

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SHttpClient::openRequest(const RequestMessage &message, QObject *receiver, const char *slot, Qt::ConnectionType connectionType)
{
  if (QThread::currentThread() != thread())
    qFatal("SHttpClient::openRequest() should be invoked from the thread "
           "that owns the SHttpClient object.");

  d->requests.append(Data::Request(message.host(), message, receiver, slot, connectionType));
  openRequest();
}

void SHttpClient::closeSocket(QIODevice *socket)
{
  if (QThread::currentThread() == thread())
  {
    SHttpClientEngine::closeSocket(socket);

    if (socket)
    {
      d->socketHost.remove(socket);

      openRequest();
    }
  }
  else
    SHttpClientEngine::closeSocket(socket); // Will call closeSocket() again from the correct thread.
}

void SHttpClient::reuseSocket(QIODevice *socket)
{
  QHash<QIODevice *, QString>::ConstIterator i = d->socketHost.find(socket);
  if (i != d->socketHost.end())
  {
    d->socketPool.insert(*i, socket);
    d->socketPoolTimer.start(30000);
  }
  else
    closeSocket(socket);

  openRequest();
}

QIODevice * SHttpClient::openSocket(const QString &host)
{
  d->socketPoolTimer.start(30000);

  QMultiHash<QString, QIODevice *>::Iterator i = d->socketPool.find(host);
  if (i != d->socketPool.end())
  {
    QTcpSocket * const socket = static_cast<QTcpSocket *>(*i);
    d->socketPool.erase(i);

    if (socket->state() == QTcpSocket::ConnectedState)
      return socket;
    else
      closeSocket(socket);
  }

  while (d->socketHost.count() >= d->maxSocketCount)
  {
    QMultiHash<QString, QIODevice *>::Iterator i = d->socketPool.begin();
    if (i != d->socketPool.end())
    {
      closeSocket(*i);
      d->socketPool.erase(i);
    }
    else
      break;
  }

  if (d->socketHost.count() < d->maxSocketCount)
  {
    QString hostname;
    quint16 port = 80;
    if (splitHost(host, hostname, port))
    {
      QTcpSocket * const socket = new QTcpSocket(this);
      socket->connectToHost(hostname, port);

#ifdef Q_OS_WIN
      // This is needed to ensure the socket isn't kept open by any child processes.
      ::SetHandleInformation((HANDLE)socket->socketDescriptor(), HANDLE_FLAG_INHERIT, 0);
#endif

      d->socketHost.insert(socket, host);

      return socket;
    }
  }

  return NULL;
}

void SHttpClient::openRequest(void)
{
  while (!d->requests.isEmpty())
  {
    const Data::Request request = d->requests.takeFirst();

    QString hostname;
    quint16 port = 80;
    if (splitHost(request.host, hostname, port))
    {
      QTcpSocket * const socket = static_cast<QTcpSocket *>(openSocket(request.host));
      if (socket)
      {
        HttpSocketRequest * const socketRequest =
            new HttpSocketRequest(this, socket, port, request.message, __FILE__, __LINE__);

        if (request.receiver)
          connect(socketRequest, SIGNAL(connected(QIODevice *, SHttpEngine *)), request.receiver, request.slot, request.connectionType);
        else
          connect(socketRequest, SIGNAL(connected(QIODevice *, SHttpEngine *)), SLOT(closeSocket(QIODevice *)));
      }
      else // No more free sockets.
      {
        d->requests.prepend(request);
        break;
      }
    }
  }
}

void SHttpClient::clearSocketPool(void)
{
  for (QMultiHash<QString, QIODevice *>::Iterator i = d->socketPool.begin();
       i != d->socketPool.end();
       i = d->socketPool.erase(i))
  {
    closeSocket(*i);
  }
}

} // End of namespace
