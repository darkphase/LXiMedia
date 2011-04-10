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

HttpClientRequest::HttpClientRequest(SHttpClientEngine *parent)
  : parent(parent),
    socket(NULL),
    responded(false)
{
  connect(&closeTimer, SIGNAL(timeout()), SLOT(close()));
  closeTimer.setSingleShot(true);
}

HttpClientRequest::~HttpClientRequest()
{
  if (socket)
    parent->closeRequest(socket, false);
}

void HttpClientRequest::start(QAbstractSocket *socket)
{
  if (socket)
  {
    this->socket = socket;

    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
    connect(socket, SIGNAL(readChannelFinished()), SLOT(close()));

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
  connect(this, SIGNAL(handleHttpRequest(SHttpEngine::RequestHeader, QAbstractSocket *)), parent, SLOT(handleHttpRequest(SHttpEngine::RequestHeader, QAbstractSocket *)));

  connect(&closeTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  closeTimer.setSingleShot(true);
}

HttpServerRequest::~HttpServerRequest()
{
  if (socket)
    parent->closeSocket(socket, false);
}

void HttpServerRequest::start(QAbstractSocket *socket)
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
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(socket, SIGNAL(readChannelFinished()), this, SLOT(deleteLater()));

        emit handleHttpRequest(request, socket);

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


HttpSocketRequest::HttpSocketRequest(const QHostAddress &host, quint16 port, const QByteArray &message)
  : port(port),
    message(message),
    socket(new QTcpSocket())
{
  connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(failed()), Qt::DirectConnection);

  socket->connectToHost(host, port);
  socket->setReadBufferSize(65536);

  connect(&deleteTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  deleteTimer.setSingleShot(true);
  deleteTimer.start(maxTTL);
}

HttpSocketRequest::HttpSocketRequest(const QString &host, quint16 port, const QByteArray &message)
  : port(port),
    message(message),
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
    socket->write(message);

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


SocketCloseRequest::SocketCloseRequest(QAbstractSocket *socket)
  : socket(socket)
{
  if ((QThread::currentThread() == socket->thread()) && (socket->thread() != qApp->thread()))
  {
    // Blocking close
    QTime timer;
    timer.start();

    while (socket->bytesToWrite() > 0)
    if (!socket->waitForBytesWritten(qMax(maxTTL- timer.elapsed(), 0)))
      break;

    socket->disconnectFromHost();
    if (socket->state() != QAbstractSocket::UnconnectedState)
      socket->waitForDisconnected(qMax(maxTTL- timer.elapsed(), 0));

    delete this;
  }
  else
  {
    connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));
    if (socket->bytesToWrite() > 0)
      connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()));
    else
      bytesWritten();

    connect(&deleteTimer, SIGNAL(timeout()), SLOT(deleteLater()));
    deleteTimer.setSingleShot(true);
    deleteTimer.start(maxTTL);
  }
}

SocketCloseRequest::~SocketCloseRequest()
{
  delete socket;

  emit closed();
}

void SocketCloseRequest::bytesWritten(void)
{
  if (socket->bytesToWrite() <= 0)
    socket->disconnectFromHost();
}
