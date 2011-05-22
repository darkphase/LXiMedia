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
#include "mediaplayersandbox.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

PlaylistServer::PlaylistServer(MediaDatabase::Category category, QObject *parent, const QString &itemTitle)
  : MediaPlayerServer(category, parent),
    itemTitle(itemTitle)
{
}

PlaylistServer::~PlaylistServer()
{
}

PlaylistServer::Stream * PlaylistServer::streamVideo(const SHttpServer::RequestMessage &request)
{
  const QStringList file = request.file().split('.');
  if (file.first() == "playlist")
  {
    QUrl url(request.path());
    if (url.hasQueryItem("query"))
      url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

    SSandboxClient * const sandbox = masterServer->createSandbox((url.queryItemValue("priority") == "low") ? SSandboxClient::Mode_Nice : SSandboxClient::Mode_Normal);
    connect(sandbox, SIGNAL(consoleLine(QString)), SLOT(consoleLine(QString)));
    sandbox->ensureStarted();

    QStringList albums;
    albums += request.directory().mid(serverPath().length() - 1);

    QMultiMap<QString, QByteArray> files;
    while (!albums.isEmpty())
    {
      const QString path = albums.takeFirst();

      foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, path))
      {
        const FileNode node = mediaDatabase->readNode(file.uid);
        if (!node.isNull())
        {
          const QDateTime lastPlayed = mediaDatabase->lastPlayed(node.filePath());
          const QString key =
              (lastPlayed.isValid() ? lastPlayed.toString("yyyyMMddhhmmss") : QString("00000000000000")) +
              ("000000000" + QString::number(node.track())).right(10) +
              path +
              SStringParser::toRawName(node.title());

          files.insert(key, node.toByteArray(-1));
        }
      }

      foreach (const QString &album, mediaDatabase->allAlbums(category))
      if (album.startsWith(path) && (album.mid(path.length()).count('/') == 1))
        albums += album;
    }

    if (!files.isEmpty())
    {
      QUrl rurl;
      rurl.setPath(MediaPlayerSandbox::path);
      rurl.addQueryItem("playlist", QString::null);
      typedef QPair<QString, QString> QStringPair;
      foreach (const QStringPair &queryItem, url.queryItems())
        rurl.addQueryItem(queryItem.first, queryItem.second);

      QByteArray content;
      foreach (const QByteArray &line, files)
        content += line + '\n';

      Stream *stream = new Stream(this, sandbox, request.path());
      if (stream->setup(rurl, content))
        return stream;

      delete stream;
    }

    disconnect(sandbox, SIGNAL(consoleLine(QString)), this, SLOT(consoleLine(QString)));
    masterServer->recycleSandbox(sandbox);

    return NULL;
  }
  else
    return MediaPlayerServer::streamVideo(request);
}

int PlaylistServer::countItems(const QString &path)
{
  const int result = MediaPlayerServer::countItems(path);

  return (result > 0) ? (result + 1) : 0;
}

QList<PlaylistServer::Item> PlaylistServer::listItems(const QString &path, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;
  QList<Item> result = listAlbums(path, start, count);

  if (returnAll || (count > 0))
  {
    result += listPlayAllItem(path, start, count, 0, result);

    if (returnAll || (count > 0))
    foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, path, start, count))
      result.append(makeItem(file.uid));
  }

  return result;
}

SHttpServer::SocketOp PlaylistServer::handleHttpRequest(const SHttpServer::RequestMessage &request, QAbstractSocket *socket)
{
  if ((request.method() == "GET") || (request.method() == "HEAD"))
  {
    const QUrl url(request.path());
    const QString file = request.file();

    if (file == "playlist.html") // Show player
    {
      const QString album = QUrl(request.directory().mid(serverPath().length() - 1)).path();
      if (!album.isEmpty())
      {
        SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
        response.setContentType("text/html;charset=utf-8");
        response.setField("Cache-Control", "no-cache");

        return sendHtmlContent(request, socket, url, response, buildVideoPlayer("playlist", dirName(album), url), headPlayer);
      }
    }
  }

  return MediaPlayerServer::handleHttpRequest(request, socket);
}

QList<PlaylistServer::Item> PlaylistServer::listPlayAllItem(const QString &path,  unsigned &start, unsigned &count, MediaDatabase::UniqueID thumbUid, const QList<Item> &thumbs)
{
  QList<Item> result;

  if (!MediaPlayerServer::isEmpty(path))
  {
    if (start == 0)
    {
      Item item;
      item.direct = true;

      item.type = defaultItemType();
      if ((item.type == Item::Type_Image) || (item.type == Item::Type_Photo))
        item.type = Item::Type_Video;

      item.url = "playlist";

      item.title = itemTitle;

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
      else foreach (const Item &thumb, thumbs)
      if (!thumb.iconUrl.isEmpty())
      {
        item.iconUrl = thumb.iconUrl;
        item.iconUrl.removeQueryItem("overlay");
        item.iconUrl.addQueryItem("overlay", "arrow-right");
        break;
      }

      result.append(item);
      count--;
    }
    else
      start--;
  }

  return result;
}

} } // End of namespaces
