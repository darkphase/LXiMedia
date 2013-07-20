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


HttpClientRequest::HttpClientRequest(SHttpClientEngine *parent, bool reuse, const char *file, int line)
  : QObject(parent),
    parent(parent),
    reuse(reuse),
    file(file), line(line),
    socket(NULL),
    responded(false)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpClientRequest::HttpClientRequest";
#endif

  connect(&closeTimer, SIGNAL(timeout()), SLOT(stop()));
  closeTimer.setSingleShot(true);
}

HttpClientRequest::~HttpClientRequest()
{
  Q_ASSERT(socket == NULL);
}

void HttpClientRequest::start(QIODevice *socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpClientRequest::start" << socket;
#endif

  if (socket)
  {
    this->socket = socket;
    data.clear();
    responded = false;

    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()), Qt::QueuedConnection);
    connect(socket, SIGNAL(disconnected()), SLOT(stop()), Qt::QueuedConnection);

    closeTimer.start(SHttpEngine::maxTTL);

    if (socket->bytesAvailable() > 0)
      readyRead();
  }
  else
    stop();
}

void HttpClientRequest::stop(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpClientRequest::stop" << responded;
#endif

  if (socket)
  {
    parent->closeSocket(socket);
    socket = NULL;

    if (!responded)
    {
      responded = true;
      emit response(SHttpEngine::ResponseMessage(message, SHttpEngine::Status_InternalServerError));
    }
  }

  deleteLater();
}

void HttpClientRequest::readyRead()
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpClientRequest::readyRead";
#endif

  if (!responded)
  {
    while (socket->bytesAvailable() > 0)
      data += socket->read(65536);

    if (socket && !data.isEmpty())
    {
      SHttpEngine::ResponseMessage response(NULL);
      response.parse(data);

      if (response.isValid() && response.isComplete())
      {
#ifdef TRACE_CONNECTIONS
        qDebug() << this << file << line << "HttpClientRequest::readyRead emit response";
#endif

        if (reuse && response.canReuseConnection())
          parent->reuseSocket(socket);
        else
          parent->closeSocket(socket);

        socket = NULL;

        responded = true;
        emit this->response(response);
      }
    }
  }
}


HttpServerRequest::HttpServerRequest(SHttpServerEngine *parent, quint16 serverPort, const char *file, int line)
  : QObject(parent),
    parent(parent),
    serverPort(serverPort),
    file(file), line(line),
    socket(NULL),
    headerReceived(false)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpServerRequest::HttpServerRequest" << serverPort;
#endif

  connect(&closeTimer, SIGNAL(timeout()), SLOT(stop()));
  closeTimer.setSingleShot(true);
}

HttpServerRequest::~HttpServerRequest()
{
  Q_ASSERT(socket == NULL);
}

void HttpServerRequest::start(QIODevice *socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpServerRequest::start" << socket;
#endif

  if (socket)
  {
    this->socket = socket;
    data.clear();
    headerReceived = false;
    content.clear();

    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()), Qt::QueuedConnection);
    connect(socket, SIGNAL(disconnected()), SLOT(stop()), Qt::QueuedConnection);

    closeTimer.start(SHttpEngine::maxTTL);

    readyRead();
  }
  else
    stop();
}

void HttpServerRequest::stop(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpServerRequest::stop" << socket;
#endif

  if (socket)
  {
    disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    disconnect(socket, SIGNAL(disconnected()), this, SLOT(stop()));

    if (parent)
      parent->closeSocket(socket);
    else
      delete socket;

    socket = NULL;
  }

  deleteLater();
}

void HttpServerRequest::readyRead()
{
  Q_ASSERT(QThread::currentThread() == thread());

  if (socket)
  {
#ifdef TRACE_CONNECTIONS
    qDebug() << this << file << line << "HttpServerRequest::readyRead" << socket->bytesAvailable();
#endif

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
      SHttpEngine::RequestMessage request(parent);
      request.parse(data);
      
      if (!request.isValid())
      {
#ifdef TRACE_CONNECTIONS
        qDebug() << this << file << line << "HttpServerRequest::readyRead request invalid";
#endif

        socket->write(SHttpEngine::ResponseHeader(request, SHttpEngine::Status_BadRequest));
        stop();
        return;
      }

      const qint64 length = request.contentLength();
      while ((socket->bytesAvailable() > 0) && (content.length() < length))
        content += socket->readAll();

      if (content.length() >= length)
      {
#ifdef TRACE_CONNECTIONS
        qDebug() << this << file << line << "HttpServerRequest::readyRead request valid" << length;
#endif

        request.setContent(content);

        // Add correct port number in the host section (is sometimes omitted)
        if (!request.host().isEmpty())
        {
          QString hostname; quint16 port = 0;
          if (parent->splitHost(request.host(), hostname, port))
            request.setHost(hostname, serverPort);
        }

        // This object may already be deleted even before handleHttpRequest or
        // sendHttpResponse return (because they process events on the event
        // loop) So we keep these on the stack so there is no dependency on
        // 'this' anymore.
        const QPointer<QIODevice> socket = this->socket;
        const QPointer<SHttpServerEngine> parent = this->parent;
        this->socket = NULL;
        deleteLater();

        SHttpEngine::ResponseMessage response = parent->handleHttpRequest(request, socket);
        if (socket)
          parent->sendHttpResponse(request, response, socket);
      }
    }
  }
}


HttpSocketRequest::HttpSocketRequest(SHttpClientEngine *parent, QAbstractSocket *socket, quint16 port, const QByteArray &message, const char *file, int line)
  : QObject(parent),
    parent(parent),
    port(port),
    file(file), line(line),
    message(message),
    socket(socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpSocketRequest::HttpSocketRequest" << port;
#endif

  connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(failed()), Qt::QueuedConnection);

  connect(&failTimer, SIGNAL(timeout()), SLOT(failed()));
  failTimer.setSingleShot(true);
  failTimer.start(SHttpEngine::maxTTL);

  if (socket->state() != QAbstractSocket::ConnectedState)
    connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  else
    connected();
}

HttpSocketRequest::HttpSocketRequest(SHttpClientEngine *parent, QLocalSocket *socket, const QByteArray &message, const char *file, int line)
  : QObject(parent),
    parent(parent),
    port(0),
    file(file), line(line),
    message(message),
    socket(socket)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpSocketRequest::HttpSocketRequest";
#endif

  connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten()), Qt::QueuedConnection);
  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(failed()), Qt::QueuedConnection);
  connect(socket, SIGNAL(disconnected()), SLOT(failed()), Qt::QueuedConnection);

  connect(&failTimer, SIGNAL(timeout()), SLOT(failed()));
  failTimer.setSingleShot(true);
  failTimer.start(SHttpEngine::maxTTL);

  if (socket->state() != QLocalSocket::ConnectedState)
    connect(socket, SIGNAL(connected()), SLOT(connected()), Qt::QueuedConnection);
  else
    connected();
}

HttpSocketRequest::~HttpSocketRequest()
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpSocketRequest::~HttpSocketRequest";
#endif

  if (socket)
    parent->closeSocket(socket);
}

void HttpSocketRequest::connectToHost(const QHostInfo &hostInfo)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpSocketRequest::connectToHost";
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
  qDebug() << this << file << line << "HttpSocketRequest::connected" << socket << message.count();
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
      disconnect(socket, SIGNAL(disconnected()), this, SLOT(failed()));
      disconnect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten()));

      if (aSocket) disconnect(aSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(failed()));
      if (lSocket) disconnect(lSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(failed()));
    }

    QIODevice * const s = socket;
    socket = NULL;
    deleteLater();

    emit connected(s, parent);
  }
}

void HttpSocketRequest::bytesWritten(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpSocketRequest::bytesWritten" << socket << socket->bytesToWrite();
#endif

  if (socket)
  if (socket->bytesToWrite() == 0)
    connected();
}

void HttpSocketRequest::failed(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "HttpSocketRequest::failed" << socket;
#endif

  if (socket)
  {
    qWarning() << "HTTP request failed" << socket->errorString() << "in request from:" << file << line;

    parent->closeSocket(socket);
    socket = NULL;
  }

  QMetaObject::invokeMethod(this, "connected", Qt::QueuedConnection);
}


HttpBlockingRequest::HttpBlockingRequest(SHttpClientEngine *parent, const char *file, int line)
  : QObject(parent),
    parent(parent),
    file(file), line(line),
    message(parent),
    hasResponse(false)
{
}

void HttpBlockingRequest::handleResponse(const SHttpEngine::ResponseMessage &m)
{
  message = m;
  hasResponse = true;
}


SandboxProcess::SandboxProcess(SSandboxClient *parent, const QString &cmd, const char *file, int line)
  : QObject(parent),
    parent(parent),
    file(file), line(line),
    process(new QProcess()),
    started(false)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "SandboxProcess::SandboxProcess" << cmd;
#endif

  connect(process, SIGNAL(readyRead()), SLOT(readyRead()));
  connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(finished(int, QProcess::ExitStatus)), Qt::QueuedConnection);

  process->setReadChannel(QProcess::StandardError);
  process->start(cmd);
}

SandboxProcess::~SandboxProcess()
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "SandboxProcess::~SandboxProcess";
#endif

  if (process->state() != QProcess::NotRunning)
  {
    disconnect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));

    process->write("##exit\n");
    process->waitForBytesWritten(250);
    if (!process->waitForFinished(250))
    {
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
  }
  else
    delete process;
}

bool SandboxProcess::waitForStarted(int timeout)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "SandboxProcess::waitForStarted" << timeout;
#endif

  QTime timer; timer.start();

  if (process->waitForStarted(qMax(0, timeout - timer.elapsed())))
  while (!started)
  if (!process->waitForReadyRead(qMax(0, timeout - timer.elapsed())))
    break;

  return started;
}

bool SandboxProcess::waitForReadyRead(int timeout)
{
  Q_ASSERT(QThread::currentThread() == thread());

  return process->waitForReadyRead(timeout);
}

bool SandboxProcess::isRunning(void) const
{
  Q_ASSERT(QThread::currentThread() == thread());

  return process->state() == QProcess::Running;
}

void SandboxProcess::kill(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "SandboxProcess::kill";
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
          qDebug() << this << file << line << "SandboxProcess::readyRead ##READY";
#endif

          const QList<QByteArray> items = line.simplified().split(' ');
          if (items.count() >= 2)
          {
            emit ready(QString::fromLatin1(items[1]));
            started = true;
          }
        }
        else if (line.startsWith("##STOP"))
        {
#ifdef TRACE_CONNECTIONS
          qDebug() << this << file << line << "SandboxProcess::readyRead ##STOP";
#endif

          emit stop();
        }
        else
          emit consoleLine(QString::fromUtf8(line.trimmed()));
      }
      else
        sApp->logLine(line.trimmed());
    }
  }
}

void SandboxProcess::finished(int, QProcess::ExitStatus status)
{
  Q_ASSERT(QThread::currentThread() == thread());

#ifdef TRACE_CONNECTIONS
  qDebug() << this << file << line << "SandboxProcess::finished" << status;
#endif

  emit finished(status);
}
