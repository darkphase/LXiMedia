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

#include "ssandboxserver.h"
#include "lxiserverprivate.h"
#include <QtNetwork>
#include <iostream>
#if defined(Q_OS_UNIX)
#include <unistd.h>
#include <sched.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#ifndef PROCESS_MODE_BACKGROUND_BEGIN
#define PROCESS_MODE_BACKGROUND_BEGIN 0x00100000
#endif
#endif

namespace LXiServer {

class SSandboxServer::Socket : public QTcpSocket
{
public:
  explicit                      Socket(SSandboxServer *parent);
  virtual                       ~Socket();

private:
  SSandboxServer        * const parent;
};

class SSandboxServer::Server : public QTcpServer
{
public:
  explicit                      Server(SSandboxServer *parent);

protected:
  virtual void                  incomingConnection(int);

private:
  SSandboxServer         * const parent;
};

struct SSandboxServer::Private
{
  Server                      * server;
  int                           sockets;
};

SSandboxServer::SSandboxServer(QObject *parent)
  : SHttpServerEngine("Sandbox/1.0", parent),
    p(new Private())
{
  p->server = NULL;
  p->sockets = 0;
}

SSandboxServer::~SSandboxServer()
{
  delete p->server;
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SSandboxServer::initialize(const QString &mode)
{
  p->server = new Server(this);

  std::cerr << "##READY "
      << p->server->serverAddress().toString().toAscii().data() << " "
      << p->server->serverPort() << std::endl;

  // This is performed after initialization to prevent priority inversion with
  // the process that is waiting for this one to start.
  if (mode == "nice")
  {
#if defined(Q_OS_UNIX)
    ::sched_param param = { 0 };
    ::sched_setscheduler(::getpid(), SCHED_BATCH, &param);

    ::nice(15);
#elif defined(Q_OS_WIN)
    ::SetPriorityClass(::GetCurrentProcess(), IDLE_PRIORITY_CLASS);

    ::SetPriorityClass(::GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN);
#endif
  }
}

void SSandboxServer::close(void)
{
  delete p->server;
  p->server = NULL;

  std::cerr << "##STOP" << std::endl;
}

void SSandboxServer::closeSocket(QAbstractSocket *socket, bool)
{
  new SocketCloseRequest(socket);
}


SSandboxServer::Socket::Socket(SSandboxServer *parent)
  : QTcpSocket(),
    parent(parent)
{
  if (parent->p->sockets++ == 0)
    emit parent->busy();
}

SSandboxServer::Socket::~Socket()
{
  if (--parent->p->sockets == 0)
    emit parent->idle();
}


SSandboxServer::Server::Server(SSandboxServer *parent)
  : QTcpServer(),
    parent(parent)
{
  // First try IPv6, then IPv4
  if (!QTcpServer::listen(QHostAddress::LocalHostIPv6))
  if (!QTcpServer::listen(QHostAddress::LocalHost))
    qWarning() << "SSandboxServer Failed to bind localhost interface";
}

void SSandboxServer::Server::incomingConnection(int socketDescriptor)
{
  QTcpSocket * const socket = new Socket(parent);
  if (socket->setSocketDescriptor(socketDescriptor))
    (new HttpServerRequest(parent))->start(socket);
  else
    delete socket;
}

} // End of namespace
