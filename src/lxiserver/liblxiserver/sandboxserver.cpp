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

#include "sandboxserver.h"
#include <QtNetwork>
#include <iostream>
#if defined(Q_OS_UNIX)
#include <unistd.h>
#include <sched.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace LXiServer {

class SandboxServer::Server : public QLocalServer
{
public:
  explicit                      Server(const QString &, SandboxServer *parent);

protected:
  virtual void                  incomingConnection(quintptr);

private:
  SandboxServer         * const parent;
};

struct SandboxServer::Private
{
  Server                      * server;
  QAtomicInt                    clients;
};

SandboxServer::SandboxServer(QObject *parent)
  : HttpServerEngine("Sandbox/1.0", parent),
    p(new Private())
{
  p->server = NULL;
  p->clients = 0;
}

SandboxServer::~SandboxServer()
{
  delete p->server;
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SandboxServer::initialize(const QString &name, const QString &mode)
{
  QWriteLocker l(lock());

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
    ::SetPriorityClass(::GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
#endif
  }
}

void SandboxServer::close(void)
{
  QWriteLocker l(lock());

  delete p->server;
  p->server = NULL;

  threadPool()->waitForDone();

  std::cerr << "##STOP" << std::endl;
}

QIODevice * SandboxServer::openSocket(quintptr socketDescriptor, int)
{
  QLocalSocket * const socket = new QLocalSocket();
  if (socket->setSocketDescriptor(socketDescriptor))
  {
    if (p->clients.fetchAndAddRelaxed(1) == 0)
      emit busy();

    return socket;
  }

  delete socket;
  return NULL;
}

void SandboxServer::closeSocket(QIODevice *device, bool, int timeout)
{
  QLocalSocket * const socket = static_cast<QLocalSocket *>(device);

  if (socket->state() == QLocalSocket::ConnectedState)
  {
    QTime timer;
    timer.start();

    socket->waitForBytesWritten(qMax(timeout - qAbs(timer.elapsed()), 0));
    socket->disconnectFromServer();
    if (socket->state() != QLocalSocket::UnconnectedState)
      socket->waitForDisconnected(qMax(timeout - qAbs(timer.elapsed()), 0));
  }

  if (p->clients.fetchAndAddRelaxed(-1) == 1)
    emit idle();

  delete socket;
}


SandboxServer::Server::Server(const QString &name, SandboxServer *parent)
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

void SandboxServer::Server::incomingConnection(quintptr socketDescriptor)
{
  if (!parent->handleConnection(socketDescriptor))
  {
    QLocalSocket socket;
    if (socket.setSocketDescriptor(socketDescriptor))
      socket.abort();
  }
}

} // End of namespace
