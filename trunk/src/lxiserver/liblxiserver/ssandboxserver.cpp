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
# include <unistd.h>
# if defined(Q_OS_LINUX)
#  include <sys/syscall.h>
# endif
#elif defined(Q_OS_WIN)
# include <windows.h>
# ifndef PROCESS_MODE_BACKGROUND_BEGIN
#  define PROCESS_MODE_BACKGROUND_BEGIN 0x00100000
# endif
#endif

namespace LXiServer {

struct SSandboxServer::Data
{
  QString                       mode;
  Server                      * server;
  int                           openConnections;
};

#ifdef SANDBOX_USE_LOCALSERVER
class SSandboxServer::Server : public QLocalServer
{
public:
  explicit Server(SSandboxServer *parent)
    : QLocalServer(parent), parent(parent)
  {
  }

  inline bool listen(void)
  {
    const QString base = QFileInfo(qApp->applicationFilePath()).baseName() + '.';

    for (int i=0; i<8; i++)
    if (QLocalServer::listen(base + QUuid::createUuid().toString().replace("{", "").replace("}", "")))
      return true;

    qDebug() << "SSandboxServer" << errorString();
    return false;
  }

private:
  const QPointer<SSandboxServer> parent;
};
#else
class SSandboxServer::Server : public QTcpServer
{
public:
  explicit Server(SSandboxServer *parent)
    : QTcpServer(parent), parent(parent)
  {
  }

  inline bool listen(void)
  {
    if (!QTcpServer::listen(QHostAddress::LocalHostIPv6))
    if (!QTcpServer::listen(QHostAddress::LocalHost))
      return false;

    return true;
  }

  inline QString serverName(void) const
  {
    return "[" + serverAddress().toString() + "]:" + QString::number(serverPort());
  }

private:
  const QPointer<SSandboxServer> parent;
};
#endif

SSandboxServer::SSandboxServer(QObject *parent)
  : SHttpServerEngine("Sandbox/1.0", parent),
    d(new Data())
{
  d->server = NULL;
  d->openConnections = 0;
}

SSandboxServer::~SSandboxServer()
{
  delete d->server;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SSandboxServer::initialize(const QString &mode)
{
  d->mode = mode;
  d->server = new Server(this);

  connect(d->server, SIGNAL(newConnection()), SLOT(newConnection()));

  if (!d->server->listen())
  {
    qWarning() << "SSandboxServer Failed to bind interface";
    return false;
  }

  if (d->mode != "local")
    std::cerr << "##READY " << d->server->serverName().toAscii().data() << std::endl;

  // This is performed after initialization to prevent priority inversion with
  // the process that is waiting for this one to start.
  if (d->mode == "lowprio")
  {
#if defined(Q_OS_UNIX)
    ::nice(15);
# if defined(Q_OS_LINUX)
    ::syscall(SYS_ioprio_set, 1, ::getpid(), 0x6007);
# endif
#elif defined(Q_OS_WIN)
    ::SetPriorityClass(::GetCurrentProcess(), IDLE_PRIORITY_CLASS);
    ::SetPriorityClass(::GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN);
#endif
  }
  else if (d->mode == "highprio")
  {
#if defined(Q_OS_UNIX)
    ::nice(-5);
# if defined(Q_OS_LINUX)
    ::syscall(SYS_ioprio_set, 1, ::getpid(), 0x4002);
# endif
#elif defined(Q_OS_WIN)
    ::SetPriorityClass(::GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
#endif
  }

  return true;
}

void SSandboxServer::close(void)
{
  d->server->close();
  delete d->server;
  d->server = NULL;

  if (d->mode != "local")
    std::cerr << "##STOP" << std::endl;
}

QString SSandboxServer::serverName(void) const
{
  return d->server->serverName();
}

void SSandboxServer::newConnection(void)
{
  while (d->server->hasPendingConnections())
  {
    QIODevice * const socket = d->server->nextPendingConnection();
    if (socket)
    {
      connect(socket, SIGNAL(destroyed()), SLOT(closedConnection()));
      if (d->openConnections++ == 0)
        emit busy();

      (new HttpServerRequest(this, 0, __FILE__, __LINE__))->start(socket);
    }
  }
}

void SSandboxServer::closedConnection(void)
{
  if (--d->openConnections == 0)
    emit idle();
}

} // End of namespace
