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

  static const int              maxSocketCount = 32;
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
  QTime timer; timer.start();

  if (!d->processStarted && !d->application.isEmpty())
  {
    startProcess();
    if (d->serverProcess)
      d->serverProcess->waitForStarted(qMax(0, timeout - timer.elapsed()));
  }

#ifdef SANDBOX_USE_LOCALSERVER
  QLocalSocket * const socket = static_cast<QLocalSocket *>(openSocket(d->serverName, true));
#else
  QTcpSocket * const socket = static_cast<QTcpSocket *>(openSocket(d->serverName, true));
#endif

  if (socket &&
      ((socket->state() == socket->ConnectedState) ||
       socket->waitForConnected(qMax(0, timeout - timer.elapsed()))))
  {
    socket->write(request);
    if (socket->waitForBytesWritten(qMax(0, timeout - timer.elapsed())))
    {
      QByteArray data;
      while (!data.endsWith("\r\n\r\n") && (qAbs(timer.elapsed()) < timeout))
      {
        if (socket->waitForReadyRead(qBound(0, timeout - timer.elapsed(), 50)))
        {
          while (socket->canReadLine() && !data.endsWith("\r\n\r\n"))
            data += socket->readLine();
        }
        else if (d->serverProcess)
        {
          if (d->serverProcess->isRunning())
            d->serverProcess->waitForReadyRead(qBound(0, timeout - timer.elapsed(), 50));
          else
            return ResponseMessage(request, Status_InternalServerError);
        }
      }

      if (data.endsWith("\r\n\r\n"))
      {
        ResponseMessage response(NULL);
        response.parse(data);

        data = socket->readAll();
        while ((!response.hasField(SHttpEngine::fieldContentLength) ||
                (data.length() < response.contentLength())) &&
               (qAbs(timer.elapsed()) < timeout))
        {
          if (socket->waitForReadyRead(qBound(0, timeout - timer.elapsed(), 50)))
          {
            data += socket->readAll();
          }
          else if (d->serverProcess)
          {
            if (d->serverProcess->isRunning())
              d->serverProcess->waitForReadyRead(qBound(0, timeout - timer.elapsed(), 50));
            else
              return ResponseMessage(request, Status_InternalServerError);
          }
        }

        if (request.canReuseConnection() && response.canReuseConnection())
          reuseSocket(socket);
        else
          closeSocket(socket);

        response.setContent(data);
        return response;
      }
    }
  }

  closeSocket(socket);

  return ResponseMessage(request, Status_BadRequest);
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

QIODevice * SSandboxClient::openSocket(const QString &host, bool force)
{
  d->socketPoolTimer.start(30000);

  while (!d->socketPool.isEmpty())
  {
#ifdef SANDBOX_USE_LOCALSERVER
    QLocalSocket * const socket = static_cast<QLocalSocket *>(d->socketPool.takeLast());

    if (socket->state() == QLocalSocket::ConnectedState)
      return socket;
    else
      closeSocket(socket);
#else
    QTcpSocket * const socket = static_cast<QTcpSocket *>(d->socketPool.takeLast());

    if (socket->state() == QTcpSocket::ConnectedState)
      return socket;
    else
      closeSocket(socket);
#endif
  }

  while ((d->socketCount >= d->maxSocketCount) && !d->socketPool.isEmpty())
    closeSocket(d->socketPool.takeFirst());

  if (force || (d->socketCount < d->maxSocketCount))
  {
#ifdef SANDBOX_USE_LOCALSERVER
    QLocalSocket * const socket = new QLocalSocket(this);
    socket->connectToServer(host);

#ifdef Q_OS_WIN
    // This is needed to ensure the socket isn't kept open by any child processes.
    ::SetHandleInformation((HANDLE)socket->socketDescriptor(), HANDLE_FLAG_INHERIT, 0);
#endif

    d->socketCount++;
    return socket;
#else
    QString hostname;
    quint16 port = 80;
    if (splitHost(host, hostname, port))
    {
      QTcpSocket * const socket = new QTcpSocket(this);
      socket->connectToHost(hostname, port);

#ifdef Q_OS_WIN
      // This is needed to ensure the socket isn't kept open by any child processes.
      ::SetHandleInformation((HANDLE)socket->socketDescriptor(), HANDLE_FLAG_INHERIT, 0);
#endif

      d->socketCount++;
      return socket;
    }
#endif
  }

  return NULL;
}

void SSandboxClient::startProcess(void)
{
  if ((d->processStarted == false) && (d->serverProcess == NULL) && !d->application.isEmpty())
  {
    d->serverProcess = new SandboxProcess(this, d->application + " " + d->modeText);

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

#ifdef SANDBOX_USE_LOCALSERVER
    QLocalSocket * const socket = static_cast<QLocalSocket *>(openSocket(d->serverName, false));
    if (socket)
    {
      HttpSocketRequest * const socketRequest =
          new HttpSocketRequest(this, socket, request.message);

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
#else
    QString hostname;
    quint16 port = 80;
    if (splitHost(d->serverName, hostname, port))
    {
      QTcpSocket * const socket = static_cast<QTcpSocket *>(openSocket(d->serverName, false));
      if (socket)
      {
        HttpSocketRequest * const socketRequest =
            new HttpSocketRequest(this, socket, port, request.message);

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
#endif
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

  emit terminated();
}

void SSandboxClient::clearSocketPool(void)
{
  while (!d->socketPool.isEmpty())
    closeSocket(d->socketPool.takeFirst());
}

} // End of namespace
