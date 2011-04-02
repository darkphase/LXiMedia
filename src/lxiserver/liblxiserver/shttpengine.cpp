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

#include "shttpengine.h"
#include "lxiserverprivate.h"
#include <QtNetwork>

#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#endif

namespace LXiServer {

const char  * const SHttpEngine::httpVersion         = "HTTP/1.1";
const int           SHttpEngine::maxTTL              = 300000;
const char  * const SHttpEngine::fieldConnection     = "CONNECTION";
const char  * const SHttpEngine::fieldContentLength  = "CONTENT-LENGTH";
const char  * const SHttpEngine::fieldContentType    = "CONTENT-TYPE";
const char  * const SHttpEngine::fieldDate           = "DATE";
const char  * const SHttpEngine::fieldHost           = "HOST";
const char  * const SHttpEngine::fieldServer         = "SERVER";
const char  * const SHttpEngine::fieldUserAgent      = "USER-AGENT";
const char  * const SHttpEngine::dateFormat          = "ddd, dd MMM yyyy hh:mm:ss 'GMT'";

SHttpEngine::SHttpEngine(void)
{
}

SHttpEngine::~SHttpEngine()
{
}

const char * SHttpEngine::toMimeType(const QString &fileName)
{
  const QString ext = QFileInfo(fileName).suffix().toLower();

  if      (ext == "js")     return "application/javascript";
  else if (ext == "pdf")    return "application/pdf";
  else if (ext == "xhtml")  return "application/xhtml+xml";
  else if (ext == "dtd")    return "application/xml-dtd";
  else if (ext == "zip")    return "application/zip";
  else if (ext == "m3u")    return "audio/x-mpegurl";
  else if (ext == "mpa")    return "audio/mpeg";
  else if (ext == "mp2")    return "audio/mpeg";
  else if (ext == "mp3")    return "audio/mpeg";
  else if (ext == "ac3")    return "audio/mpeg";
  else if (ext == "dts")    return "audio/mpeg";
  else if (ext == "oga")    return "audio/ogg";
  else if (ext == "ogg")    return "audio/ogg";
  else if (ext == "wav")    return "audio/x-wav";
  else if (ext == "lpcm")   return "audio/L16;rate=48000;channels=2";
  else if (ext == "jpeg")   return "image/jpeg";
  else if (ext == "jpg")    return "image/jpeg";
  else if (ext == "png")    return "image/png";
  else if (ext == "svg")    return "image/svg+xml";
  else if (ext == "tiff")   return "image/tiff";
  else if (ext == "css")    return "text/css;charset=utf-8";
  else if (ext == "html")   return "text/html;charset=utf-8";
  else if (ext == "htm")    return "text/html;charset=utf-8";
  else if (ext == "txt")    return "text/plain;charset=utf-8";
  else if (ext == "log")    return "text/plain;charset=utf-8";
  else if (ext == "xml")    return "text/xml;charset=utf-8";
  else if (ext == "mpeg")   return "video/mpeg";
  else if (ext == "mpg")    return "video/mpeg";
  else if (ext == "mp4")    return "video/mpeg";
  else if (ext == "ts")     return "video/mpeg";
  else if (ext == "ogv")    return "video/ogg";
  else if (ext == "ogx")    return "video/ogg";
  else if (ext == "spx")    return "video/ogg";
  else if (ext == "qt")     return "video/quicktime";
  else if (ext == "flv")    return "video/x-flv";

  // For licenses
  else if (fileName.startsWith("COPYING")) return "text/plain";

  else                      return "application/octet-stream";
}


SHttpEngine::SocketPtr & SHttpEngine::SocketPtr::operator=(QIODevice *socket)
{
  this->socket = socket;
  this->abstractSocket = qobject_cast<QAbstractSocket *>(socket);
  this->localSocket = qobject_cast<QLocalSocket *>(socket);

  return *this;
}

bool SHttpEngine::SocketPtr::isConnected(void) const
{
  if (abstractSocket)
    return abstractSocket->state() == QAbstractSocket::ConnectedState;
  else if (localSocket)
    return localSocket->state() == QLocalSocket::ConnectedState;
  else
    return !socket->atEnd();
}


class SHttpServerEngine::SocketHandler : public QRunnable
{
public:
                                SocketHandler(SHttpServerEngine *, quintptr);

protected:
  virtual void                  run();

private:
  SHttpServerEngine      * const parent;
  const quintptr                socketDescriptor;
  QTime                         timer;
  QByteArray                    response;
};

struct SHttpServerEngine::Private
{
  inline Private(void) : lock(QReadWriteLock::Recursive) { }

  QReadWriteLock                lock;
  QString                       senderId;
  QThreadPool                 * threadPool;
  static const int              maxPendingConnections = 1024;
  QAtomicInt                    numPendingConnections;
  QMap<QString, Callback *>     callbacks;
};

SHttpServerEngine::SHttpServerEngine(const QString &protocol, QObject *parent)
  : QObject(parent),
    p(new Private())
{
#if defined(Q_OS_UNIX)
  struct utsname osname;
  if (uname(&osname) >= 0)
    p->senderId = QString(osname.sysname) + "/" + QString(osname.release);
  else
    p->senderId = "Unix";
#elif defined(Q_OS_WIN)
  p->senderId = "Windows";
#endif

  if (!protocol.isEmpty())
    p->senderId += " " + protocol;

  p->senderId += " " + qApp->applicationName() + "/" + qApp->applicationVersion();

  p->threadPool = QThreadPool::globalInstance();
  p->numPendingConnections = 0;
}

SHttpServerEngine::~SHttpServerEngine()
{
  p->threadPool->waitForDone();

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SHttpServerEngine::setThreadPool(QThreadPool *threadPool)
{
  p->threadPool = threadPool ? threadPool : QThreadPool::globalInstance();
}

QThreadPool * SHttpServerEngine::threadPool(void)
{
  return p->threadPool;
}

void SHttpServerEngine::registerCallback(const QString &path, Callback *callback)
{
  QWriteLocker l(&p->lock);

  p->callbacks.insert(path, callback);
}

void SHttpServerEngine::unregisterCallback(Callback *callback)
{
  QWriteLocker l(&p->lock);

  for (QMap<QString, Callback *>::Iterator i=p->callbacks.begin(); i!=p->callbacks.end(); )
  if (i.value() == callback)
    i = p->callbacks.erase(i);
  else
    i++;
}

const char * SHttpServerEngine::senderType(void) const
{
  return fieldServer;
}

const QString & SHttpServerEngine::senderId(void) const
{
  return p->senderId;
}

QByteArray SHttpServerEngine::readContent(const RequestHeader &request, QIODevice *socket)
{
  QTime timer;
  timer.start();

  QByteArray content=socket->readAll();

  const qint64 length = request.contentLength();
  if (length > 0)
  {
    for (; content.length() < length; content += socket->readAll())
    if (!socket->waitForReadyRead(qMax(maxTTL - qAbs(timer.elapsed()), 0)))
      return QByteArray();
  }
  else
  {
    while (socket->waitForReadyRead(qMin(2000, qMax(maxTTL - qAbs(timer.elapsed()), 0))))
      content += socket->readAll();
  }

  return content;
}

SHttpServerEngine::SocketOp SHttpServerEngine::sendResponse(const RequestHeader &request, QIODevice *socket, Status status, const QByteArray &content, const QObject *object)
{
  if (status >= 400)
  {
    qDebug() << "HTTP response:" << int(status) << "\""
        << ResponseHeader::statusText(status) << "\" for request:"
        << request.method() << request.path() << "from object: \""
        << (object ? object->metaObject()->className() : "NULL") << "\"";
  }

  ResponseHeader response(request, status);
  response.setContentLength(content.size());
  socket->write(response);
  socket->write(content);
  return SocketOp_Close;
}

SHttpServerEngine::SocketOp SHttpServerEngine::sendResponse(const RequestHeader &request, QIODevice *socket, Status status, const QObject *object)
{
  return sendResponse(request, socket, status, QByteArray(), object);
}

SHttpServerEngine::SocketOp SHttpServerEngine::sendRedirect(const RequestHeader &request, QIODevice *socket, const QString &newUrl)
{
  SHttpEngine::ResponseHeader response(request, SHttpEngine::Status_TemporaryRedirect);
  response.setField("LOCATION", newUrl);
  socket->write(response);
  return SocketOp_Close;
}

QReadWriteLock * SHttpServerEngine::lock(void) const
{
  return &p->lock;
}

bool SHttpServerEngine::handleConnection(quintptr socketDescriptor)
{
  if (p->numPendingConnections < p->maxPendingConnections)
  {
    p->threadPool->start(new SocketHandler(this, socketDescriptor));
    return true;
  }
  else
    return false;
}


SHttpServerEngine::SocketHandler::SocketHandler(SHttpServerEngine *parent, quintptr socketDescriptor)
  : parent(parent),
    socketDescriptor(socketDescriptor)
{
  parent->p->numPendingConnections.ref();
  timer.start();
}

void SHttpServerEngine::SocketHandler::run()
{
  parent->p->numPendingConnections.deref();

  QIODevice *socket = parent->openSocket(socketDescriptor);
  if (socket)
  while (socket->canReadLine() || socket->waitForReadyRead(qMax(maxTTL - qAbs(timer.elapsed()), 0)))
  {
    const QByteArray line = socket->readLine();

    if (line.trimmed().length() > 0)
    {
      response += line;
    }
    else // Header complete
    {
      response += line;

      const RequestHeader request(response, parent);
      if (request.isValid())
      {
        QReadLocker l(&parent->p->lock);

        const QString path = QUrl(request.path()).path();

        QString dir = path.left(path.lastIndexOf('/') + 1);
        QMap<QString, Callback *>::ConstIterator callback = parent->p->callbacks.find(dir);
        while ((callback == parent->p->callbacks.end()) && !dir.isEmpty())
        {
          dir = dir.left(dir.left(dir.length() - 1).lastIndexOf('/') + 1);
          callback = parent->p->callbacks.find(dir);
        }

        if ((callback != parent->p->callbacks.end()) && dir.startsWith(callback.key()))
        {
          if ((*callback)->handleHttpRequest(request, socket) == SocketOp_LeaveOpen)
            socket = NULL; // The callback took over the socket, it is responsible for closing and deleteing it.
        }
        else
          socket->write(ResponseHeader(request, Status_NotFound));
      }
      else
        socket->write(ResponseHeader(request, Status_BadRequest));

      // Finished.
      if (socket)
        parent->closeSocket(socket, false);

      return;
    }
  }

  if (socket)
    parent->closeSocket(socket, false);
}


struct SHttpClientEngine::Private
{
  static const QEvent::Type     requestEventType;

  class RequestEvent : public QEvent
  {
  public:
    explicit RequestEvent(const SHttpEngine::RequestMessage &request)
      : QEvent(requestEventType),
        request(request)
    {
    }

  public:
    const SHttpEngine::RequestMessage request;
  };

  QString                       senderId;
};

const QEvent::Type  SHttpClientEngine::Private::requestEventType = QEvent::Type(QEvent::registerEventType());

SHttpClientEngine::SHttpClientEngine(QObject *parent)
  : QObject(parent),
    p(new Private())
{
  p->senderId = qApp->applicationName() + "/" + qApp->applicationVersion();

#if defined(Q_OS_UNIX)
  struct utsname osname;
  if (uname(&osname) >= 0)
    p->senderId += " " + QString(osname.sysname) + "/" + QString(osname.release);
  else
    p->senderId += " Unix";
#elif defined(Q_OS_WIN)
  p->senderId += " Windows";
#endif
}

SHttpClientEngine::~SHttpClientEngine()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

const char * SHttpClientEngine::senderType(void) const
{
  return fieldUserAgent;
}

const QString & SHttpClientEngine::senderId(void) const
{
  return p->senderId;
}

void SHttpClientEngine::sendRequest(const SHttpEngine::RequestMessage &request)
{
  if (QThread::currentThread() == thread())
    openRequest(request, new HttpClientRequest(this, request), SLOT(start(QIODevice *)));
  else
    QCoreApplication::postEvent(this, new Private::RequestEvent(request));
}

void SHttpClientEngine::customEvent(QEvent *e)
{
  if (e->type() == Private::requestEventType)
  {
    Private::RequestEvent * const event = static_cast<Private::RequestEvent *>(e);

    openRequest(event->request, new HttpClientRequest(this, event->request), SLOT(start(QIODevice *)));
  }
  else
    QObject::customEvent(e);
}

} // End of namespace
