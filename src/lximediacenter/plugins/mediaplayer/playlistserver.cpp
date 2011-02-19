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

#include "playlistserver.h"
#include <LXiStreamGui>

namespace LXiMediaCenter {

PlaylistServer::PlaylistServer(MediaDatabase *mediaDatabase, MediaDatabase::Category category, const char *name, Plugin *plugin, MasterServer *server, const QString &itemTitle)
  : MediaPlayerServer(mediaDatabase, category, name, plugin, server),
    itemTitle(itemTitle)
{
}

PlaylistServer::~PlaylistServer()
{
}

HttpServer::SocketOp PlaylistServer::streamVideo(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QStringList file = request.file().split('.');

  if ((file.count() > 1) && (file[1] == "playlist"))
  {
    QStringList albums;
    albums += QString::fromUtf8(QByteArray::fromHex(file.first().toAscii()));

    QMultiMap<QString, SMediaInfo> files;
    while (!albums.isEmpty())
    {
      const QString path = albums.takeFirst();

      foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, path))
      {
        const SMediaInfo node = mediaDatabase->readNode(file.uid);
        if (!node.isNull())
        {
          const QDateTime lastPlayed = mediaDatabase->lastPlayed(node.filePath());
          const QString key =
              (lastPlayed.isValid() ? lastPlayed.toString("yyyyMMddhhmmss") : QString("00000000000000")) +
              path +
              ("000000000" + QString::number(node.track())).right(10) +
              SStringParser::toRawName(node.title());

          files.insert(key, node);
        }
      }

      foreach (const QString &album, mediaDatabase->allAlbums(category))
      if (album.startsWith(path) && (album.mid(path.length()).count('/') == 1))
        albums += album;
    }

    if (!files.isEmpty())
    {
      PlaylistStream *stream = new PlaylistStream(this, socket->peerAddress(), request.path(), files.values());
      if (stream->setup(request, socket))
      if (stream->start())
        return HttpServer::SocketOp_LeaveOpen; // The graph owns the socket now.

      delete stream;
    }

    qWarning() << "Failed to start stream" << request.path();
    socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
    return HttpServer::SocketOp_Close;
  }
  else
    return MediaPlayerServer::streamVideo(request, socket);
}

int PlaylistServer::countItems(const QString &path)
{
  if (mediaDatabase->countAlbumFiles(category, path) > 0)
    return MediaPlayerServer::countItems(path) + 1;
  else
    return MediaPlayerServer::countItems(path);
}

QList<PlaylistServer::Item> PlaylistServer::listItems(const QString &path, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;
  QList<Item> result = listAlbums(path, start, count);

  if (returnAll || (count > 0))
  {
    result += listPlayAllItem(path, start, count);

    if (returnAll || (count > 0))
    foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, path, start, count))
      result.append(makeItem(file.uid));
  }

  return result;
}

HttpServer::SocketOp PlaylistServer::handleHttpRequest(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = request.file();

  if (file.endsWith(".playlist.html")) // Show player
  {
    const QByteArray item = file.left(file.length() - 5).toAscii();
    const QString album = QString::fromUtf8(QByteArray::fromHex(file.left(file.length() - 15).toAscii()));
    if (!album.isEmpty())
    {
      HttpServer::ResponseHeader response(HttpServer::Status_Ok);
      response.setContentType("text/html;charset=utf-8");
      response.setField("Cache-Control", "no-cache");

      const QString albumName = album.mid(album.lastIndexOf('/') + 1);

      HtmlParser htmlParser;

      htmlParser.setField("PLAYER", buildVideoPlayer(item, albumName, url));

      htmlParser.setField("PLAYER_INFOITEMS", QByteArray(""));
      htmlParser.setField("ITEM_NAME", tr("Title"));
      htmlParser.setField("ITEM_VALUE", albumName);

      htmlParser.setField("PLAYER_DESCRIPTION_NAME", QByteArray(""));
      htmlParser.setField("PLAYER_DESCRIPTION", QByteArray(""));

      return sendHtmlContent(socket, url, response, htmlParser.parse(htmlPlayer), headPlayer);
    }
  }

  return MediaPlayerServer::handleHttpRequest(request, socket);
}

QList<PlaylistServer::Item> PlaylistServer::listPlayAllItem(const QString &path,  unsigned &start, unsigned &count, MediaDatabase::UniqueID thumbUid)
{
  QList<Item> result;

  if (start == 0)
  {
    Item item;
    item.mode = Item::Mode_Direct;
    item.title = itemTitle;
    item.mimeType = "video/mpeg";
    item.url = httpPath() + path.toUtf8().toHex() + ".playlist";

    if (thumbUid == 0)
    {
      const QList<MediaDatabase::File> first = mediaDatabase->getAlbumFiles(category, path, 0, 1);
      if (!first.isEmpty())
        thumbUid = first.first().uid;
    }

    if (thumbUid != 0)
    {
      item.iconUrl = makeItem(thumbUid).iconUrl;
      item.iconUrl.addQueryItem("overlay", "arrow-right");
    }

    result.append(item);
    count--;
  }
  else
    start--;

  return result;
}


PlaylistServer::PlaylistStream::PlaylistStream(PlaylistServer *parent, const QHostAddress &peer, const QString &url, const SMediaInfoList &files)
  : MediaPlayerServer::TranscodeStream(parent, peer, url),
    playlistNode(this, files),
    streamHelper(this, static_cast<PlaylistServer *>(parent)->mediaDatabase)
{
  connect(&playlistNode, SIGNAL(finished()), SLOT(stop()));
  connect(&playlistNode, SIGNAL(opened(QString)), &streamHelper, SLOT(opened(QString)));
  connect(&playlistNode, SIGNAL(closed(QString)), &streamHelper, SLOT(closed(QString)));
  connect(&playlistNode, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&playlistNode, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&playlistNode, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
}

PlaylistServer::PlaylistStream::~PlaylistStream()
{
}

bool PlaylistServer::PlaylistStream::setup(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  return TranscodeStream::setup(request, socket, &playlistNode);
}


PlaylistServerStreamHelper::PlaylistServerStreamHelper(QObject *parent, MediaDatabase *mediaDatabase)
  : QObject(parent),
    mediaDatabase(mediaDatabase)
{
}

PlaylistServerStreamHelper::~PlaylistServerStreamHelper()
{
  if (!currentFile.isEmpty())
  if (startTime.secsTo(QDateTime::currentDateTime()) >= 120)
    mediaDatabase->setLastPlayed(currentFile);
}

void PlaylistServerStreamHelper::opened(const QString &filePath)
{
  currentFile = filePath;
  startTime = QDateTime::currentDateTime();
}

void PlaylistServerStreamHelper::closed(const QString &filePath)
{
  mediaDatabase->setLastPlayed(filePath);

  if (currentFile == filePath)
    currentFile = QString::null;
}

} // End of namespace
