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

HttpClientRequest::HttpClientRequest(SHttpClientEngine *parent, const SHttpEngine::RequestMessage &message)
  : parent(parent),
    message(message),
    socket(NULL),
    responded(false)
{
  connect(this, SIGNAL(response(SHttpEngine::ResponseMessage)), parent, SIGNAL(response(SHttpEngine::ResponseMessage)));

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
      }
      else
        emit consoleLine(QString::fromUtf8(line.trimmed()));
    }
  }
}

void SandboxProcess::finished(int, QProcess::ExitStatus)
{
  emit finished();
}


SandboxSocketRequest::SandboxSocketRequest(SSandboxClient *parent)
  : parent(parent),
    socket(new QLocalSocket())
{
  connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(failed()), Qt::DirectConnection);

  socket->setReadBufferSize(65536);
  socket->connectToServer(parent->serverName());

  connect(&deleteTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  deleteTimer.setSingleShot(true);
  deleteTimer.start(maxTTL);
}

SandboxSocketRequest::SandboxSocketRequest(QIODevice *socket)
  : parent(NULL),
    socket(static_cast<QLocalSocket *>(socket))
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

SandboxSocketRequest::~SandboxSocketRequest()
{
  delete socket;
}

void SandboxSocketRequest::failed(void)
{
  socket = NULL;

  QMetaObject::invokeMethod(this, "connected", Qt::QueuedConnection);
}

void SandboxSocketRequest::connected(void)
{
  emit connected(socket);
  socket = NULL;
  deleteLater();
}

void SandboxSocketRequest::bytesWritten(void)
{
  if (socket->bytesToWrite() <= 0)
    socket->disconnectFromServer();
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
