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

#ifndef QT_NO_DEBUG
//#define TRACE_CONNECTIONS
#endif

class LXiServerInit : public SApplication::Initializer
{
public:
  virtual void                  startup(void);
  virtual void                  shutdown(void);

private:
  static LXiServerInit          self;
};

LXiServerInit LXiServerInit::self;

void LXiServerInit::startup(void)
{
  static bool firsttime = true;
  if (firsttime)
  {
    firsttime = false;

    // Register metatypes.
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QLocalSocket::LocalSocketError>("QLocalSocket::LocalSocketError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
  }

  sApp->addModuleFilter("lxiserver");
}

void LXiServerInit::shutdown(void)
{
}

HttpClientRequest::HttpClientRequest(SHttpClientEngine *parent)
  : QObject(parent),
    socket(NULL),
    responded(false)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpClientRequest::HttpClientRequest";
#endif

  connect(&closeTimer, SIGNAL(timeout()), SLOT(close()));
  closeTimer.setSingleShot(true);
}

HttpClientRequest::~HttpClientRequest()
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpClientRequest::~HttpClientRequest";
#endif

  delete socket;
}

void HttpClientRequest::start(QIODevice *socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpClientRequest::start" << socket;
#endif

  if (socket)
  {
    this->socket = socket;

    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()), Qt::QueuedConnection);
    connect(socket, SIGNAL(disconnected()), SLOT(close()), Qt::QueuedConnection);

    closeTimer.start(SHttpEngine::maxTTL);
  }
  else
    close();
}

void HttpClientRequest::readyRead()
{
  Q_ASSERT(QThread::currentThread() == thread());

  if (!responded)
  {
    while (socket->bytesAvailable())
      data += socket->read(65536);

    if (socket && !data.isEmpty())
    {
      SHttpEngine::ResponseMessage message(data);
      if (message.isValid() && (message.content().size() >= message.contentLength()))
      {
#ifdef TRACE_CONNECTIONS
        qDebug() << this << "HttpClientRequest::readyRead emit response";
#endif

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
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpClientRequest::close" << responded;
#endif

  if (!responded)
  {
    responded = true;
    emit response(SHttpEngine::ResponseMessage(message, SHttpEngine::Status_InternalServerError));
  }

  deleteLater();
}


HttpServerRequest::HttpServerRequest(SHttpServerEngine *parent, quint16 serverPort)
  : QObject(parent),
    parent(parent),
    serverPort(serverPort),
    socket(NULL),
    headerReceived(false)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpServerRequest::HttpServerRequest" << serverPort;
#endif

  connect(this, SIGNAL(handleHttpRequest(SHttpEngine::RequestMessage, QIODevice *)), parent, SLOT(handleHttpRequest(SHttpEngine::RequestMessage, QIODevice *)));

  connect(&closeTimer, SIGNAL(timeout()), SLOT(deleteLater()));
  closeTimer.setSingleShot(true);
}

HttpServerRequest::~HttpServerRequest()
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpServerRequest::~HttpServerRequest" << socket;
#endif

  SHttpEngine::closeSocket(socket);
}

void HttpServerRequest::start(QIODevice *socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpServerRequest::start" << socket;
#endif

  if (socket)
  {
    this->socket = socket;
    headerReceived = false;

    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()), Qt::QueuedConnection);
    connect(socket, SIGNAL(disconnected()), SLOT(close()), Qt::QueuedConnection);

    closeTimer.start(SHttpEngine::maxTTL);
  }
  else
    deleteLater();
}

void HttpServerRequest::readyRead()
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpServerRequest::readyRead";
#endif

  if (socket)
  {
    if (!headerReceived)
    while (socket->canReadLine())
    {
      data += socket->readLine();
      if (data.endsWith("\r\n\r\n"))
      {
        headerReceived = true;
        break;
      }
    }

    if (headerReceived)
    {
      SHttpEngine::RequestMessage request(data, parent);
      if (!request.isValid())
      {
#ifdef TRACE_CONNECTIONS
        qDebug() << this << "HttpServerRequest::readyRead request invalid";
#endif

        socket->write(SHttpEngine::ResponseHeader(request, SHttpEngine::Status_BadRequest));

        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(socket, SIGNAL(disconnected()), this, SLOT(close()));

        deleteLater();
        return;
      }

      const qint64 length = request.contentLength();
      while ((socket->bytesAvailable() > 0) && (content.length() < length))
        content += socket->readAll();

      if (content.length() >= length)
      {
#ifdef TRACE_CONNECTIONS
        qDebug() << this << "HttpServerRequest::readyRead request valid" << length;
#endif

        request.setContent(content);

        // Add correct port number in the host section (is sometimes omitted)
        if (!request.host().isEmpty())
        {
          QString hostname; quint16 port = 0;
          if (parent->splitHost(request.host(), hostname, port))
            request.setHost(hostname, serverPort);
        }

        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(socket, SIGNAL(disconnected()), this, SLOT(close()));

        emit handleHttpRequest(request, socket);

        socket = NULL;
        deleteLater();
        return;
      }
    }
  }
}

void HttpServerRequest::close()
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpServerRequest::close";
#endif

  deleteLater();
}


HttpSocketRequest::HttpSocketRequest(QObject *parent, QAbstractSocket *socket, const QHostAddress &host, quint16 port, const QByteArray &message)
  : QObject(parent),
    port(port),
    message(message),
    socket(socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpSocketRequest::HttpSocketRequest" << host.toString() << port;
#endif

  connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(failed()), Qt::QueuedConnection);

  socket->connectToHost(host, port);

  connect(&failTimer, SIGNAL(timeout()), SLOT(failed()));
  failTimer.setSingleShot(true);
  failTimer.start(maxTTL);
}

HttpSocketRequest::HttpSocketRequest(QObject *parent, QAbstractSocket *socket, const QString &host, quint16 port, const QByteArray &message)
  : QObject(parent),
    port(port),
    message(message),
    socket(socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpSocketRequest::HttpSocketRequest" << host << port;
#endif

  connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(failed()), Qt::QueuedConnection);

  QHostInfo::lookupHost(host, this, SLOT(connectToHost(QHostInfo)));

  connect(&failTimer, SIGNAL(timeout()), SLOT(failed()));
  failTimer.setSingleShot(true);
  failTimer.start(maxTTL);
}

HttpSocketRequest::HttpSocketRequest(QObject *parent, QLocalSocket *socket, const QString &server, const QByteArray &message)
  : QObject(parent),
    port(port),
    message(message),
    socket(socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpSocketRequest::HttpSocketRequest" << server;
#endif

  connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(failed()), Qt::QueuedConnection);

  socket->connectToServer(server);

  connect(&failTimer, SIGNAL(timeout()), SLOT(failed()));
  failTimer.setSingleShot(true);
  failTimer.start(maxTTL);
}

HttpSocketRequest::~HttpSocketRequest()
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpSocketRequest::~HttpSocketRequest";
#endif

  delete socket;
}

void HttpSocketRequest::connectToHost(const QHostInfo &hostInfo)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpSocketRequest::connectToHost";
#endif

  if (!hostInfo.addresses().isEmpty())
    static_cast<QAbstractSocket *>(socket.data())->connectToHost(hostInfo.addresses().first(), port);
  else
    deleteLater();
}

void HttpSocketRequest::connected(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpSocketRequest::connected" << socket << message.count();
#endif

  QAbstractSocket * const aSocket = qobject_cast<QAbstractSocket *>(socket);
  QLocalSocket * const lSocket = qobject_cast<QLocalSocket *>(socket);

  if (socket && !message.isEmpty())
  {
    if (aSocket) aSocket->setReadBufferSize(65536);
    if (lSocket) lSocket->setReadBufferSize(65536);

    socket->write(message);
    message.clear();
  }
  else
  {
    if (socket)
    {
      disconnect(socket, SIGNAL(connected()), this, SLOT(connected()));
      disconnect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten()));


      if (aSocket) disconnect(aSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(failed()));
      if (lSocket) disconnect(lSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(failed()));
    }

    emit connected(socket);
    socket = NULL;
    deleteLater();
  }
}

void HttpSocketRequest::bytesWritten(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpSocketRequest::bytesWritten" << socket << socket->bytesToWrite();
#endif

  if (socket)
  if (socket->bytesToWrite() == 0)
    connected();
}

void HttpSocketRequest::failed(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "HttpSocketRequest::failed" << socket;
#endif

  if (socket)
  {
    qWarning() << "HTTP request failed" << socket->errorString();

    socket->deleteLater();
    socket = NULL;
  }

  QMetaObject::invokeMethod(this, "connected", Qt::QueuedConnection);
}


SandboxProcess::SandboxProcess(SSandboxClient *parent, const QString &cmd)
  : QObject(parent),
    parent(parent),
    process(new QProcess(parent))
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "SandboxProcess::SandboxProcess" << cmd;
#endif

  connect(process, SIGNAL(readyRead()), SLOT(readyRead()), Qt::QueuedConnection);
  connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(finished(int, QProcess::ExitStatus)), Qt::QueuedConnection);

  process->setReadChannel(QProcess::StandardError);
  process->start(cmd);
}

SandboxProcess::~SandboxProcess()
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "SandboxProcess::~SandboxProcess";
#endif

  if (process->state() != QProcess::NotRunning)
  {
    disconnect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));

    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));
    process->terminate();
    if (!process->waitForFinished(250))
    {
      process->kill();
      if (!process->waitForFinished(250))
        QTimer::singleShot(30000, process, SLOT(deleteLater()));
      else
        process->deleteLater();
    }
  }
  else
    delete process;
}

void SandboxProcess::kill(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "SandboxProcess::kill";
#endif

  process->kill();
}

void SandboxProcess::readyRead()
{
  Q_ASSERT(QThread::currentThread() == thread());

  while (process->canReadLine())
  {
    const QByteArray line = process->readLine();
    if (!line.isEmpty())
    {
      if (line[0] == '#')
      {
        if (line.startsWith("##READY"))
        {
#ifdef TRACE_CONNECTIONS
          qDebug() << this << "SandboxProcess::readyRead ##READY";
#endif

          const QList<QByteArray> items = line.simplified().split(' ');
          if (items.count() >= 2)
            emit ready(QString::fromAscii(items[1]));
        }
        else if (line.startsWith("##STOP"))
        {
#ifdef TRACE_CONNECTIONS
          qDebug() << this << "SandboxProcess::readyRead ##STOP";
#endif

          emit stop();
        }
        else
          emit consoleLine(QString::fromUtf8(line.trimmed()));
      }
      else
        sApp->logLineToActiveLogFile(line.trimmed());
    }
  }
}

void SandboxProcess::finished(int, QProcess::ExitStatus status)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << "SandboxProcess::finished" << status;
#endif

  emit finished(status);
}
