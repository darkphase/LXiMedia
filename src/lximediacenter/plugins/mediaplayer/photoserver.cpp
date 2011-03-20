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

namespace LXiMediaCenter {

PhotoServer::PhotoServer(MediaDatabase *mediaDatabase, MediaDatabase::Category category, const char *name, Plugin *plugin, MasterServer *server)
  : PlaylistServer(mediaDatabase, category, name, plugin, server, tr("Slideshow"))
{
}

PhotoServer::~PhotoServer()
{
}

HttpServer::SocketOp PhotoServer::streamVideo(const HttpServer::RequestHeader &request, QIODevice *socket)
{
  const QStringList file = request.file().split('.');
  if (file.first() == "playlist")
  {
    QStringList albums;
    albums += request.directory().mid(httpPath().length() - 1);

    QMultiMap<QString, MediaDatabase::File> files;
    while (!albums.isEmpty())
    {
      const QString path = albums.takeFirst();

      foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(MediaDatabase::Category_Photos, path))
        files.insert(path + file.name, file);

      foreach (const QString &album, mediaDatabase->allAlbums(MediaDatabase::Category_Photos))
      if (album.startsWith(path) && (album.mid(path.length()).count('/') == 1))
        albums += album;
    }

    if (!files.isEmpty())
    {
      SlideShowStream *stream = new SlideShowStream(this, request.path(), files.values());
      if (stream->setup(request, socket))
      if (stream->start())
        return HttpServer::SocketOp_LeaveOpen; // The graph owns the socket now.

      delete stream;
    }

    return HttpServer::sendResponse(request, socket, HttpServer::Status_NotFound, this);
  }
  else
    return MediaPlayerServer::streamVideo(request, socket);
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

HttpServer::SocketOp PhotoServer::handleHttpRequest(const HttpServer::RequestHeader &request, QIODevice *socket)
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

HttpServer::SocketOp PhotoServer::sendPhoto(const HttpServer::RequestHeader &request, QIODevice *socket, MediaDatabase::UniqueID uid, const QString &format) const
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

  return HttpServer::sendResponse(request, socket, HttpServer::Status_NotFound, this);
}


PhotoServer::SlideShowStream::SlideShowStream(PhotoServer *parent, const QString &url, const QList<MediaDatabase::File> &files)
  : Stream(parent, url),
    slideShow(this, files, parent->mediaDatabase)
{
  connect(&slideShow, SIGNAL(finished()), SLOT(stop()));
  connect(&slideShow, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
  connect(&slideShow, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
}

bool PhotoServer::SlideShowStream::setup(const HttpServer::RequestHeader &request, QIODevice *socket)
{
  if (Stream::setup(request, socket,
                    slideShow.duration(),
                    SInterval::fromFrequency(slideShow.frameRate), slideShow.size(),
                    SAudioFormat::Channel_Stereo))
  {
    slideShow.setSize(videoResizer.size());

    return true;
  }
  else
    return false;
}

} // End of namespace
