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
#endif

namespace LXiServer {

class SHttpServer::Socket : public QTcpSocket
{
public:
  explicit                      Socket(SHttpServer *parent);
  virtual                       ~Socket();

  virtual void                  close(void);

public:
  bool                          closing;

private:
  SHttpServer           * const parent;
};

class SHttpServer::Server : public QTcpServer
{
public:
  explicit                      Server(const QHostAddress &, quint16, SHttpServer *parent);

protected:
  virtual void                  incomingConnection(int);

private:
  SHttpServer            * const parent;
};

struct SHttpServer::Private
{
  QList<QHostAddress>           addresses;
  quint16                       port;
  QString                       serverUdn;
  QMultiMap<QString, Server *>  servers;
  int                           sockets;
};


SHttpServer::SHttpServer(const QString &protocol, const QUuid &serverUuid, QObject *parent)
  : SHttpServerEngine(protocol, parent),
    p(new Private())
{
  p->serverUdn = QString("uuid:" + serverUuid.toString()).replace("{", "").replace("}", "");
  p->sockets = 0;
}

SHttpServer::~SHttpServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SHttpServer::initialize(const QList<QHostAddress> &addresses, quint16 port)
{
  foreach (const QHostAddress &address, addresses)
    p->servers.insert(address.toString(), new Server(address, port, this));
}

void SHttpServer::close(void)
{
  foreach (Server *iface, p->servers)
    delete iface;

  p->servers.clear();
}

quint16 SHttpServer::serverPort(const QHostAddress &address) const
{
  QMultiMap<QString, Server *>::ConstIterator i = p->servers.find(address.toString());
  if (i != p->servers.end())
    return (*i)->serverPort();

  return 0;
}

const QString & SHttpServer::serverUdn(void) const
{
  return p->serverUdn;
}

void SHttpServer::closeSocket(QIODevice *socket, bool)
{
  if (!static_cast<Socket *>(socket)->closing)
  {
    static_cast<Socket *>(socket)->closing = true;
    new SocketCloseRequest(socket);
  }
}


SHttpServer::Socket::Socket(SHttpServer *parent)
  : QTcpSocket(),
    closing(false),
    parent(parent)
{
  if (parent->p->sockets++ == 0)
    emit parent->busy();
}

SHttpServer::Socket::~Socket()
{
  if (--parent->p->sockets == 0)
    emit parent->idle();
}

void SHttpServer::Socket::close(void)
{
  parent->closeSocket(this, false);
}


SHttpServer::Server::Server(const QHostAddress &interfaceAddr, quint16 port, SHttpServer *parent)
  : QTcpServer(),
    parent(parent)
{
  if (port > 0)
  if (QTcpServer::listen(interfaceAddr, port))
    return;

  if (!QTcpServer::listen(interfaceAddr))
    qWarning() << "Failed to bind interface" << interfaceAddr.toString();
}

void SHttpServer::Server::incomingConnection(int socketDescriptor)
{
  QTcpSocket * const socket = new Socket(parent);
  if (socket->setSocketDescriptor(socketDescriptor))
    (new HttpServerRequest(parent))->start(socket);
  else
    delete socket;
}

} // End of namespace
