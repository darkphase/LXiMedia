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

bool SHttpEngine::splitHost(const QString &host, QString &hostname, quint16 &port)
{
  bool result = false;

  const int openBr = host.indexOf('[');
  const int closeBr = host.indexOf(']', openBr);

  if ((openBr >= 0) && (closeBr > openBr))
  {
    hostname = host.mid(openBr + 1, closeBr - openBr - 1);
    result = true;

    const int colon = host.indexOf(':', closeBr);
    if (colon > closeBr)
      port = host.mid(colon + 1).toUShort(&result);
  }
  else
  {
    const int colon = host.indexOf(':');
    if (colon >= 0)
    {
      hostname = host.left(colon);
      port = host.mid(colon + 1).toUShort(&result);
    }
    else
    {
      hostname = host;
      result = true;
    }
  }

  return result;
}


struct SHttpServerEngine::Data
{
  QString                       senderId;
  QMap<QString, Callback *>     callbacks;
};

SHttpServerEngine::SHttpServerEngine(const QString &protocol, QObject *parent)
  : QObject(parent),
    d(new Data())
{
#if defined(Q_OS_UNIX)
  struct utsname osname;
  if (uname(&osname) >= 0)
    d->senderId = QString(osname.sysname) + "/" + QString(osname.release);
  else
    d->senderId = "Unix";
#elif defined(Q_OS_WIN)
  d->senderId = "Windows";
#endif

  if (!protocol.isEmpty())
    d->senderId += " " + protocol;

  d->senderId += " " + qApp->applicationName() + "/" + qApp->applicationVersion();
}

SHttpServerEngine::~SHttpServerEngine()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SHttpServerEngine::registerCallback(const QString &path, Callback *callback)
{
  d->callbacks.insert(path, callback);
}

void SHttpServerEngine::unregisterCallback(Callback *callback)
{
  for (QMap<QString, Callback *>::Iterator i=d->callbacks.begin(); i!=d->callbacks.end(); )
  if (i.value() == callback)
    i = d->callbacks.erase(i);
  else
    i++;
}

const char * SHttpServerEngine::senderType(void) const
{
  return fieldServer;
}

const QString & SHttpServerEngine::senderId(void) const
{
  return d->senderId;
}

QByteArray SHttpServerEngine::readContent(const RequestHeader &request, QAbstractSocket *socket)
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

SHttpServerEngine::SocketOp SHttpServerEngine::sendResponse(const RequestHeader &request, QAbstractSocket *socket, Status status, const QByteArray &content, const QObject *object)
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

SHttpServerEngine::SocketOp SHttpServerEngine::sendResponse(const RequestHeader &request, QAbstractSocket *socket, Status status, const QObject *object)
{
  return sendResponse(request, socket, status, QByteArray(), object);
}

SHttpServerEngine::SocketOp SHttpServerEngine::sendRedirect(const RequestHeader &request, QAbstractSocket *socket, const QString &newUrl)
{
  SHttpEngine::ResponseHeader response(request, SHttpEngine::Status_TemporaryRedirect);
  response.setField("LOCATION", newUrl);
  socket->write(response);
  return SocketOp_Close;
}

void SHttpServerEngine::handleHttpRequest(const SHttpEngine::RequestHeader &request, QAbstractSocket *socket)
{
  const QString path = QUrl(request.path()).path();

  QString dir = path.left(path.lastIndexOf('/') + 1);
  QMap<QString, Callback *>::ConstIterator callback = d->callbacks.find(dir);
  while ((callback == d->callbacks.end()) && !dir.isEmpty())
  {
    dir = dir.left(dir.left(dir.length() - 1).lastIndexOf('/') + 1);
    callback = d->callbacks.find(dir);
  }

  if ((callback != d->callbacks.end()) && dir.startsWith(callback.key()))
  {
    if ((*callback)->handleHttpRequest(request, socket) == SocketOp_LeaveOpen)
      socket = NULL; // The callback took over the socket, it is responsible for closing and deleteing it.
  }
  else
    socket->write(ResponseHeader(request, Status_NotFound));

  if (socket)
  {
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    QTimer::singleShot(30000, socket, SLOT(deleteLater()));
    socket->disconnectFromHost();
  }
}


const QEvent::Type  SHttpClientEngine::socketDestroyedEventType = QEvent::Type(QEvent::registerEventType());

struct SHttpClientEngine::Data
{
  class Socket : public QTcpSocket
  {
  public:
    Socket(SHttpClientEngine *parent)
      : QTcpSocket(parent), parent(parent)
    {
    }

    virtual ~Socket()
    {
      if (parent)
        qApp->postEvent(parent, new QEvent(socketDestroyedEventType));
    }

  private:
    const QPointer<SHttpClientEngine> parent;
  };

  QString                       senderId;

  int                           maxOpenSockets;
  int                           openSockets;
};

SHttpClientEngine::SHttpClientEngine(QObject *parent)
  : QObject(parent),
    d(new Data())
{
  d->senderId = qApp->applicationName() + "/" + qApp->applicationVersion();

#if defined(Q_OS_UNIX)
  struct utsname osname;
  if (uname(&osname) >= 0)
    d->senderId += " " + QString(osname.sysname) + "/" + QString(osname.release);
  else
    d->senderId += " Unix";
#elif defined(Q_OS_WIN)
  d->senderId += " Windows";
#endif

  d->maxOpenSockets = qMax(1, QThread::idealThreadCount()) * 2;
  d->openSockets = 0;
}

SHttpClientEngine::~SHttpClientEngine()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

const char * SHttpClientEngine::senderType(void) const
{
  return fieldUserAgent;
}

const QString & SHttpClientEngine::senderId(void) const
{
  return d->senderId;
}

void SHttpClientEngine::sendRequest(const SHttpEngine::RequestMessage &request)
{
  HttpClientRequest * const clientRequest = new HttpClientRequest(this);

  connect(clientRequest, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)));
  openRequest(request, clientRequest, SLOT(start(QAbstractSocket *)));
}

void SHttpClientEngine::customEvent(QEvent *e)
{
  if (e->type() == socketDestroyedEventType)
    socketDestroyed();
  else
    QObject::customEvent(e);
}

int SHttpClientEngine::socketsAvailable(void) const
{
  return d->maxOpenSockets - d->openSockets;
}

QAbstractSocket * SHttpClientEngine::createSocket(void)
{
  d->openSockets++;

  return new Data::Socket(this);
}

void SHttpClientEngine::socketDestroyed(void)
{
  Q_ASSERT(d->openSockets > 0);
  d->openSockets--;
}

void SHttpClientEngine::handleResponse(const SHttpEngine::ResponseMessage &message)
{
  emit response(message);
}

} // End of namespace
