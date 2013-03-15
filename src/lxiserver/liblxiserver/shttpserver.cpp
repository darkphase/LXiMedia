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
  quint16                       defaultPort;
  QString                       serverUdn;
  QMultiMap<QString, QPointer<QTcpServer> >  servers;
  int                           openConnections;
};

/*! Creates an instance of the HTTP server, the specified protocol and
    serverUuid are used in the HTTP response messages.
 */
SHttpServer::SHttpServer(const QString &protocol, const QUuid &serverUuid, QObject *parent)
  : SHttpServerEngine(protocol, parent),
    d(new Data())
{
  d->defaultPort = 0;
  d->serverUdn = QString("uuid:" + serverUuid.toString()).replace("{", "").replace("}", "");
  d->openConnections = 0;
}

SHttpServer::~SHttpServer()
{
  close();

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

/*! Initializes the HTTP server.
 */
void SHttpServer::initialize(quint16 port)
{
  d->defaultPort = port;
}

/*! Uninitializes the HTTP server by releasing the bound ports.
 */
void SHttpServer::close(void)
{
  foreach (QTcpServer *server, d->servers)
  {
    server->close();
    server->deleteLater();
  }

  d->servers.clear();
}

/*! binds thespecified interfac and port.
 */
bool SHttpServer::bind(const QHostAddress &address)
{
  QTcpServer * const server = new QTcpServer(this);

  connect(server, SIGNAL(newConnection()), SLOT(newConnection()));

  if (d->defaultPort > 0)
  if (server->listen(address, d->defaultPort))
  {
    d->servers.insert(address.toString(), server);
    return true;
  }

  if (server->listen(address))
  {
    d->servers.insert(address.toString(), server);
    return true;
  }

  delete server;
  return false;
}

/*! releases the bound interface.
 */
void SHttpServer::release(const QHostAddress &address)
{
  forever
  {
    QMultiMap<QString, QPointer<QTcpServer> >::Iterator i = d->servers.find(address.toString());
    if (i != d->servers.end())
    {
      (*i)->close();
      (*i)->deleteLater();
      d->servers.erase(i);
    }
    else
      break;
  }
}

/*! Returns the default port number as provided to initialize() and reset().
 */
quint16 SHttpServer::defaultPort(void) const
{
  return d->defaultPort;
}

/*! Returns the bound port number for the specified interface.
 */
quint16 SHttpServer::serverPort(const QHostAddress &address) const
{
  QMultiMap<QString, QPointer<QTcpServer> >::ConstIterator i = d->servers.find(address.toString());
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

void SHttpServer::newConnection(void)
{
  foreach (QTcpServer *server, d->servers)
  while (server->hasPendingConnections())
  {
    QTcpSocket * const socket = server->nextPendingConnection();
    if (socket)
    {
#ifdef Q_OS_WIN
      // This is needed to ensure the socket isn't kept open by any child
      // processes.
      ::SetHandleInformation((HANDLE)socket->socketDescriptor(), HANDLE_FLAG_INHERIT, 0);
#endif

      connect(socket, SIGNAL(destroyed()), SLOT(closedConnection()));
      if (d->openConnections++ == 0)
        emit busy();

      (new HttpServerRequest(this, server->serverPort(), __FILE__, __LINE__))->start(socket);
    }
  }
}

void SHttpServer::closedConnection(void)
{
  if (--d->openConnections == 0)
    emit idle();
}

} // End of namespace
