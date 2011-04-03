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

#include "mediaserver.h"
#include "globalsettings.h"
#include "htmlparser.h"
#include "mediastream.h"
#include <LXiStreamGui>

namespace LXiMediaCenter {

struct MediaServer::Data
{
  class StreamEvent : public QEvent
  {
  public:
    inline                      StreamEvent(QEvent::Type type, const SHttpServer::RequestHeader &request, QIODevice *socket, QSemaphore *sem)
        : QEvent(type), request(request), socket(socket), sem(sem) { }

    const SHttpServer::RequestHeader request;
    QIODevice     * const socket;
    QSemaphore          * const sem;
  };

  inline                        Data(void) : mutex(QMutex::Recursive)        { }

  static const QEvent::Type     startStreamEventType;
  static const int              maxStreams = 64;

  MasterServer                * masterServer;
  QMutex                        mutex;
  QList<Stream *>               streams;
  QList<Stream *>               reusableStreams;
};

const QEvent::Type MediaServer::Data::startStreamEventType = QEvent::Type(QEvent::registerEventType());

const qint32  MediaServer::defaultDirSortOrder  = -65536;
const qint32  MediaServer::defaultFileSortOrder = 0;
const int     MediaServer::seekBySecs = 120;


MediaServer::MediaServer(QObject *parent)
  : BackendServer(parent),
    d(new Data())
{
  d->masterServer = NULL;
}

MediaServer::~MediaServer()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void MediaServer::initialize(MasterServer *masterServer)
{
  d->masterServer = masterServer;

  BackendServer::initialize(masterServer);

  d->masterServer->httpServer()->registerCallback(serverPath(), this);
  d->masterServer->contentDirectory()->registerCallback(serverPath(), this);
}

void MediaServer::close(void)
{
  BackendServer::close();

  d->masterServer->httpServer()->unregisterCallback(this);
  d->masterServer->contentDirectory()->unregisterCallback(this);
}

void MediaServer::customEvent(QEvent *e)
{
  if (e->type() == d->startStreamEventType)
  {
    QMutexLocker l(&d->mutex);

    Data::StreamEvent * const event = static_cast<Data::StreamEvent *>(e);
    const QString url = event->request.path();

    foreach (Stream *stream, d->streams)
    if (stream->url == url)
    if (stream->proxy.addSocket(event->socket))
    {
      event->sem->release();
      return;
    }

    Stream * const stream = streamVideo(event->request);
    if (stream)
    {
      stream->proxy.addSocket(event->socket);
    }
    else
    {
      SHttpServer::sendResponse(event->request, event->socket, SHttpServer::Status_NotFound, this);
      event->socket->close();
    }

    event->sem->release();
  }
}

void MediaServer::cleanStreams(void)
{
  QMutexLocker l(&d->mutex);

  QList<Stream *> obsolete;
  foreach (Stream *stream, d->streams)
  if (!stream->proxy.isConnected())
    obsolete += stream;

  foreach (Stream *stream, obsolete)
    delete stream;
}

SHttpServer::SocketOp MediaServer::handleHttpRequest(const SHttpServer::RequestHeader &request, QIODevice *socket)
{
  const QUrl url(request.path());
  const QString file = request.file();

  if (file.isEmpty())
  {
    SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
    response.setContentType("text/html;charset=utf-8");
    response.setField("Cache-Control", "no-cache");

    QString basePath = url.path().mid(serverPath().length());
    basePath = basePath.startsWith('/') ? basePath : ('/' + basePath);

    const int total = countItems(basePath);
    const int start = url.queryItemValue("start").toInt();

    ThumbnailListItemList thumbItems;

    foreach (const SUPnPContentDirectory::Item &item, listItems(basePath, start, itemsPerThumbnailPage))
    {
      if (item.isDir)
      {
        ThumbnailListItem thumbItem;
        thumbItem.title = item.title;
        thumbItem.iconurl = item.iconUrl;
        thumbItem.url = item.title + '/';

        thumbItems.append(thumbItem);
      }
      else
      {
        ThumbnailListItem thumbItem;
        thumbItem.title = item.title;
        thumbItem.iconurl = item.iconUrl;
        thumbItem.url = item.url;
        thumbItem.url.setPath(thumbItem.url.path() + ".html");
        thumbItem.played = item.played;

        thumbItems.append(thumbItem);
      }
    }

    return sendHtmlContent(socket, url, response, buildThumbnailView(basePath, thumbItems, start, total), headList);
  }
  else
  {
    QSemaphore sem(0);

    socket->moveToThread(QObject::thread());
    QCoreApplication::postEvent(this, new Data::StreamEvent(Data::startStreamEventType, request, socket, &sem));

    sem.acquire();
    return SHttpServer::SocketOp_LeaveOpen; // Socket will be closed by event handler
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

int MediaServer::countContentDirItems(const QString &dirPath)
{
  QString subPath = dirPath.mid(serverPath().length());
  subPath = subPath.startsWith('/') ? subPath : ('/' + subPath);

  return countItems(subPath);
}

QList<SUPnPContentDirectory::Item> MediaServer::listContentDirItems(const QString &dirPath, unsigned start, unsigned count)
{
  QString subPath = dirPath.mid(serverPath().length());
  subPath = subPath.startsWith('/') ? subPath : ('/' + subPath);

  QString basePath = serverPath();
  basePath = basePath.endsWith('/') ? basePath.left(basePath.length() - 1) : basePath;
  basePath += subPath;
  basePath = basePath.endsWith('/') ? basePath.left(basePath.length() - 1) : basePath;

  QList<SUPnPContentDirectory::Item> result;
  foreach (Item item, listItems(subPath, start, count))
  {
    const QString itemPath = item.url.path(), iconPath = item.iconUrl.path();

    if (!itemPath.isEmpty())
      item.url.setPath(itemPath.startsWith('/') ? itemPath : (basePath + '/' + itemPath));

    if (!iconPath.isEmpty())
      item.iconUrl.setPath(iconPath.startsWith('/') ? iconPath : (basePath + '/' + iconPath));

    result += item;
  }

  return result;
}

void MediaServer::addStream(Stream *stream)
{
  QMutexLocker l(&d->mutex);

  connect(&stream->proxy, SIGNAL(disconnected()), SLOT(cleanStreams()), Qt::QueuedConnection);

  d->streams += stream;
}

void MediaServer::removeStream(Stream *stream)
{
  QMutexLocker l(&d->mutex);

  d->streams.removeAll(stream);
  d->reusableStreams.removeAll(stream);
}

MediaServer::Stream::Stream(MediaServer *parent, const QString &url)
  : parent(parent),
    url(url),
    proxy()
{
  parent->addStream(this);
}

MediaServer::Stream::~Stream()
{
  parent->removeStream(this);
}

} // End of namespace
