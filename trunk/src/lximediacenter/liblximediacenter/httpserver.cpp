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

#include "httpserver.h"

#include <QtNetwork>

namespace LXiMediaCenter {

const char  * const HttpServer::fieldContentLength  = "Content-Length";
const char  * const HttpServer::fieldContentType    = "Content-Type";
const char  * const HttpServer::fieldHost           = "Host";

class HttpServer::Interface : public QTcpServer
{
public:
  explicit                      Interface(const QHostAddress &, quint16, HttpServer *parent);

protected:
  virtual void                  incomingConnection(int);

private:
  HttpServer            * const parent;
};

class HttpServer::SocketHandler : public QRunnable
{
public:
                                SocketHandler(HttpServer *, int);

protected:
  virtual void                  run();

private:
  static const int              maxTtl = 60000;

  HttpServer            * const parent;
  const int                     socketDescriptor;
  QTime                         timer;
  QByteArray                    response;
};

struct HttpServer::Private
{
  static const int              maxThreads = 50;

  QReadWriteLock                lock;
  QList<QHostAddress>           addresses;
  quint16                       port;
  QMap<QString, Interface *>    interfaces;
  QThreadPool                   threadPool;
  QAtomicInt                    numPendingConnections;
  QSemaphore                  * startSem;
  QMap<QString, Callback *>     callbacks;
};


HttpServer::HttpServer(void)
  : QThread(),
    p(new Private())
{
  p->threadPool.setMaxThreadCount(Private::maxThreads);
  p->numPendingConnections = 0;
  p->startSem = NULL;
}

HttpServer::~HttpServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void HttpServer::initialize(const QList<QHostAddress> &addresses, quint16 port)
{
  p->addresses = addresses;
  p->port = port;

  QSemaphore sem(0);
  p->startSem = &sem;

  moveToThread(this);
  start();

  sem.acquire(1);
  p->startSem = NULL;
}

void HttpServer::close(void)
{
  quit();
  wait();
}

quint16 HttpServer::serverPort(const QHostAddress &address) const
{
  SDebug::WriteLocker l(&(p->lock), __FILE__, __LINE__);

  QMap<QString, Interface *>::ConstIterator i = p->interfaces.find(address.toString());
  if (i != p->interfaces.end())
    return (*i)->serverPort();

  return 0;
}

void HttpServer::registerCallback(const QString &path, Callback *callback)
{
  SDebug::WriteLocker l(&(p->lock), __FILE__, __LINE__);

  p->callbacks.insert(path, callback);
}

void HttpServer::unregisterCallback(Callback *callback)
{
  SDebug::WriteLocker l(&(p->lock), __FILE__, __LINE__);

  for (QMap<QString, Callback *>::Iterator i=p->callbacks.begin(); i!=p->callbacks.end(); )
  if (i.value() == callback)
    i = p->callbacks.erase(i);
  else
    i++;
}

const char * HttpServer::toMimeType(const QString &fileName)
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
  else if (ext == "ogv")    return "video/ogg";
  else if (ext == "ogx")    return "video/ogg";
  else if (ext == "spx")    return "video/ogg";
  else if (ext == "qt")     return "video/quicktime";
  else if (ext == "flv")    return "video/x-flv";

  // For licenses
  else if (fileName.startsWith("COPYING")) return "text/plain";

  else                      return "application/octet-stream";
}

const char * HttpServer::statusText(int code)
{
  switch (code)
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

void HttpServer::run(void)
{
  {
    SDebug::WriteLocker l(&(p->lock), __FILE__, __LINE__);

    foreach (const QHostAddress &address, p->addresses)
      p->interfaces[address.toString()] = new Interface(address, p->port, this);

    p->startSem->release(1);
  }

  exec();

  {
    SDebug::WriteLocker l(&(p->lock), __FILE__, __LINE__);

    foreach (Interface *iface, p->interfaces)
      delete iface;

    p->interfaces.clear();
  }

  p->threadPool.waitForDone();
}


HttpServer::Interface::Interface(const QHostAddress &interfaceAddr, quint16 port, HttpServer *parent)
  : QTcpServer(),
    parent(parent)
{
  if (port > 0)
  if (QTcpServer::listen(interfaceAddr, port))
    return;

  if (!QTcpServer::listen(interfaceAddr))
    qWarning() << "Failed to bind interface" << interfaceAddr.toString();
}

void HttpServer::Interface::incomingConnection(int socketDescriptor)
{
  if (parent->p->numPendingConnections < 1024)
  {
    parent->p->threadPool.start(new SocketHandler(parent, socketDescriptor));
  }
  else // Overloaded
  {
    QTcpSocket socket;
    if (socket.setSocketDescriptor(socketDescriptor))
      socket.abort();
  }
}


HttpServer::SocketHandler::SocketHandler(HttpServer *parent, int socketDescriptor)
  : parent(parent),
    socketDescriptor(socketDescriptor)
{
  parent->p->numPendingConnections.ref();
  timer.start();
}

void HttpServer::SocketHandler::run()
{
  parent->p->numPendingConnections.deref();

  QTcpSocket *socket = new QTcpSocket();
  if (socket->setSocketDescriptor(socketDescriptor))
  while (socket->canReadLine() || socket->waitForReadyRead(qMax(maxTtl - qAbs(timer.elapsed()), 0)))
  {
    const QByteArray line = socket->readLine();

    if (line.trimmed().length() > 0)
    {
      response += line;
    }
    else // Header complete
    {
      response += line;

      const RequestHeader request(response);
      if (request.isValid())
      {
        SDebug::ReadLocker l(&(parent->p->lock), __FILE__, __LINE__);

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
            socket = NULL; // The file accepted the connection, it is responsible for closing and deleteing it.
        }
        else
          socket->write(ResponseHeader(Status_NotFound));
      }
      else
        socket->write(ResponseHeader(Status_BadRequest));

      // Finished.
      if (socket)
      {
        if (socket->state() == QAbstractSocket::ConnectedState)
        {
          socket->waitForBytesWritten();
          socket->disconnectFromHost();
          if (socket->state() != QAbstractSocket::UnconnectedState)
            socket->waitForDisconnected();
        }

        delete socket;
      }

      return;
    }
  }

  if (socket)
    delete socket;
}


HttpServer::Header::Header(const QByteArray &header)
{
  QList<QByteArray> lines = header.split('\r');
  if (!lines.isEmpty())
    head = lines.takeFirst().split(' ');

  foreach (const QByteArray &line, lines) // Note: each line starts with a \n.
  {
    const int colon = line.indexOf(':');
    if (colon > 0)
    {
      fields.insert(
          QString::fromUtf8(line.mid(1, colon - 1)).trimmed(),
          QString::fromUtf8(line.mid(colon + 1)).trimmed());
    }
  }
}

QByteArray HttpServer::Header::toUtf8(void) const
{
  QByteArray result;
  if (isValid())
  {
    result = head[0] + ' ' + head[1] + ' ' + head[2] + "\r\n";

    for (QMap<QString, QString>::ConstIterator i=fields.begin(); i!=fields.end(); i++)
      result += i.key().toUtf8() + ": " + i.value().toUtf8() + "\r\n";

    result += "\r\n";
  }

  return result;
}


HttpServer::RequestHeader::RequestHeader(void)
{
  head << "GET" << "/" << "HTTP/1.1";
}

QString HttpServer::RequestHeader::file(void) const
{
  QByteArray result = path();

  const int q = result.indexOf('?');
  if (q >= 0)
    result = result.left(q);

  const int s = result.lastIndexOf('/');
  if (s > 0)
    result = result.mid(s + 1);

  return QString::fromUtf8(QByteArray::fromPercentEncoding(result));
}

HttpServer::ResponseHeader::ResponseHeader(Status status)
{
  head << "HTTP/1.1" << QByteArray::number(status) << statusText(status);
}

HttpServer::ResponseHeader::ResponseHeader(const QByteArray &header)
  : Header(header)
{
  while (head.count() > 3)
    head[2] += " " + head.takeAt(3);

}

void HttpServer::ResponseHeader::setStatus(Status status)
{
  if (head.size() > 1)
    head[1] = QByteArray::number(status);
  else
    head.append(QByteArray::number(status));

  if (head.size() > 2)
    head[2] = statusText(status);
  else
    head.append(statusText(status));
}


} // End of namespace
