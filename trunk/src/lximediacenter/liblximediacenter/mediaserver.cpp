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
  static const int              maxStreams = 64;

  MasterServer                * masterServer;
  QList<Stream *>               streams;
  QList<Stream *>               reusableStreams;
  QTimer                        cleanStreamsTimer;
};

const qint32  MediaServer::defaultDirSortOrder  = -65536;
const qint32  MediaServer::defaultFileSortOrder = 0;
const int     MediaServer::seekBySecs = 120;


MediaServer::MediaServer(QObject *parent)
  : BackendServer(parent),
    d(new Data())
{
  d->masterServer = NULL;

  connect(&d->cleanStreamsTimer, SIGNAL(timeout()), SLOT(cleanStreams()));
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

  d->cleanStreamsTimer.start(30000);
}

void MediaServer::close(void)
{
  d->cleanStreamsTimer.stop();

  BackendServer::close();

  d->masterServer->httpServer()->unregisterCallback(this);
  d->masterServer->contentDirectory()->unregisterCallback(this);
}

void MediaServer::cleanStreams(void)
{
  QList<Stream *> obsolete;
  foreach (Stream *stream, d->streams)
  if (!stream->proxy.isConnected())
    obsolete += stream;

  foreach (Stream *stream, obsolete)
    delete stream;
}

SHttpServer::ResponseMessage MediaServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isGet())
  {
    const MediaServer::File file(request);
    if (file.baseName().isEmpty())
    {
      if (file.url().hasQueryItem("items"))
      {
        const QStringList range = file.url().queryItemValue("items").split(',');
        unsigned start = 0, count = 0;
        if (!range.isEmpty())
        {
          start = range[0].toUInt();
          if (range.count() >= 2)
            count = range[1].toUInt();
        }

        ThumbnailListItemList thumbItems;
        foreach (const SUPnPContentDirectory::Item &item, listItems(basePath(file.url().path()), start, count))
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

            if (item.played)
            {
              thumbItem.iconurl.removeQueryItem("overlay");
              thumbItem.iconurl.addQueryItem("overlay", "played");
            }

            thumbItems.append(thumbItem);
          }
        }

        return makeResponse(request, buildThumbnailItems(thumbItems), "text/html;charset=utf-8", false);
      }
      else
        return makeHtmlContent(request, file.url(), buildThumbnailLoader(dirName(file.url().path())), headList);
    }
    else if (!request.isHead())
    {
      foreach (Stream *stream, d->streams)
      if (stream->url == request.path())
      if (stream->proxy.addSocket(socket))
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_None);

      Stream * const stream = streamVideo(request);
      if (stream)
      {
        stream->proxy.addSocket(socket);
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_None);
      }
      else
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
    }
    else // Return head only.
    {
      SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
      response.setField("transferMode.dlna.org", "Streaming");
      if (file.url().hasQueryItem("contentFeatures"))
        response.setField("contentFeatures.dlna.org", QByteArray::fromHex(file.url().queryItemValue("contentFeatures").toAscii()));

      return response;
    }
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
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
  connect(&stream->proxy, SIGNAL(disconnected()), SLOT(cleanStreams()), Qt::QueuedConnection);

  d->streams += stream;
}

void MediaServer::removeStream(Stream *stream)
{
  d->streams.removeAll(stream);
  d->reusableStreams.removeAll(stream);
}

MediaServer::File::File(const SHttpServer::RequestMessage &request)
{
  const QUrl url(request.path());
  const QStringList list = request.file().split("..");
  if (list.count() >= 2)
  {
    const QByteArray q = SUPnPContentDirectory::fromQueryID(list.last().toAscii());
    const QString path = url.path();
    d.url = path.left(path.lastIndexOf('/') + 1) + list.first() + '?' + q;
  }
  else
    d.url = url;

  if (!list.isEmpty())
  {
    QStringList file = list.first().toLower().split('.');

    if (file.count() >= 2)
      d.suffix = file.takeLast();

    if (!file.isEmpty())
      d.baseName = file.join(".");
  }

  if (!d.baseName.isEmpty() || !d.suffix.isEmpty())
    d.fullName = d.baseName + '.' + d.suffix;

  QString path = url.path();
  path = path.left(path.lastIndexOf('/'));
  d.parentDir = path.mid(path.lastIndexOf('/') + 1);
}

MediaServer::File::~File()
{
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


MediaServer::ThumbnailListItem::ThumbnailListItem(void)
  : played(false)
{
}

MediaServer::ThumbnailListItem::~ThumbnailListItem()
{
}


MediaServer::DetailedListItem::DetailedListItem(void)
  : played(false)
{
}

MediaServer::DetailedListItem::~DetailedListItem(void)
{
}


MediaServer::DetailedListItem::Column::Column(QString title, QUrl iconurl, QUrl url)
  : title(title),
    iconurl(iconurl),
    url(url)
{
}

MediaServer::DetailedListItem::Column::~Column()
{
}

} // End of namespace
