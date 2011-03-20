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

#include "httpserver.h"

#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#endif

namespace LXiServer {

class HttpServer::Interface : public QTcpServer
{
public:
  explicit                      Interface(const QHostAddress &, quint16, HttpServer *parent);

protected:
  virtual void                  incomingConnection(int);

private:
  HttpServer            * const parent;
};

struct HttpServer::Private
{
  QList<QHostAddress>           addresses;
  quint16                       port;
  QString                       serverUdn;
  QMultiMap<QString, Interface *> interfaces;
};


HttpServer::HttpServer(const QString &protocol, const QUuid &serverUuid, QObject *parent)
  : HttpServerEngine(protocol, parent),
    p(new Private())
{
  p->serverUdn = QString("uuid:" + serverUuid.toString()).replace("{", "").replace("}", "");
}

HttpServer::~HttpServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void HttpServer::initialize(const QList<QHostAddress> &addresses, quint16 port)
{
  QWriteLocker l(lock());

  foreach (const QHostAddress &address, addresses)
    p->interfaces.insert(address.toString(), new Interface(address, port, this));
}

void HttpServer::close(void)
{
  QWriteLocker l(lock());

  foreach (Interface *iface, p->interfaces)
    delete iface;

  p->interfaces.clear();

  l.unlock();

  threadPool()->waitForDone();
}

quint16 HttpServer::serverPort(const QHostAddress &address) const
{
  QReadLocker l(lock());

  QMultiMap<QString, Interface *>::ConstIterator i = p->interfaces.find(address.toString());
  if (i != p->interfaces.end())
    return (*i)->serverPort();

  return 0;
}

const QString & HttpServer::serverUdn(void) const
{
  return p->serverUdn;
}

void HttpServer::closeSocket(QIODevice *device, int timeout)
{
  QTcpSocket * const socket = static_cast<QTcpSocket *>(device);

  if (socket->state() == QAbstractSocket::ConnectedState)
  {
    QTime timer;
    timer.start();

    socket->waitForBytesWritten(qMax(timeout - qAbs(timer.elapsed()), 0));
    socket->disconnectFromHost();
    if (socket->state() != QAbstractSocket::UnconnectedState)
      socket->waitForDisconnected(qMax(timeout - qAbs(timer.elapsed()), 0));
  }

  delete socket;
}

QIODevice * HttpServer::openSocket(quintptr socketDescriptor, int)
{
  QTcpSocket * const socket = new QTcpSocket();
  if (socket->setSocketDescriptor(socketDescriptor))
    return socket;

  delete socket;
  return NULL;
}

void HttpServer::closeSocket(QIODevice *socket, bool, int timeout)
{
  closeSocket(socket, timeout);
}


HttpServer::Interface::Interface(const QHostAddress &interfaceAddr, quint16 port, HttpServer *parent)
  : QTcpServer(),
    parent(parent)
{
  if (port > 0)
  if (QTcpServer::listen(interfaceAddr, port))
    return;

  if (!QTcpServer::listen(interfaceAddr))
    qWarning() << "Failed to bind interface" << interfaceAddr.toString();
}

void HttpServer::Interface::incomingConnection(int socketDescriptor)
{
  if (!parent->handleConnection(socketDescriptor))
  {
    QTcpSocket socket;
    if (socket.setSocketDescriptor(socketDescriptor))
      socket.abort();
  }
}

} // End of namespace
