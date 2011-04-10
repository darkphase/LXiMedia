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
    if (parent)
    {
      QTcpSocket * const socket = new Socket(parent);
      if (socket->setSocketDescriptor(socketDescriptor))
        (new HttpServerRequest(parent))->start(socket);
      else
        delete socket;
    }
  }

private:
  const QPointer<SHttpServer>   parent;
};

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

void SHttpServer::close(void)
{
  foreach (Server *server, d->servers)
  {
    server->close();
    delete server;
  }

  d->servers.clear();
}

quint16 SHttpServer::serverPort(const QHostAddress &address) const
{
  QMultiMap<QString, Server *>::ConstIterator i = d->servers.find(address.toString());
  if (i != d->servers.end())
    return (*i)->serverPort();

  return 0;
}

const QString & SHttpServer::serverUdn(void) const
{
  return d->serverUdn;
}

void SHttpServer::closeSocket(QAbstractSocket *socket, bool)
{
  new SocketCloseRequest(this, socket);
}

} // End of namespace
