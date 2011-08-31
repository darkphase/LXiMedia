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

const char  SHttpEngine::httpVersion[]       = "HTTP/1.1";
const int   SHttpEngine::maxTTL              = 300000;
const char  SHttpEngine::fieldConnection[]   = "Connection";
const char  SHttpEngine::fieldContentLength[]= "Content-Length";
const char  SHttpEngine::fieldContentType[]  = "Content-Type";
const char  SHttpEngine::fieldDate[]         = "Date";
const char  SHttpEngine::fieldHost[]         = "Host";
const char  SHttpEngine::fieldServer[]       = "Server";
const char  SHttpEngine::fieldUserAgent[]    = "User-Agent";
const char  SHttpEngine::dateFormat[]        = "ddd, dd MMM yyyy hh:mm:ss 'GMT'";

SHttpEngine::SHttpEngine(void)
{
}

SHttpEngine::~SHttpEngine()
{
}

/*! Returns the error description for the HTTP status code.
 */
const char * SHttpEngine::errorDescription(StatusCode code)
{
  switch (int(code))
  {
  case 100: return "Continue";
  case 101: return "Switching Protocols";

  case 200: return "OK";
  case 201: return "Created";
  case 202: return "Accepted";
  case 203: return "Non-Authoritative Information";
  case 204: return "No Content";
  case 205: return "Reset Content";
  case 206: return "Partial Content";

  case 300: return "Multiple Choices";
  case 301: return "Moved Permanently";
  case 302: return "Found";
  case 303: return "See Other";
  case 304: return "Not Modified";
  case 305: return "Use Proxy";
  case 307: return "Temporary Redirect";

  case 400: return "Bad Request";
  case 401: return "Unauthorized";
  case 402: return "Payment Required";
  case 403: return "Forbidden";
  case 404: return "Not Found";
  case 405: return "Method Not Allowed";
  case 406: return "Not Acceptable";
  case 407: return "Proxy Authentication Required";
  case 408: return "Request Time-out";
  case 409: return "Conflict";
  case 410: return "Gone";
  case 411: return "Length Required";
  case 412: return "Precondition Failed";
  case 413: return "Request Entity Too Large";
  case 414: return "Request-URI Too Large";
  case 415: return "Unsupported Media Type";
  case 416: return "Requested range not satisfiable";
  case 417: return "Expectation Failed";

  case 500: return "Internal Server Error";
  case 501: return "Not Implemented";
  case 502: return "Bad Gateway";
  case 503: return "Service Unavailable";
  case 504: return "Gateway Time-out";
  case 505: return "HTTP Version not supported";
  }

  return "";
}

/*! Returns the MIME type for the specified filename, based on the extension.
 */
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
  else if (ext == "mp3")    return "audio/mp3";
  else if (ext == "ac3")    return "audio/mpeg";
  else if (ext == "dts")    return "audio/mpeg";
  else if (ext == "oga")    return "audio/ogg";
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
  else if (ext == "ogg")    return "video/ogg";
  else if (ext == "ogv")    return "video/ogg";
  else if (ext == "ogx")    return "video/ogg";
  else if (ext == "spx")    return "video/ogg";
  else if (ext == "qt")     return "video/quicktime";
  else if (ext == "flv")    return "video/x-flv";

  // For licenses
  else if (fileName.startsWith("COPYING")) return "text/plain";

  else                      return "application/octet-stream";
}

/*! Splits the specified host string into a hostname and port number.
 */
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

void SHttpEngine::closeSocket(QIODevice *socket)
{
  if (socket)
  {
    QObject::connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    QTimer::singleShot(30000, socket, SLOT(deleteLater()));

    QAbstractSocket * const aSocket = qobject_cast<QAbstractSocket *>(socket);
    if (aSocket)
      aSocket->disconnectFromHost();

    QLocalSocket * const lSocket = qobject_cast<QLocalSocket *>(socket);
    if (lSocket)
      lSocket->disconnectFromServer();
  }
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

/*! Registers a callback with the server for the specified path.
 */
void SHttpServerEngine::registerCallback(const QString &path, Callback *callback)
{
  d->callbacks.insert(path, callback);
}

/*! Unregisters a callback with the server for the specified path.
 */
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

SHttpServerEngine::ResponseMessage SHttpServerEngine::handleHttpRequest(const SHttpEngine::RequestMessage &request, QIODevice *socket)
{
  if (request.method().compare("OPTIONS", Qt::CaseInsensitive) == 0)
  {
    const QString baseOptions = "OPTIONS,TRACE";

    if (request.path().trimmed() != "*")
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
        ResponseMessage response = (*callback)->httpOptions(request);
        const QString allow = response.field("Allow");
        response.setField("Allow", allow.isEmpty() ? baseOptions : (baseOptions + ',' + allow));

        return response;
      }
    }

    ResponseMessage response(request, Status_Ok);
    response.setField("Allow", baseOptions);

    return response;
  }
  else if (request.method().compare("TRACE", Qt::CaseInsensitive) == 0)
  {
    socket->write(request);
    closeSocket(socket);

    return ResponseMessage(request, Status_None);
  }
  else
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
      return (*callback)->httpRequest(request, socket);
    else
      return ResponseMessage(request, Status_NotFound);
  }
}

bool SHttpServerEngine::sendHttpResponse(const SHttpEngine::RequestHeader &request, SHttpEngine::ResponseMessage &response, QIODevice *socket, bool reuse)
{
  if (response.status() != SHttpEngine::Status_None)
  {
    if (reuse && response.hasField(fieldContentLength) &&
        (request.connection().compare("Close", Qt::CaseInsensitive) != 0) &&
        ((response.version() >= SHttpEngine::httpVersion) ||
         (request.connection().compare("Keep-Alive", Qt::CaseInsensitive) == 0)))
    {
      response.setConnection("Keep-Alive");

      if (request.isHead())
        response.setContent(QByteArray());

      socket->write(response);

      QAbstractSocket * const aSocket = qobject_cast<QAbstractSocket *>(socket);
      if (aSocket)
        aSocket->flush();

      QLocalSocket * const lSocket = qobject_cast<QLocalSocket *>(socket);
      if (lSocket)
        lSocket->flush();

      return true; // Reuse the socket
    }
    else
    {
      response.setConnection("Close");
      socket->write(response);
      SHttpEngine::closeSocket(socket);
    }
  }

  return false;
}

SHttpServerEngine::ResponseMessage SHttpServerEngine::Callback::httpOptions(const RequestMessage &request)
{
  ResponseMessage response(request, Status_Ok);
  response.setField("Allow", "GET,HEAD,POST");

  return response;
}


const QEvent::Type  SHttpClientEngine::socketCreatedEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SHttpClientEngine::socketDestroyedEventType = QEvent::Type(QEvent::registerEventType());

struct SHttpClientEngine::Data
{
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

  d->maxOpenSockets = qMax(1, QThread::idealThreadCount()) * 4;
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

void SHttpClientEngine::sendRequest(const RequestMessage &request)
{
  HttpClientRequest * const clientRequest = new HttpClientRequest(this);

  connect(clientRequest, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)));
  openRequest(request, clientRequest, SLOT(start(QIODevice *)));
}

void SHttpClientEngine::customEvent(QEvent *e)
{
  if (e->type() == socketCreatedEventType)
    socketCreated();
  else if (e->type() == socketDestroyedEventType)
    socketDestroyed();
  else
    QObject::customEvent(e);
}

int SHttpClientEngine::socketsAvailable(void) const
{
  return d->maxOpenSockets - d->openSockets;
}

void SHttpClientEngine::socketCreated(void)
{
  d->openSockets++;
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
