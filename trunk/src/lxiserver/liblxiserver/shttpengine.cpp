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

class SHttpEngine::CloseSocketEvent : public QEvent
{
public:
  inline CloseSocketEvent(QIODevice *socket)
    : QEvent(closeSocketEventType), socket(socket)
  {
  }

public:
  QIODevice * const socket;
};

const QEvent::Type  SHttpEngine::closeSocketEventType = QEvent::Type(QEvent::registerEventType());

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

const char  SHttpEngine::mimeAppOctet[]      = "application/octet-stream";
const char  SHttpEngine::mimeAudioAac[]      = "audio/aac";
const char  SHttpEngine::mimeAudioAc3[]      = "audio/x-ac3";
const char  SHttpEngine::mimeAudioLpcm[]     = "audio/L16;rate=48000;channels=2";
const char  SHttpEngine::mimeAudioMp3[]      = "audio/mp3";
const char  SHttpEngine::mimeAudioMpeg[]     = "audio/mpeg";
const char  SHttpEngine::mimeAudioMpegUrl[]  = "audio/x-mpegurl";
const char  SHttpEngine::mimeAudioOgg[]      = "audio/ogg";
const char  SHttpEngine::mimeAudioWave[]     = "audio/wave";
const char  SHttpEngine::mimeAudioWma[]      = "audio/x-ms-wma";
const char  SHttpEngine::mimeImageJpeg[]     = "image/jpeg";
const char  SHttpEngine::mimeImagePng[]      = "image/png";
const char  SHttpEngine::mimeImageSvg[]      = "image/svg+xml";
const char  SHttpEngine::mimeImageTiff[]     = "image/tiff";
const char  SHttpEngine::mimeVideo3g2[]      = "video/3gpp";
const char  SHttpEngine::mimeVideoAsf[]      = "video/x-ms-asf";
const char  SHttpEngine::mimeVideoAvi[]      = "video/avi";
const char  SHttpEngine::mimeVideoFlv[]      = "video/x-flv";
const char  SHttpEngine::mimeVideoMatroska[] = "video/x-matroska";
const char  SHttpEngine::mimeVideoMpeg[]     = "video/mpeg";
const char  SHttpEngine::mimeVideoMpegM2TS[] = "video/vnd.dlna.mpeg-tts";
const char  SHttpEngine::mimeVideoMpegTS[]   = "video/x-mpegts";
const char  SHttpEngine::mimeVideoMp4[]      = "video/mp4";
const char  SHttpEngine::mimeVideoOgg[]      = "video/ogg";
const char  SHttpEngine::mimeVideoQt[]       = "video/quicktime";
const char  SHttpEngine::mimeVideoWmv[]      = "video/x-ms-wmv";
const char  SHttpEngine::mimeTextCss[]       = "text/css;charset=\"utf-8\"";
const char  SHttpEngine::mimeTextHtml[]      = "text/html;charset=\"utf-8\"";
const char  SHttpEngine::mimeTextJs[]        = "text/javascript;charset=\"utf-8\"";
const char  SHttpEngine::mimeTextPlain[]     = "text/plain;charset=\"utf-8\"";
const char  SHttpEngine::mimeTextXml[]       = "text/xml;charset=\"utf-8\"";

SHttpEngine::SHttpEngine(QObject *parent)
  : QObject(parent)
{
}

SHttpEngine::~SHttpEngine()
{
}

void SHttpEngine::closeSocket(QIODevice *socket)
{
  if (socket)
  {
    if (QThread::currentThread() == thread())
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
    else
    {
      socket->moveToThread(thread());
      qApp->postEvent(this, new CloseSocketEvent(socket));
    }
  }
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

  if      (ext == "js")     return mimeTextJs;
  else if (ext == "pdf")    return "application/pdf";
  else if (ext == "xhtml")  return "application/xhtml+xml";
  else if (ext == "dtd")    return "application/xml-dtd";
  else if (ext == "zip")    return "application/zip";
  else if (ext == "aac")    return mimeAudioAac;
  else if (ext == "ac3")    return mimeAudioAc3;
  else if (ext == "lpcm")   return mimeAudioLpcm;
  else if (ext == "m3u")    return mimeAudioMpegUrl;
  else if (ext == "mpa")    return mimeAudioMpeg;
  else if (ext == "mp2")    return mimeAudioMpeg;
  else if (ext == "mp3")    return mimeAudioMp3;
  else if (ext == "ac3")    return mimeAudioMpeg;
  else if (ext == "dts")    return mimeAudioMpeg;
  else if (ext == "oga")    return mimeAudioOgg;
  else if (ext == "wav")    return mimeAudioWave;
  else if (ext == "wma")    return mimeAudioWma;
  else if (ext == "jpeg")   return mimeImageJpeg;
  else if (ext == "jpg")    return mimeImageJpeg;
  else if (ext == "png")    return mimeImagePng;
  else if (ext == "svg")    return mimeImageSvg;
  else if (ext == "tiff")   return mimeImageTiff;
  else if (ext == "css")    return mimeTextCss;
  else if (ext == "html")   return mimeTextHtml;
  else if (ext == "htm")    return mimeTextHtml;
  else if (ext == "txt")    return mimeTextPlain;
  else if (ext == "log")    return mimeTextPlain;
  else if (ext == "xml")    return mimeTextXml;
  else if (ext == "3g2")    return mimeVideo3g2;
  else if (ext == "asf")    return mimeVideoAsf;
  else if (ext == "avi")    return mimeVideoAvi;
  else if (ext == "m2ts")   return mimeVideoMpegTS;
  else if (ext == "mkv")    return mimeVideoMatroska;
  else if (ext == "mpeg")   return mimeVideoMpeg;
  else if (ext == "mpg")    return mimeVideoMpeg;
  else if (ext == "mp4")    return mimeVideoMp4;
  else if (ext == "ts")     return mimeVideoMpeg;
  else if (ext == "ogg")    return mimeVideoOgg;
  else if (ext == "ogv")    return mimeVideoOgg;
  else if (ext == "ogx")    return mimeVideoOgg;
  else if (ext == "spx")    return mimeVideoOgg;
  else if (ext == "qt")     return mimeVideoQt;
  else if (ext == "flv")    return mimeVideoFlv;
  else if (ext == "wmv")    return mimeVideoWmv;

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

SHttpEngine::MimePartMap SHttpEngine::splitMultipartMime(const QByteArray &content)
{
  MimePartMap result;

  const QByteArray boundary = content.left(content.indexOf("\r\n"));
  if (!boundary.isEmpty())
  for (int i=content.indexOf(boundary), n=-1; i>=0; i=n)
  {
    i += boundary.length() + 2;
    n = content.indexOf(boundary, i);

    if (n > i)
    {
      const QByteArray part = content.mid(i, (n - 2) - i);
      const int d = part.indexOf("\r\n\r\n");
      if (d >= 0)
      {
        MimePart mimePart;
        mimePart.content = part.mid(d + 4);

        foreach (const QByteArray &line, part.left(d).split('\n'))
        if (line.startsWith("Content-Disposition:"))
        foreach (QByteArray head, line.mid(21).split(';'))
        {
          head = head.trimmed();
          const int e = head.indexOf('=');
          if (e > 0)
          {
            const QString name = QString::fromUtf8(head.left(e)).trimmed();
            QString value = QString::fromUtf8(head.mid(e + 1)).trimmed();

            if (value.startsWith('\"') && value.endsWith('\"'))
              value = value.mid(1, value.length() - 2);

            mimePart.fields[name] = value;
          }
        }

        if (mimePart.fields.contains("name"))
          result.insert(mimePart.fields["name"], mimePart);
      }
    }

    if (content.mid(n + boundary.length(), 2) == "--")
      break;
  }

  return result;
}

void SHttpEngine::customEvent(QEvent *e)
{
  if (e->type() == closeSocketEventType)
  {
    const CloseSocketEvent * const event = static_cast<const CloseSocketEvent *>(e);

    closeSocket(event->socket);
  }
  else
    QObject::customEvent(e);
}


struct SHttpServerEngine::Data
{
  QString                       senderId;
  QMap<QString, Callback *>     callbacks;
};

SHttpServerEngine::SHttpServerEngine(const QString &protocol, QObject *parent)
  : SHttpEngine(parent),
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
  if (request.method() == "OPTIONS")
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
  else if (request.method() == "TRACE")
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

void SHttpServerEngine::sendHttpResponse(const SHttpEngine::RequestHeader &request, SHttpEngine::ResponseMessage &response, QIODevice *socket, bool reuse)
{
  struct T
  {
    static void send(const SHttpEngine::RequestHeader &request, SHttpEngine::ResponseMessage &response, QIODevice *socket)
    {
      if (request.isHead())
        response.setContent(QByteArray());

      socket->write(response);

      QAbstractSocket * const aSocket = qobject_cast<QAbstractSocket *>(socket);
      if (aSocket)
        aSocket->flush();

      QLocalSocket * const lSocket = qobject_cast<QLocalSocket *>(socket);
      if (lSocket)
        lSocket->flush();
    }
  };

  if (response.status() != SHttpEngine::Status_None)
  {
    if (reuse && response.canReuseConnection())
    {
      // Reuse the socket if possible
      SHttpServerEngine * const serverEngine =
          qobject_cast<SHttpServerEngine *>(
              const_cast<SHttpEngine *>(request.httpEngine));

      QString hostname; quint16 port = 0;
      if (serverEngine && splitHost(request.host(), hostname, port))
      {
        response.setConnection("Keep-Alive");
        T::send(request, response, socket);

        (new HttpServerRequest(serverEngine, port, __FILE__, __LINE__))->start(socket);

        return;
      }
    }

    response.setConnection("Close");
    T::send(request, response, socket);

    closeSocket(socket);
  }
}

SHttpServerEngine::ResponseMessage SHttpServerEngine::Callback::httpOptions(const RequestMessage &request)
{
  ResponseMessage response(request, Status_Ok);
  response.setField("Allow", "GET,HEAD,POST");

  return response;
}


struct SHttpClientEngine::Data
{
  QString                       senderId;
};

SHttpClientEngine::SHttpClientEngine(QObject *parent)
  : SHttpEngine(parent),
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
  HttpClientRequest * const clientRequest = new HttpClientRequest(this, request.canReuseConnection(), __FILE__, __LINE__);
  connect(clientRequest, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(handleResponse(SHttpEngine::ResponseMessage)));

  openRequest(request, clientRequest, SLOT(start(QIODevice *)));
}

void SHttpClientEngine::handleResponse(const SHttpEngine::ResponseMessage &message)
{
  emit response(message);
}

} // End of namespace
