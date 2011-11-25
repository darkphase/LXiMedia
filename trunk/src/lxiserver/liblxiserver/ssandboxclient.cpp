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
#ifdef SANDBOX_USE_LOCALSERVER
  class Socket : public QLocalSocket
  {
  public:
    Socket(SHttpClientEngine *parent)
      : QLocalSocket(parent), parent(parent)
    {
      if (parent)
        qApp->sendEvent(parent, new QEvent(socketCreatedEventType));
    }

    virtual ~Socket()
    {
      if (parent)
        qApp->postEvent(parent, new QEvent(socketDestroyedEventType));
    }

    inline HttpSocketRequest * createRequest(const QString &serverName, const QByteArray &message)
    {
      return new HttpSocketRequest(parent, this, serverName, message);
    }

  private:
    const QPointer<SHttpClientEngine> parent;
  };
#else
  class Socket : public QTcpSocket
  {
  public:
    Socket(SHttpClientEngine *parent)
      : QTcpSocket(parent), parent(parent)
    {
      if (parent)
        qApp->sendEvent(parent, new QEvent(socketCreatedEventType));

#ifdef Q_OS_WIN
      // This is needed to ensure the socket isn't kept open by any child
      // processes.
      ::SetHandleInformation((HANDLE)socketDescriptor(), HANDLE_FLAG_INHERIT, 0);
#endif
    }

    virtual ~Socket()
    {
      if (parent)
        qApp->postEvent(parent, new QEvent(socketDestroyedEventType));
    }

    inline HttpSocketRequest * createRequest(const QString &serverName, const QByteArray &message)
    {
      QString address; quint16 port = 0;
      if (splitHost(serverName, address, port))
        return new HttpSocketRequest(parent, this, QHostAddress(address), port, message);

      delete this;
      return NULL;
    }

    inline void connectToServer(const QString &serverName)
    {
      QString address; quint16 port = 0;
      if (splitHost(serverName, address, port))
        QTcpSocket::connectToHost(address, port);
    }

  private:
    const QPointer<SHttpClientEngine> parent;
  };
#endif

  struct Request
  {
    inline Request(const QByteArray &message, QObject *receiver, const char *slot)
      : message(message), receiver(receiver), slot(slot)
    {
    }

    QByteArray                  message;
    QObject                   * receiver;
    const char                * slot;
  };

  QString                       application;
  SSandboxServer              * localServer;
  Priority                      priority;
  QString                       modeText;

  QString                       serverName;
  SandboxProcess              * serverProcess;
  bool                          processStarted;
  QList<Request>                requests;
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
}

SSandboxClient::~SSandboxClient()
{
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

void SSandboxClient::openRequest(const RequestMessage &message, QObject *receiver, const char *slot)
{
  if (QThread::currentThread() != thread())
    qFatal("SSandboxClient::openRequest() should be invoked from the thread "
           "that owns the SSandboxClient object.");

  d->requests.append(Data::Request(message, receiver, slot));
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

  Data::Socket socket(NULL);
  socket.connectToServer(d->serverName);
  if (socket.waitForConnected(qMax(0, timeout - timer.elapsed())))
  {
    socket.write(request);
    if (socket.waitForBytesWritten(qMax(0, timeout - timer.elapsed())))
    {
      QByteArray data;
      while (!data.endsWith("\r\n\r\n") && (qAbs(timer.elapsed()) < timeout))
      {
        if (socket.waitForReadyRead(qBound(0, timeout - timer.elapsed(), 50)))
        {
          while (socket.canReadLine() && !data.endsWith("\r\n\r\n"))
            data += socket.readLine();
        }
        else if (d->serverProcess)
          d->serverProcess->waitForReadyRead(qBound(0, timeout - timer.elapsed(), 50));
      }

      ResponseMessage response(NULL);
      response.parse(data);

      data = socket.readAll();
      while (((response.contentLength() == 0) || (data.length() < response.contentLength())) &&
             (qAbs(timer.elapsed()) < timeout))
      {
        if (socket.waitForReadyRead(qBound(0, timeout - timer.elapsed(), 50)))
          data += socket.readAll();
        else if (d->serverProcess)
          d->serverProcess->waitForReadyRead(qBound(0, timeout - timer.elapsed(), 50));
      }

      response.setContent(data);
      return response;
    }
  }

  return ResponseMessage(request, Status_BadRequest);
}

void SSandboxClient::socketDestroyed(void)
{
  SHttpClientEngine::socketDestroyed();

  openRequest();
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
  if (!d->processStarted)
  {
    startProcess();
  }
  else while (d->processStarted && !d->requests.isEmpty() && (socketsAvailable() > 0))
  {
    const Data::Request request = d->requests.takeFirst();

    HttpSocketRequest * const socketRequest = (new Data::Socket(this))->createRequest(d->serverName, request.message);
    if (socketRequest && request.receiver)
      connect(socketRequest, SIGNAL(connected(QIODevice *)), request.receiver, request.slot, Qt::DirectConnection);
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
}

} // End of namespace
