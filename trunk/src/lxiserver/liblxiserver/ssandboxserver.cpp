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

class SSandboxServer::Socket : public QLocalSocket
{
public:
  explicit                      Socket(SSandboxServer *parent);
  virtual                       ~Socket();

  virtual void                  close(void);

public:
  bool                          closing;

private:
  SSandboxServer        * const parent;
};

class SSandboxServer::Server : public QLocalServer
{
public:
  explicit                      Server(const QString &, SSandboxServer *parent);

protected:
  virtual void                  incomingConnection(quintptr);

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

void SSandboxServer::initialize(const QString &name, const QString &mode)
{
  p->server = new Server(name, this);

  std::cerr << "##READY" << std::endl;

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

void SSandboxServer::closeSocket(QIODevice *socket, bool)
{
  if (!static_cast<Socket *>(socket)->closing)
  {
    static_cast<Socket *>(socket)->closing = true;
    new SocketCloseRequest(socket);
  }
}


SSandboxServer::Socket::Socket(SSandboxServer *parent)
  : QLocalSocket(),
    closing(false),
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

void SSandboxServer::Socket::close(void)
{
  parent->closeSocket(this, false);
}


SSandboxServer::Server::Server(const QString &name, SSandboxServer *parent)
  : QLocalServer(),
    parent(parent)
{
  if (!name.isEmpty())
  {
    QLocalServer::removeServer(name);

    if (QLocalServer::listen(name))
      return;
  }

  qWarning() << "Failed to bind local socket" << name;
}

void SSandboxServer::Server::incomingConnection(quintptr socketDescriptor)
{
  QLocalSocket * const socket = new Socket(parent);
  if (socket->setSocketDescriptor(socketDescriptor))
    (new HttpServerRequest(parent))->start(socket);
  else
    delete socket;
}

} // End of namespace
