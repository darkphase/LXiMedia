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

PhotoServer::PhotoServer(MediaDatabase *mediaDatabase, MediaDatabase::Category category, const char *name, Plugin *plugin, BackendServer::MasterServer *server)
  : MediaPlayerServer(mediaDatabase, category, name, plugin, server)
{
}

PhotoServer::~PhotoServer()
{
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
  else if (file.endsWith(".html")) // Show player
    return handleHtmlRequest(url, file, socket);

  return MediaPlayerServer::handleHttpRequest(request, socket);
}

HttpServer::SocketOp PhotoServer::streamVideo(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');

  if (file.count() >= 2)
  {
    const QString album = QString::fromUtf8(QByteArray::fromHex(file.first().toAscii()));
    const QList<MediaDatabase::File> files = mediaDatabase->getAlbumFiles(MediaDatabase::Category_Photos, album);
    if (!files.isEmpty())
    {
      QString title;
      QImage thumb;
      QStringList fileNames;
      foreach (const MediaDatabase::File &file, files)
      {
        const SMediaInfo node = mediaDatabase->readNode(file.uid);
        if (!node.isNull())
        {
          if (fileNames.isEmpty())
          if (!node.thumbnails().isEmpty())
            thumb = QImage::fromData(node.thumbnails().first());

          fileNames.append(node.filePath());

          if (title.isEmpty())
          {
            QDir parentDir(node.filePath());
            parentDir.cdUp();

            title = parentDir.dirName();
          }
        }
      }

      SlideShowStream *stream = new SlideShowStream(this, socket->peerAddress(), request.path(), fileNames);
      if (stream->setup(request, socket, stream->slideShow.duration(), title, thumb))
      if (stream->start())
        return HttpServer::SocketOp_LeaveOpen; // The graph owns the socket now.

      delete stream;
    }
  }

  qWarning() << "Failed to start stream" << request.path();
  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}

int PhotoServer::countItems(const QString &path)
{
  if (mediaDatabase->countAlbumFiles(category, path) > 0)
    return MediaPlayerServer::countItems(path) + 1;
  else
    return MediaPlayerServer::countItems(path);
}

QList<PhotoServer::Item> PhotoServer::listItems(const QString &path, unsigned start, unsigned count)
{
  if (mediaDatabase->countAlbumFiles(category, path) > 0)
  {
    if (start == 0)
    {
      QList<Item> result;
      if (count > 1)
        result = MediaPlayerServer::listItems(path, start, count - 1);
      else if (count == 0)
        result = MediaPlayerServer::listItems(path, start, count);

      Item item;
      item.title = tr("Start Slideshow");
      item.mimeType = "video/mpeg";
      item.url = httpPath() + path.toUtf8().toHex() + ".slideshow.mpeg";

      if (result.isEmpty())
      {
        const QList<Item> first = MediaPlayerServer::listItems(path, 0, 1);
        if (!first.isEmpty())
        {
          item.iconUrl = first.first().iconUrl;
          item.iconUrl.addQueryItem("overlay", "slideshow");
        }
      }
      else
      {
        item.iconUrl = result.first().iconUrl;
        item.iconUrl.addQueryItem("overlay", "slideshow");
      }

      result.prepend(item);

      return result;
    }
    else
      return MediaPlayerServer::listItems(path, start - 1, count);
  }
  else
    return MediaPlayerServer::listItems(path, start, count);
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


PhotoServer::SlideShowStream::SlideShowStream(PhotoServer *parent, const QHostAddress &peer, const QString &url, const QStringList &fileNames)
  : Stream(parent, peer, url),
    slideShow(this, fileNames)
{
  connect(&slideShow, SIGNAL(finished()), SLOT(stop()));
  connect(&slideShow, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
  connect(&slideShow, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
}

bool PhotoServer::SlideShowStream::setup(const HttpServer::RequestHeader &request, QAbstractSocket *socket, STime duration, const QString &name, const QImage &thumb)
{
  const QUrl url(request.path());
  const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');

  const SInterval frameRate = SInterval::fromFrequency(slideShow.frameRate);

  SSize size = slideShow.size();
  if (url.hasQueryItem("size"))
  {
    const QStringList sizeTxt = url.queryItemValue("size").split('/').first().split('x');
    if (sizeTxt.count() >= 2)
    {
      size.setWidth(sizeTxt[0].toInt());
      size.setHeight(sizeTxt[1].toInt());
      if (sizeTxt.count() >= 3)
        size.setAspectRatio(sizeTxt[2].toFloat());
      else
        size.setAspectRatio(1.0f);

      slideShow.setSize(size);
    }
  }

  const SInterfaces::AudioEncoder::Flags audioEncodeFlags = SInterfaces::AudioEncoder::Flag_Fast;
  const SInterfaces::VideoEncoder::Flags videoEncodeFlags = SInterfaces::VideoEncoder::Flag_Fast;

  HttpServer::ResponseHeader header(HttpServer::Status_Ok);
  header.setField("Cache-Control", "no-cache");

  if ((file.last().toLower() == "mpeg") || (file.last().toLower() == "mpg"))
  {
    header.setContentType("video/MP2P");

    const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channel_Stereo, 48000);
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    if (!videoEncoder.openCodec(SVideoCodec("MPEG2", size, frameRate), videoEncodeFlags))
    if (!videoEncoder.openCodec(SVideoCodec("MPEG1", size, frameRate), videoEncodeFlags))
      return false;

    output.openFormat("vob", audioEncoder.codec(), videoEncoder.codec(), duration);
  }
  else if ((file.last().toLower() == "ogg") || (file.last().toLower() == "ogv"))
  {
    header.setContentType("video/ogg");

    const SAudioCodec audioOutCodec("FLAC", SAudioFormat::Channel_Stereo, 44100);
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    const SVideoCodec videoOutCodec("THEORA", size, frameRate);
    if (!videoEncoder.openCodec(videoOutCodec, videoEncodeFlags))
      return false;

    output.openFormat("ogg", audioEncoder.codec(), videoEncoder.codec(), duration);
  }
  else if (file.last().toLower() == "flv")
  {
    header.setContentType("video/x-flv");

    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    const SVideoCodec videoOutCodec("FLV1", size, frameRate);
    if (!videoEncoder.openCodec(videoOutCodec, videoEncodeFlags))
      return false;

    output.openFormat("flv", audioEncoder.codec(), videoEncoder.codec(), duration);
  }
  else
    return false;

  Stream::setup(url.queryItemValue("header") == "true", name, thumb);

  output.setHeader(header);
  output.addSocket(socket);
  return true;
}

} // End of namespace
