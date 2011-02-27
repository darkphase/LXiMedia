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

HttpServer::SocketOp PhotoServer::streamVideo(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QStringList file = request.file().split('.');

  if ((file.count() > 1) && (file[1] == "playlist"))
  {
    const QString album = QString::fromUtf8(QByteArray::fromHex(file.first().toAscii()));
    const QList<MediaDatabase::File> files = mediaDatabase->getAlbumFiles(MediaDatabase::Category_Photos, album);
    if (!files.isEmpty())
    {
      SlideShowStream *stream = new SlideShowStream(this, socket->peerAddress(), request.path(), files);
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

QList<PhotoServer::Item> PhotoServer::listItems(const QString &path, unsigned start, unsigned count)
{
  QList<Item> items;
  foreach (Item item, PlaylistServer::listItems(path, start, count))
  {
    if (!item.isDir)
    {
      item.played = false; // Not useful for photos.
      item.mode = Item::Mode_Direct;
    }

    items += item;
  }

  return items;
}

HttpServer::SocketOp PhotoServer::handleHttpRequest(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = request.file();

  if (file.endsWith(".jpeg") && !file.endsWith("-thumb.jpeg"))
  {
    unsigned width = 0, height = 0;
    if (url.hasQueryItem("size"))
    {
      const QStringList size = url.queryItemValue("size").split('x');
      if (size.count() >= 2)
      {
        width = qMax(0, size[0].toInt());
        height = qMax(0, size[1].toInt());
      }
    }

    return sendPhoto(socket, MediaDatabase::fromUidString(file.left(16)), width, height);
  }
  else if (file.endsWith(".html") && !file.endsWith(".playlist.html")) // Show photo
    return handleHtmlRequest(url, file, socket);

  return PlaylistServer::handleHttpRequest(request, socket);
}

HttpServer::SocketOp PhotoServer::sendPhoto(QAbstractSocket *socket, MediaDatabase::UniqueID uid, unsigned width, unsigned height) const
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

      if ((width > 0) && (height > 0) &&
          ((width < unsigned(image.width())) ||
           (height < unsigned(image.height()))))
      {
        image = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      }

      image.save(&buffer, "JPEG", 80);
      buffer.close();

      return sendReply(socket, buffer.data(), "image/jpeg", true);
    }
  }

  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}


PhotoServer::SlideShowStream::SlideShowStream(PhotoServer *parent, const QHostAddress &peer, const QString &url, const QList<MediaDatabase::File> &files)
  : Stream(parent, peer, url),
    slideShow(this, files, parent->mediaDatabase)
{
  connect(&slideShow, SIGNAL(finished()), SLOT(stop()));
  connect(&slideShow, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
  connect(&slideShow, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
}

bool PhotoServer::SlideShowStream::setup(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
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
