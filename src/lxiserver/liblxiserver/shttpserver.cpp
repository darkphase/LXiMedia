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

#include "shttpserver.h"
#include "lxiserverprivate.h"
#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace LXiServer {

struct SHttpServer::Data
{
  QList<QHostAddress>           addresses;
  quint16                       port;
  QString                       serverUdn;
  QMultiMap<QString, Server *>  servers;
  int                           openSockets;
};

class SHttpServer::Socket : public QTcpSocket
{
public:
  explicit Socket(SHttpServer *parent)
    : QTcpSocket(parent), parent(parent)
  {
    if (parent->d->openSockets++ == 0)
      emit parent->busy();
  }

  virtual ~Socket()
  {
    if (parent)
    if (parent->d)
    if (--parent->d->openSockets == 0)
      emit parent->idle();
  }

private:
  const QPointer<SHttpServer>   parent;
};

class SHttpServer::Server : public QTcpServer
{
public:
  explicit Server(SHttpServer *parent)
    : QTcpServer(parent), parent(parent)
  {
  }

protected:
  virtual void incomingConnection(int socketDescriptor)
  {
    Q_ASSERT(QThread::currentThread() == thread());

    if (parent)
    {
      QTcpSocket * const socket = new Socket(parent);
      if (socket->setSocketDescriptor(socketDescriptor))
      {
#ifdef Q_OS_WIN
        // This is needed to ensure the socket isn't kept open by any child
        // processes.
        HANDLE handle = (HANDLE)socket->socketDescriptor();
        ::SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0);
#endif
        (new HttpServerRequest(parent))->start(socket);
      }
      else
        delete socket;
    }
  }

private:
  const QPointer<SHttpServer>   parent;
};

/*! Creates an instance of the HTTP server, the specified protocol and
    serverUuid are used in the HTTP response messages.
 */
SHttpServer::SHttpServer(const QString &protocol, const QUuid &serverUuid, QObject *parent)
  : SHttpServerEngine(protocol, parent),
    d(new Data())
{
  d->serverUdn = QString("uuid:" + serverUuid.toString()).replace("{", "").replace("}", "");
  d->openSockets = 0;
}

SHttpServer::~SHttpServer()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

/*! Initializes the HTTP server by binding the specified interfaces and port.
 */
void SHttpServer::initialize(const QList<QHostAddress> &addresses, quint16 port)
{
  foreach (const QHostAddress &address, addresses)
  {
    Server * const server = new Server(this);

    if (port > 0)
    if (server->listen(address, port))
    {
      d->servers.insert(address.toString(), server);
      continue;
    }

    if (server->listen(address))
      d->servers.insert(address.toString(), server);
    else
      delete server;
  }
}

/*! Uninitializes the HTTP server by releasing the bound ports.
 */
void SHttpServer::close(void)
{
  foreach (Server *server, d->servers)
  {
    server->close();
    delete server;
  }

  d->servers.clear();
}

/*! Returns the bound port number for the specified interface.
 */
quint16 SHttpServer::serverPort(const QHostAddress &address) const
{
  QMultiMap<QString, Server *>::ConstIterator i = d->servers.find(address.toString());
  if (i != d->servers.end())
    return (*i)->serverPort();

  return 0;
}

/*! Returns the UDN for the server.
 */
const QString & SHttpServer::serverUdn(void) const
{
  return d->serverUdn;
}

} // End of namespace
