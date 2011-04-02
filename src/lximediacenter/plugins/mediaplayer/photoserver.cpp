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

#include "photoserver.h"
#include <LXiStreamGui>
#include "mediaplayersandbox.h"

namespace LXiMediaCenter {

PhotoServer::PhotoServer(MediaDatabase *mediaDatabase, MediaDatabase::Category category, const char *name, Plugin *plugin, MasterServer *server)
  : PlaylistServer(mediaDatabase, category, name, plugin, server, tr("Slideshow"))
{
}

PhotoServer::~PhotoServer()
{
}

PhotoServer::Stream * PhotoServer::streamVideo(const SHttpServer::RequestHeader &request)
{
  const QStringList file = request.file().split('.');
  if (file.first() == "playlist")
  {
    QUrl url(request.path());
    if (url.hasQueryItem("query"))
      url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

    QStringList albums;
    albums += request.directory().mid(httpPath().length() - 1);

    QMultiMap<QString, QByteArray> files;
    while (!albums.isEmpty())
    {
      const QString path = albums.takeFirst();

      foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(MediaDatabase::Category_Photos, path))
      {
        const SMediaInfo node = mediaDatabase->readNode(file.uid);
        if (!node.isNull())
          files.insert(path + file.name, node.toByteArray(-1));
      }

      foreach (const QString &album, mediaDatabase->allAlbums(MediaDatabase::Category_Photos))
      if (album.startsWith(path) && (album.mid(path.length()).count('/') == 1))
        albums += album;
    }

    if (!files.isEmpty())
    {
      QUrl rurl;
      rurl.setPath(MediaPlayerSandbox::path);
      rurl.addQueryItem("playslideshow", QString::null);
      typedef QPair<QString, QString> QStringPair;
      foreach (const QStringPair &queryItem, url.queryItems())
        rurl.addQueryItem(queryItem.first, queryItem.second);

      QByteArray content;
      foreach (const QByteArray &line, files)
        content += line + '\n';

      Stream *stream = new Stream(this, request.path());
      if (stream->setup(rurl, content))
        return stream;

      delete stream;
    }

    return NULL;
  }
  else
    return MediaPlayerServer::streamVideo(request);
}

QList<PhotoServer::Item> PhotoServer::listItems(const QString &path, unsigned start, unsigned count)
{
  QList<Item> items;
  foreach (Item item, PlaylistServer::listItems(path, start, count))
  {
    if (!item.isDir)
    {
      item.played = false; // Not useful for photos.
      item.direct = true;
    }

    items += item;
  }

  return items;
}

SHttpServer::SocketOp PhotoServer::handleHttpRequest(const SHttpServer::RequestHeader &request, QIODevice *socket)
{
  const QUrl url(request.path());
  const QString file = request.file();

  if ((file.endsWith(".jpeg") && !file.endsWith("-thumb.jpeg")) ||
      (file.endsWith(".png") && !file.endsWith("-thumb.png")))
  {
    return sendPhoto(request, socket, MediaDatabase::fromUidString(file), file.split('.').last());
  }
  else if (file.endsWith(".html") && (file != "playlist.html")) // Show photo
    return handleHtmlRequest(request, socket, file);

  return PlaylistServer::handleHttpRequest(request, socket);
}

SHttpServer::SocketOp PhotoServer::sendPhoto(const SHttpServer::RequestHeader &request, QIODevice *socket, MediaDatabase::UniqueID uid, const QString &format) const
{
  const SMediaInfo node = mediaDatabase->readNode(uid);

  if (!node.isNull())
  if (node.containsImage())
  {
    SImage image(node.filePath());
    if (!image.isNull())
    {
      QByteArray jpgData;
      QBuffer buffer(&jpgData);
      buffer.open(QIODevice::WriteOnly);

      unsigned width = 0, height = 0;
      const QUrl url(request.path());
      if (url.hasQueryItem("size"))
      {
        const QStringList size = url.queryItemValue("size").split('x');
        if (size.count() >= 2)
        {
          width = qMax(0, size[0].toInt());
          height = qMax(0, size[1].toInt());
        }
      }

      if ((width > 0) && (height > 0) &&
          ((width < unsigned(image.width())) ||
           (height < unsigned(image.height()))))
      {
        image = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      }

      image.save(&buffer, format.toUpper().toAscii(), 80);
      buffer.close();

      return sendResponse(request, socket, buffer.data(), ("image/" + format.toLower()).toAscii(), true);
    }
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

} // End of namespace
