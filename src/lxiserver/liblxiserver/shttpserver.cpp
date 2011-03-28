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

#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#endif

namespace LXiServer {

class SHttpServer::Interface : public QTcpServer
{
public:
  explicit                      Interface(const QHostAddress &, quint16, SHttpServer *parent);

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
  QMultiMap<QString, Interface *> interfaces;
};


SHttpServer::SHttpServer(const QString &protocol, const QUuid &serverUuid, QObject *parent)
  : SHttpServerEngine(protocol, parent),
    p(new Private())
{
  p->serverUdn = QString("uuid:" + serverUuid.toString()).replace("{", "").replace("}", "");
}

SHttpServer::~SHttpServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SHttpServer::initialize(const QList<QHostAddress> &addresses, quint16 port)
{
  QWriteLocker l(lock());

  foreach (const QHostAddress &address, addresses)
    p->interfaces.insert(address.toString(), new Interface(address, port, this));
}

void SHttpServer::close(void)
{
  QWriteLocker l(lock());

  foreach (Interface *iface, p->interfaces)
    delete iface;

  p->interfaces.clear();

  l.unlock();

  threadPool()->waitForDone();
}

quint16 SHttpServer::serverPort(const QHostAddress &address) const
{
  QReadLocker l(lock());

  QMultiMap<QString, Interface *>::ConstIterator i = p->interfaces.find(address.toString());
  if (i != p->interfaces.end())
    return (*i)->serverPort();

  return 0;
}

const QString & SHttpServer::serverUdn(void) const
{
  return p->serverUdn;
}

void SHttpServer::closeSocket(QIODevice *device, int timeout)
{
  QTcpSocket * const socket = static_cast<QTcpSocket *>(device);

  if (socket->state() == QAbstractSocket::ConnectedState)
  {
    QTime timer;
    timer.start();

    while (socket->bytesToWrite() > 0)
    if (!socket->waitForBytesWritten(qMax(timeout - qAbs(timer.elapsed()), 0)))
      break;

    socket->disconnectFromHost();
    if (socket->state() != QAbstractSocket::UnconnectedState)
      socket->waitForDisconnected(qMax(timeout - qAbs(timer.elapsed()), 0));
  }

  delete socket;
}

QIODevice * SHttpServer::openSocket(quintptr socketDescriptor, int)
{
  QTcpSocket * const socket = new QTcpSocket();
  if (socket->setSocketDescriptor(socketDescriptor))
    return socket;

  delete socket;
  return NULL;
}

void SHttpServer::closeSocket(QIODevice *socket, bool, int timeout)
{
  closeSocket(socket, timeout);
}


SHttpServer::Interface::Interface(const QHostAddress &interfaceAddr, quint16 port, SHttpServer *parent)
  : QTcpServer(),
    parent(parent)
{
  if (port > 0)
  if (QTcpServer::listen(interfaceAddr, port))
    return;

  if (!QTcpServer::listen(interfaceAddr))
    qWarning() << "Failed to bind interface" << interfaceAddr.toString();
}

void SHttpServer::Interface::incomingConnection(int socketDescriptor)
{
  if (!parent->handleConnection(socketDescriptor))
  {
    QTcpSocket socket;
    if (socket.setSocketDescriptor(socketDescriptor))
      socket.abort();
  }
}

} // End of namespace
