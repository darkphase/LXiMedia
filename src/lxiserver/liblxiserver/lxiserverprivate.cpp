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
#if defined(Q_OS_WIN)
#include <windows.h>
#endif

HttpClientRequest::HttpClientRequest(SHttpClientEngine *parent)
  : QObject(parent),
    socket(NULL),
    responded(false)
{
  connect(&closeTimer, SIGNAL(timeout()), SLOT(close()));
  closeTimer.setSingleShot(true);
}

HttpClientRequest::~HttpClientRequest()
{
  delete socket;
}

void HttpClientRequest::start(QAbstractSocket *socket)
{
  if (socket)
  {
    this->socket = socket;

    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), SLOT(close()));

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

        socket->deleteLater();
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
  : QObject(parent),
    parent(parent),
    socket(NULL)
{
  connect(this, SIGNAL(handleHttpRequest(SHttpEngine::RequestHeader, QAbstractSocket *)), parent, SLOT(handleHttpRequest(SHttpEngine::RequestHeader, QAbstractSocket *)));

  connect(&closeTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  closeTimer.setSingleShot(true);
}

HttpServerRequest::~HttpServerRequest()
{
  if (socket)
  {
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    socket->disconnectFromHost();
  }
}

void HttpServerRequest::start(QAbstractSocket *socket)
{
  if (socket)
  {
    this->socket = socket;

    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));

    closeTimer.start(SHttpEngine::maxTTL);

    readyRead();
  }
  else
    deleteLater();
}

void HttpServerRequest::readyRead()
{
  if (socket)
  while (socket->canReadLine())
  {
    data += socket->readLine();
    if (data.endsWith("\r\n\r\n"))
    {
      SHttpEngine::RequestHeader request(data, parent);
      if (request.isValid())
      {
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));

        emit handleHttpRequest(request, socket);

        socket = NULL;
      }
      else
      {
        socket->write(SHttpEngine::ResponseHeader(request, SHttpEngine::Status_BadRequest));

        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        socket->disconnectFromHost();
        socket = NULL;

        deleteLater();
      }

      break;
    }
  }
}


HttpSocketRequest::HttpSocketRequest(QObject *parent, const QHostAddress &host, quint16 port, const QByteArray &message)
  : QObject(parent),
    port(port),
    message(message),
    socket(new QTcpSocket(parent))
{
#ifdef Q_OS_WIN
  // This is needed to ensure the socket isn't kept open by any child
  // processes.
  HANDLE handle = (HANDLE)socket->socketDescriptor();
  ::SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0);
#endif

  connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(failed()), Qt::DirectConnection);

  socket->connectToHost(host, port);
  socket->setReadBufferSize(65536);

  connect(&deleteTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  deleteTimer.setSingleShot(true);
  deleteTimer.start(maxTTL);
}

HttpSocketRequest::HttpSocketRequest(QObject *parent, const QString &host, quint16 port, const QByteArray &message)
  : QObject(parent),
    port(port),
    message(message),
    socket(new QTcpSocket(parent))
{
#ifdef Q_OS_WIN
  // This is needed to ensure the socket isn't kept open by any child
  // processes.
  HANDLE handle = (HANDLE)socket->socketDescriptor();
  ::SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0);
#endif

  connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()), Qt::QueuedConnection);
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
  {
    socket->connectToHost(hostInfo.addresses().first(), port);
    socket->setReadBufferSize(65536);
  }
  else
    deleteLater();
}

void HttpSocketRequest::connected(void)
{
  if (socket && !message.isEmpty())
  {
    socket->write(message);
    message.clear();
  }
  else
  {
    if (socket)
    {
      disconnect(socket, SIGNAL(connected()), this, SLOT(connected()));
      disconnect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten()));
      disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(failed()));
    }

    emit connected(socket);
    socket = NULL;
    deleteLater();
  }
}

void HttpSocketRequest::bytesWritten(void)
{
  if (socket)
  if (socket->bytesToWrite() == 0)
    connected();
}

void HttpSocketRequest::failed(void)
{
  qDebug() << "HTTP request failed" << socket->errorString();

  socket->deleteLater();
  socket = NULL;

  QMetaObject::invokeMethod(this, "connected", Qt::QueuedConnection);
}


SandboxProcess::SandboxProcess(SSandboxClient *parent, const QString &cmd)
  : QObject(parent),
    parent(parent),
    process(new QProcess(parent))
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
    if (!process->waitForFinished(250))
      process->kill();

    QTimer::singleShot(1000, process, SLOT(deleteLater()));
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
        {
          const QList<QByteArray> items = line.simplified().split(' ');
          if (items.count() >= 3)
            emit ready(QHostAddress(QString::fromAscii(items[1])), items[2].toUShort());
        }
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
