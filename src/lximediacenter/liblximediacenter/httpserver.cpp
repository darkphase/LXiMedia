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

  QMutex                        mutex;
  QList<QHostAddress>           addresses;
  quint16                       port;
  QMap<QString, Interface *>    interfaces;
  QThreadPool                   threadPool;
  QAtomicInt                    numPendingConnections;
  QSemaphore                  * startSem;
};


HttpServer::HttpServer(void)
  : QThread(),
    p(new Private())
{
  p->threadPool.setMaxThreadCount(Private::maxThreads);
  p->numPendingConnections = 0;
  p->startSem = NULL;

  setRoot(new HttpServerDir(this));
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
  SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

  QMap<QString, Interface *>::ConstIterator i = p->interfaces.find(address.toString());
  if (i != p->interfaces.end())
    return (*i)->serverPort();

  return 0;
}

bool HttpServer::addFile(const QString &path, const QString &realFile)
{
  SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

  Q_ASSERT(!path.isEmpty());
  Q_ASSERT(!realFile.isEmpty());

  const int lastSlash = path.lastIndexOf('/');
  if (lastSlash >= 0)
  {
    const QString dir = path.left(lastSlash + 1);
    const QString file = path.mid(lastSlash + 1);

    HttpServerDir * const d = qobject_cast<HttpServerDir *>(findDir(dir));
    if (d)
    {
      d->addFile(file, realFile);
      return true;
    }
  }

  return false;
}

QString HttpServer::toMimeType(const QString &fileName)
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

void HttpServer::run(void)
{
  {
    SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

    foreach (const QHostAddress &address, p->addresses)
      p->interfaces[address.toString()] = new Interface(address, p->port, this);

    p->startSem->release(1);
  }

  exec();

  {
    SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

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

      const QHttpRequestHeader requestHeader(QString::fromUtf8(response));
      if (requestHeader.isValid())
      {
        const QUrl url(requestHeader.path());
        if (!url.path().isEmpty())
        {
          const QString path = url.path().left(url.path().lastIndexOf('/') + 1);

          HttpServerDir * const dir = qobject_cast<HttpServerDir *>(parent->findDir(path));
          if (dir)
          {
            if (dir->handleConnection(requestHeader, socket))
              socket = NULL; // The file accepted the connection, it is responsible for closing and deleteing it.
          }
          else
            socket->write(QHttpResponseHeader(404).toString().toUtf8());
        }
        else
          socket->write(QHttpResponseHeader(404).toString().toUtf8());
      }
      else
        socket->write(QHttpResponseHeader(400).toString().toUtf8());

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


HttpServerDir::HttpServerDir(HttpServer *parent)
  : FileServerDir(parent)
{
  Q_ASSERT(parent);
}

void HttpServerDir::addFile(const QString &name, const QString &realFile)
{
  Q_ASSERT(!name.isEmpty());
  Q_ASSERT(!realFile.isEmpty());

  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  files[name] = realFile;
}

/*! Is invoked for an incoming HTTP request on this directory when queryFile()
    returns NULL.

    \note Invoked from a thread from the threadpool of the HTTP server.
 */
bool HttpServerDir::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = url.path().mid(url.path().lastIndexOf('/') + 1);

  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  QMap<QString, QString>::ConstIterator i = files.find(file);
  if (i != files.end())
  {
    QFile file(*i);
    if (file.open(QFile::ReadOnly))
    {
      QHttpResponseHeader response(200);
      response.setContentType(HttpServer::toMimeType(file.fileName()));
      response.setContentLength(file.size());

      socket->write(response.toString().toUtf8());
      for (QByteArray data=file.read(65536); !data.isEmpty(); data=file.read(65536))
      {
        if (socket->waitForBytesWritten())
          socket->write(data);
        else
          break;
      }

      file.close();
      return false;
    }
  }

  qDebug() << "HttpServer: Failed to find file:" << request.path();
  socket->write(QHttpResponseHeader(404).toString().toUtf8());
  return false;
}


} // End of namespace
