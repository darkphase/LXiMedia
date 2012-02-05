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

#include "ssandboxclient.h"
#include "lxiserverprivate.h"
#include <QtNetwork>
#if defined(Q_OS_WIN)
# include <windows.h>
#endif

namespace LXiServer {

struct SSandboxClient::Data
{
  struct Request
  {
    inline Request(const QByteArray &message, QObject *receiver, const char *slot, Qt::ConnectionType connectionType)
      : message(message), receiver(receiver), slot(slot), connectionType(connectionType)
    {
    }

    QByteArray                  message;
    QObject                   * receiver;
    const char                * slot;
    Qt::ConnectionType          connectionType;
  };

  QString                       application;
  SSandboxServer              * localServer;
  Priority                      priority;
  QString                       modeText;

  QString                       serverName;
  SandboxProcess              * serverProcess;
  bool                          processStarted;
  QList<Request>                requests;

#if defined(Q_OS_WIN)
  static const int              maxSocketCount = 4;
#else
  static const int              maxSocketCount = 32;
#endif
  int                           socketCount;
  QList<QIODevice *>            socketPool;
  QTimer                        socketPoolTimer;
};

SSandboxClient::SSandboxClient(const QString &application, Priority priority, QObject *parent)
  : SHttpClientEngine(parent),
    d(new Data())
{
  d->application = application;
  d->localServer = NULL;
  d->priority = priority;

  switch(priority)
  {
  case Priority_Low:    d->modeText = "lowprio";    break;
  case Priority_Normal: d->modeText = "normalprio"; break;
  case Priority_High:   d->modeText = "highprio";   break;
  }

  d->serverProcess = NULL;
  d->processStarted = false;

  d->socketCount = 0;
  d->socketPoolTimer.setSingleShot(true);
  connect(&d->socketPoolTimer, SIGNAL(timeout()), SLOT(clearSocketPool()));
}

SSandboxClient::SSandboxClient(SSandboxServer *localServer, Priority priority, QObject *parent)
  : SHttpClientEngine(parent),
    d(new Data())
{
  d->localServer = localServer;
  d->priority = priority;
  d->serverProcess = NULL;
  d->serverName = localServer->serverName();
  d->processStarted = true;

  d->socketCount = 0;
  d->socketPoolTimer.setSingleShot(true);
  connect(&d->socketPoolTimer, SIGNAL(timeout()), SLOT(clearSocketPool()));
}

SSandboxClient::~SSandboxClient()
{
  emit terminated();
  clearSocketPool();

  Q_ASSERT(d->socketCount == 0);

  delete d->serverProcess;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

SSandboxClient::Priority SSandboxClient::priority(void) const
{
  return d->priority;
}

void SSandboxClient::ensureStarted(void)
{
  openRequest();
}

void SSandboxClient::openRequest(const RequestMessage &message, QObject *receiver, const char *slot, Qt::ConnectionType connectionType)
{
  if (QThread::currentThread() != thread())
    qFatal("SSandboxClient::openRequest() should be invoked from the thread "
           "that owns the SSandboxClient object.");

  d->requests.append(Data::Request(message, receiver, slot, connectionType));
  openRequest();
}

SSandboxClient::ResponseMessage SSandboxClient::blockingRequest(const RequestMessage &request, int timeout)
{
  RequestMessage r = request;
  r.setHost(d->serverName);

  return SHttpClientEngine::blockingRequest(r, timeout);
}

int SSandboxClient::maxSocketCount(void) const
{
  return d->maxSocketCount;
}

void SSandboxClient::closeSocket(QIODevice *socket)
{
  if (QThread::currentThread() == thread())
  {
    SHttpClientEngine::closeSocket(socket);

    if (socket)
    {
      d->socketCount--;

      openRequest();
    }
  }
  else
    SHttpClientEngine::closeSocket(socket); // Will call closeSocket() again from the correct thread.
}

void SSandboxClient::reuseSocket(QIODevice *socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

  d->socketPool.append(socket);
  d->socketPoolTimer.start(30000);

  openRequest();
}

QIODevice * SSandboxClient::openSocket(const QString &host)
{
  d->socketPoolTimer.start(30000);

  while (!d->socketPool.isEmpty())
  {
    QLocalSocket * const socket = static_cast<QLocalSocket *>(d->socketPool.takeLast());

    if (socket->state() == QLocalSocket::ConnectedState)
      return socket;
    else
      closeSocket(socket);
  }

  while ((d->socketCount >= d->maxSocketCount) && !d->socketPool.isEmpty())
    closeSocket(d->socketPool.takeFirst());

  if (d->socketCount < d->maxSocketCount)
  {
    QLocalSocket * const socket = new QLocalSocket(this);
    socket->connectToServer(host);

#ifdef Q_OS_WIN
    // This is needed to ensure the socket isn't kept open by any child processes.
    ::SetHandleInformation((HANDLE)socket->socketDescriptor(), HANDLE_FLAG_INHERIT, 0);
#endif

    d->socketCount++;
    return socket;
  }

  return NULL;
}

void SSandboxClient::startProcess(void)
{
  if ((d->processStarted == false) && (d->serverProcess == NULL) && !d->application.isEmpty())
  {
    d->serverProcess = new SandboxProcess(this, d->application + " " + d->modeText, __FILE__, __LINE__);

    connect(d->serverProcess, SIGNAL(ready(QString)), SLOT(processStarted(QString)));
    connect(d->serverProcess, SIGNAL(stop()), SLOT(stop()));
    connect(d->serverProcess, SIGNAL(finished(QProcess::ExitStatus)), SLOT(finished(QProcess::ExitStatus)));
    connect(d->serverProcess, SIGNAL(consoleLine(QString)), SIGNAL(consoleLine(QString)));
  }
}

void SSandboxClient::processStarted(const QString &serverName)
{
  d->serverName = serverName;
  d->processStarted = true;

  openRequest();
}

void SSandboxClient::openRequest(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

  if (!d->processStarted)
  {
    startProcess();
  }
  else while (d->processStarted && !d->requests.isEmpty())
  {
    const Data::Request request = d->requests.takeFirst();

    QLocalSocket * const socket = static_cast<QLocalSocket *>(openSocket(d->serverName));
    if (socket)
    {
      HttpSocketRequest * const socketRequest =
          new HttpSocketRequest(this, socket, request.message, __FILE__, __LINE__);

      if (request.receiver)
        connect(socketRequest, SIGNAL(connected(QIODevice *, SHttpEngine *)), request.receiver, request.slot, request.connectionType);
      else
        connect(socketRequest, SIGNAL(connected(QIODevice *, SHttpEngine *)), SLOT(closeSocket(QIODevice *)));
    }
    else // No more free sockets.
    {
      d->requests.prepend(request);
      break;
    }
  }
}

void SSandboxClient::stop(void)
{
  if (d->serverProcess)
  {
    disconnect(d->serverProcess, SIGNAL(ready(QString)), this, SLOT(processStarted(QString)));
    disconnect(d->serverProcess, SIGNAL(stop()), this, SLOT(stop()));
    disconnect(d->serverProcess, SIGNAL(finished(QProcess::ExitStatus)), this, SLOT(finished(QProcess::ExitStatus)));
    disconnect(d->serverProcess, SIGNAL(consoleLine(QString)), this, SIGNAL(consoleLine(QString)));

    QTimer::singleShot(250, d->serverProcess, SLOT(kill()));
    QTimer::singleShot(1000, d->serverProcess, SLOT(deleteLater()));

    d->serverProcess = NULL;
  }

  d->processStarted = false;

  clearSocketPool();
}

void SSandboxClient::finished(QProcess::ExitStatus status)
{
  if (d->serverProcess)
  {
    if (status == QProcess::CrashExit)
      qWarning() << "Sandbox process terminated unexpectedly.";

    d->serverProcess->deleteLater();
    d->serverProcess = NULL;
  }

  d->processStarted = false;

  clearSocketPool();

  emit terminated();
}

void SSandboxClient::clearSocketPool(void)
{
  while (!d->socketPool.isEmpty())
    closeSocket(d->socketPool.takeFirst());
}

} // End of namespace
