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

struct SSandboxServer::Data
{
  QTcpServer                  * server;
  int                           openSockets;
  QQueue<int>                   pendingSockets;
};

class SSandboxServer::Socket : public QTcpSocket
{
public:
  explicit Socket(SSandboxServer *parent)
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
  const QPointer<SSandboxServer> parent;
};

class SSandboxServer::Server : public QTcpServer
{
public:
  explicit Server(SSandboxServer *parent)
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
  const QPointer<SSandboxServer> parent;
};

SSandboxServer::SSandboxServer(QObject *parent)
  : SHttpServerEngine("Sandbox/1.0", parent),
    d(new Data())
{
  d->server = NULL;
  d->openSockets = 0;
}

SSandboxServer::~SSandboxServer()
{
  delete d->server;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SSandboxServer::initialize(const QString &mode)
{
  d->server = new Server(this);

  // First try IPv6, then IPv4
  if (!d->server->listen(QHostAddress::LocalHostIPv6))
  if (!d->server->listen(QHostAddress::LocalHost))
  {
    qWarning() << "SSandboxServer Failed to bind localhost interface";
    return false;
  }

  std::cerr << "##READY "
      << d->server->serverAddress().toString().toAscii().data() << " "
      << d->server->serverPort() << std::endl;

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

  return true;
}

void SSandboxServer::close(void)
{
  d->server->close();
  delete d->server;
  d->server = NULL;

  std::cerr << "##STOP" << std::endl;
}

void SSandboxServer::closeSocket(QAbstractSocket *socket, bool)
{
  new SocketCloseRequest(this, socket);
}

} // End of namespace
