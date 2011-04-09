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

#include "lxiserverprivate.h"
#include <LXiCore>

HttpClientRequest::HttpClientRequest(SHttpClientEngine *parent, const SHttpEngine::RequestMessage &message)
  : parent(parent),
    message(message),
    socket(NULL),
    responded(false)
{
  connect(this, SIGNAL(response(SHttpEngine::ResponseMessage)), parent, SLOT(handleResponse(SHttpEngine::ResponseMessage)));

  connect(&closeTimer, SIGNAL(timeout()), SLOT(close()));
  closeTimer.setSingleShot(true);
}

HttpClientRequest::~HttpClientRequest()
{
  if (socket)
    parent->closeRequest(socket, false);
}

void HttpClientRequest::start(QIODevice *socket)
{
  if (socket)
  {
    this->socket = socket;

    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(socket, SIGNAL(readChannelFinished()), SLOT(close()));

    socket->write(message.content());

    closeTimer.start(SHttpEngine::maxTTL);
  }
  else
    close();
}

void HttpClientRequest::readyRead()
{
  if (!responded)
  {
    while (socket->bytesAvailable())
      data += socket->read(65536);

    if (socket && !data.isEmpty())
    {
      SHttpEngine::ResponseMessage message(data);
      if (message.isValid() && (message.content().size() >= message.contentLength()))
      {
        responded = true;
        emit response(message);

        parent->closeRequest(socket, true);
        socket = NULL;

        deleteLater();
      }
    }
  }
}

void HttpClientRequest::close()
{
  if (!responded)
  {
    responded = true;
    emit response(SHttpEngine::ResponseMessage(message, SHttpEngine::Status_InternalServerError));
  }

  deleteLater();
}


HttpServerRequest::HttpServerRequest(SHttpServerEngine *parent)
  : parent(parent),
    socket(NULL)
{
  connect(this, SIGNAL(handleHttpRequest(SHttpEngine::RequestHeader, QIODevice *)), parent, SLOT(handleHttpRequest(SHttpEngine::RequestHeader, QIODevice *)));

  connect(&closeTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  closeTimer.setSingleShot(true);
}

HttpServerRequest::~HttpServerRequest()
{
  if (socket)
    parent->closeSocket(socket, false);
}

void HttpServerRequest::start(QIODevice *socket)
{
  if (socket)
  {
    this->socket = socket;

    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(socket, SIGNAL(readChannelFinished()), SLOT(deleteLater()));

    closeTimer.start(SHttpEngine::maxTTL);

    readyRead();
  }
  else
    deleteLater();
}

void HttpServerRequest::readyRead()
{
  while (socket->canReadLine())
  {
    data += socket->readLine();
    if (data.endsWith("\r\n\r\n"))
    {
      SHttpEngine::RequestHeader request(data, parent);
      if (request.isValid())
      {
        emit handleHttpRequest(request, socket);

        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(socket, SIGNAL(readChannelFinished()), this, SLOT(deleteLater()));

        socket = NULL;
      }
      else
      {
        socket->write(SHttpEngine::ResponseHeader(request, SHttpEngine::Status_BadRequest));

        deleteLater();
      }

      break;
    }
  }
}


HttpSocketRequest::HttpSocketRequest(const QString &host, quint16 port)
  : port(port),
    socket(new QTcpSocket())
{
  connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(failed()), Qt::DirectConnection);

  QHostInfo::lookupHost(host, this, SLOT(connectToHost(QHostInfo)));

  connect(&deleteTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  deleteTimer.setSingleShot(true);
  deleteTimer.start(maxTTL);
}

HttpSocketRequest::~HttpSocketRequest()
{
  delete socket;
}

void HttpSocketRequest::connectToHost(const QHostInfo &hostInfo)
{
  if (!hostInfo.addresses().isEmpty())
    socket->connectToHost(hostInfo.addresses().first(), port);
  else
    deleteLater();
}

void HttpSocketRequest::connected(void)
{
  emit connected(socket);
  socket = NULL;
  deleteLater();
}

void HttpSocketRequest::failed(void)
{
  delete socket;
  socket = NULL;

  QMetaObject::invokeMethod(this, "connected", Qt::QueuedConnection);
}


SandboxProcess::SandboxProcess(SSandboxClient *parent, const QString &cmd)
  : parent(parent),
    process(new QProcess())
{
  connect(process, SIGNAL(readyRead()), SLOT(readyRead()));
  connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(finished(int, QProcess::ExitStatus)));

  process->setReadChannel(QProcess::StandardError);
  process->start(cmd);
}

SandboxProcess::~SandboxProcess()
{
  if (process->state() != QProcess::NotRunning)
  {
    process->terminate();

    QTimer::singleShot(1000, process, SLOT(kill()));
    QTimer::singleShot(2000, process, SLOT(deleteLater()));
  }
  else
    delete process;
}

void SandboxProcess::kill(void)
{
  process->kill();
}

void SandboxProcess::readyRead()
{
  while (process->canReadLine())
  {
    const QByteArray line = process->readLine();
    if (!line.isEmpty())
    {
      if (line[0] == '#')
      {
        if (line.startsWith("##READY"))
          emit ready();
        else if (line.startsWith("##STOP"))
          emit stop();
        else
          emit consoleLine(QString::fromUtf8(line.trimmed()));
      }
      else
        sApp->logLineToActiveLogFile(line.trimmed());
    }
  }
}

void SandboxProcess::finished(int, QProcess::ExitStatus)
{
  emit finished();
}


SandboxSocketRequest::SandboxSocketRequest(SSandboxClient *parent, QLocalSocket *reuse)
  : parent(parent),
    socket(reuse ? reuse : new QLocalSocket())
{
  connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(failed()), Qt::DirectConnection);

  if (reuse == NULL)
  {
    socket->setReadBufferSize(65536);
    socket->connectToServer(parent->serverName());
  }

  connect(&deleteTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  deleteTimer.setSingleShot(true);
  deleteTimer.start(maxTTL);
}

SandboxSocketRequest::~SandboxSocketRequest()
{
  delete socket;
}

void SandboxSocketRequest::connected(void)
{
  disconnect(socket, SIGNAL(connected()), this, SLOT(connected()));
  disconnect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(failed()));

  emit connected(socket);

  socket = NULL;
  deleteLater();
}

void SandboxSocketRequest::failed(void)
{
  delete socket;
  socket = NULL;

  QMetaObject::invokeMethod(this, "connected", Qt::QueuedConnection);
}


SandboxMessageRequest::SandboxMessageRequest(const SHttpEngine::RequestMessage &message)
  : message(message)
{
}

SandboxMessageRequest::~SandboxMessageRequest()
{
}

void SandboxMessageRequest::connected(QIODevice *socket)
{
  if (socket)
    socket->write(message);

  emit headerSent(socket);

  deleteLater();
}


SocketCloseRequest::SocketCloseRequest(QIODevice *socket)
  : localSocket(qobject_cast<QLocalSocket *>(socket)),
    tcpSocket(qobject_cast<QTcpSocket *>(socket))
{
  if ((QThread::currentThread() == socket->thread()) && (socket->thread() != qApp->thread()))
  {
    // Blocking close
    QTime timer;
    timer.start();

    while (socket->bytesToWrite() > 0)
    if (!socket->waitForBytesWritten(qMax(maxTTL- timer.elapsed(), 0)))
      break;

    if (localSocket)
    {
      localSocket->disconnectFromServer();
      if (localSocket->state() != QLocalSocket::UnconnectedState)
        localSocket->waitForDisconnected(qMax(maxTTL- timer.elapsed(), 0));
    }
    else if (tcpSocket)
    {
      tcpSocket->disconnectFromHost();
      if (tcpSocket->state() != QTcpSocket::UnconnectedState)
        tcpSocket->waitForDisconnected(qMax(maxTTL- timer.elapsed(), 0));
    }

    delete this;
  }
  else
  {
    if (localSocket)
    {
      connect(localSocket, SIGNAL(disconnected()), SLOT(deleteLater()));
      if (localSocket->bytesToWrite() > 0)
        connect(localSocket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()));
      else
        bytesWritten();
    }
    else if (tcpSocket)
    {
      connect(tcpSocket, SIGNAL(disconnected()), SLOT(deleteLater()));
      if (tcpSocket->bytesToWrite() > 0)
        connect(tcpSocket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()));
      else
        bytesWritten();
    }

    connect(&deleteTimer, SIGNAL(timeout()), SLOT(deleteLater()));
    deleteTimer.setSingleShot(true);
    deleteTimer.start(maxTTL);
  }
}

SocketCloseRequest::~SocketCloseRequest()
{
  delete localSocket;
  delete tcpSocket;

  emit closed();
}

void SocketCloseRequest::bytesWritten(void)
{
  if (localSocket)
  {
    if (localSocket->bytesToWrite() <= 0)
      localSocket->disconnectFromServer();
  }
  else if (tcpSocket)
  {
    if (tcpSocket->bytesToWrite() <= 0)
      tcpSocket->disconnectFromHost();
  }
}
