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

#include "mediaplayerserver.h"

namespace LXiMediaCenter {

MediaPlayerServer::MediaPlayerServer(MediaDatabase *mediaDatabase, MediaDatabase::Category category, const char *name, Plugin *plugin, BackendServer::MasterServer *server)
  : MediaServer(name, plugin, server),
    mediaDatabase(mediaDatabase),
    category(category)
{
}

/*void MediaPlayerServer::addVideoFile(DlnaServerDir *dir, const PlayItem &item, const QString &name, int sortOrder) const
{
  if (!item.mediaInfo.duration().isValid() || (item.mediaInfo.duration().toSec() < 10 * 60))
  {
    DlnaServer::File file(dir->server());
    file.date = item.mediaInfo.lastModified();
    file.url = httpPath() + MediaDatabase::toUidString(item.uid) + ".mpeg";
    file.iconUrl = httpPath() + MediaDatabase::toUidString(item.uid) + "-thumb.jpeg";
    file.mimeType = "video/mpeg";
    file.sortOrder = sortOrder;
    file.played = mediaDatabase->lastPlayed(item.uid).isValid();

    dir->addFile(name, file);
  }
  else
    addVideoFile(dir, QList<PlayItem>() << item, name, sortOrder);
}

void MediaPlayerServer::addVideoFile(DlnaServerDir *dir, const QList<PlayItem> &items, const QString &name, int sortOrder) const
{
  if (!items.isEmpty())
  {
    MediaServerFileDir * const fileRootDir = new MediaServerFileDir(dir->server());
    fileRootDir->played = true;

    const QList<SMediaInfo::AudioStreamInfo> audioStreams = items.first().mediaInfo.audioStreams();
    const QList<SMediaInfo::VideoStreamInfo> videoStreams = items.first().mediaInfo.videoStreams();
    QList<SMediaInfo::DataStreamInfo> dataStreams = items.first().mediaInfo.dataStreams();
    dataStreams +=
        SIOInputNode::DataStreamInfo(
            SIOInputNode::DataStreamInfo::Type_Subtitle, 0xFFFF,
            NULL, SDataCodec());

    for (int a=0, an=audioStreams.count(); a < an; a++)
    for (int d=0, dn=dataStreams.count(); d < dn; d++)
    {
      DlnaServerDir * const fileDir = ((an + dn) == 2) ? fileRootDir : new DlnaServerDir(fileRootDir->server());
      DlnaServerDir * const chapterDir = new DlnaServerDir(fileDir->server());
      DlnaServerDir * const seekDir = new DlnaServerDir(fileDir->server());

      typedef QPair<QString, PlayItem> VideoFile;
      QList<VideoFile> videoFiles;
      if (items.count() > 1)
      {
        unsigned i = 1;
        foreach (const PlayItem &item, items)
          videoFiles += VideoFile(tr("Play part") + " " + QString::number(i++), item);
      }
      else
        videoFiles += VideoFile(tr("Play"), items.first());

      int chapterNum = 1;
      int seekSec = 0, seekOfs = 0;
      foreach (const VideoFile &videoFile, videoFiles)
      {
        QString url = httpPath() + MediaDatabase::toUidString(videoFile.second.uid) + ".mpeg";
        url += "?language=" + QString::number(audioStreams[a], 16) + "&subtitles=";
        if (dataStreams[d].streamId() != 0xFFFF)
          url += QString::number(dataStreams[d], 16);

        DlnaServer::File file(dir->server());
        file.date = videoFile.second.mediaInfo.lastModified();
        file.duration = videoFile.second.mediaInfo.duration();
        file.url = url;
        file.iconUrl = httpPath() + MediaDatabase::toUidString(videoFile.second.uid) + "-thumb.jpeg";
        file.mimeType = "video/mpeg";
        //file.description = video.plot;
        file.sortOrder = 1;
        fileDir->addFile(videoFile.first, file);

        fileRootDir->played &= mediaDatabase->lastPlayed(videoFile.second.uid).isValid();

        foreach (const SMediaInfo::Chapter &chapter, videoFile.second.mediaInfo.chapters())
        {
          DlnaServer::File file(chapterDir->server());
          file.date = videoFile.second.mediaInfo.lastModified();
          file.duration = videoFile.second.mediaInfo.duration() - chapter.begin;
          file.url = url + (url.contains('?') ? "&position=" : "?position=") +
                      QString::number(chapter.begin.toSec());
          file.mimeType = "video/mpeg";
          file.sortOrder = chapterNum++;

          QString name = tr("Chapter") + " " + QString::number(file.sortOrder);
          if (!chapter.title.isEmpty())
            name += ", " + chapter.title;

          chapterDir->addFile(name, file);
        }

        seekOfs += videoFile.second.mediaInfo.duration().toSec();
        for (; seekSec<seekOfs; seekSec+=seekBySecs)
        {
          DlnaServer::File file(seekDir->server());
          file.date = videoFile.second.mediaInfo.lastModified();
          file.duration = videoFile.second.mediaInfo.duration() - STime::fromSec(seekSec);
          file.url = url + (url.contains('?') ? "&position=" : "?position=") +
                      QString::number(seekSec);
          file.mimeType = "video/mpeg";
          file.sortOrder = seekSec;

          seekDir->addFile(tr("Play from") + " " + QString::number(seekSec / 3600) +
                           ":" + ("0" + QString::number((seekSec / 60) % 60)).right(2),
                           file);
        }

        fileDir->date = fileDir->date.isValid()
                      ? qMax(fileDir->date, videoFile.second.mediaInfo.lastModified())
                      : videoFile.second.mediaInfo.lastModified();
      }

      if (chapterDir->count() > 0)
      {
        chapterDir->sortOrder = 2;
        fileDir->addDir(tr("Chapters"), chapterDir);
      }
      else
        delete chapterDir;

      if (seekDir->count() > 0)
      {
        seekDir->sortOrder = 3;
        fileDir->addDir(tr("Seek"), seekDir);
      }
      else
        delete seekDir;

      if (fileDir != fileRootDir)
      {
        fileDir->sortOrder = (a + 1) * 2;
        QString name;
        if (an == 1) // One audio stream
        {
          if (dataStreams[d].streamId() != 0xFFFF)
          {
            fileDir->sortOrder -= 1;
            name += tr("With subtitles");
            if (dataStreams[d].language[0])
            {
              name += " (";
              if (dn > 2)
                name += QString::number(d + 1) + ". ";

              name += SStringParser::iso639Language(dataStreams[d].language) + ")";
            }
            else if (dn > 2)
              name += " (" + QString::number(d + 1) + ")";
          }
          else
            name += tr("Without subtitles");
        }
        else
        {
          name += QString::number(a + 1) + ". ";
          if (audioStreams[a].language[0])
            name += SStringParser::iso639Language(audioStreams[a].language);
          else
            name += tr("Unknown");

          if (dataStreams[d].streamId() != 0xFFFF)
          {
            fileDir->sortOrder -= 1;
            name += " " + tr("with subtitles");
            if (dataStreams[d].language[0])
            {
              name += " (";
              if (dn > 2)
                name += QString::number(d + 1) + ". ";

              name += SStringParser::iso639Language(dataStreams[d].language) + ")";
            }
            else if (dn > 2)
              name += " (" + QString::number(d + 1) + ")";
          }
        }

        fileRootDir->addDir(name, fileDir);
      }
    }

    foreach (const PlayItem &item, items)
      fileRootDir->uids += item.uid;

    fileRootDir->sortOrder = sortOrder;
    dir->addDir(name, fileRootDir);
  }
}*/

HttpServer::SocketOp MediaPlayerServer::streamVideo(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());

  MediaDatabase::UniqueID uid = 0;
  if (url.hasQueryItem("item"))
  {
    uid = MediaDatabase::fromUidString(url.queryItemValue("item"));
  }
  else
  {
    const QStringList file = request.file().split('.');
    if (file.count() >= 2)
      uid = MediaDatabase::fromUidString(file.first());
  }

  if (uid != 0)
  {
    const SMediaInfo node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      QImage thumb;
      if (!node.thumbnails().isEmpty())
        thumb = QImage::fromData(node.thumbnails().first());

      // Create a new stream
      if (node.isDisc())
      {
        DiscStream *stream = new DiscStream(this, socket->peerAddress(), request.path(), node.filePath(), uid);
        if (stream->disc.playTitle(0))
        if (stream->setup(request, socket, &stream->disc, node.duration(), node.title(), thumb))
        if (stream->start())
          return HttpServer::SocketOp_LeaveOpen; // The graph owns the socket now.

        delete stream;
      }
      else
      {
        FileStream *stream = new FileStream(this, socket->peerAddress(), request.path(), node.filePath(), uid);
        if (stream->file.open())
        if (stream->setup(request, socket, &stream->file, node.duration(), node.title(), thumb))
        if (stream->start())
          return HttpServer::SocketOp_LeaveOpen; // The graph owns the socket now.

        delete stream;
      }
    }
  }

  qWarning() << "Failed to start stream" << request.path();
  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}

HttpServer::SocketOp MediaPlayerServer::buildPlaylist(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QStringList file = request.file().split('.');

  HtmlParser htmlParser;
  htmlParser.setField("ITEMS", QByteArray(""));

  const QString server = "http://" + request.host() + httpPath();

  if (file.count() >= 2)
  {
    const MediaDatabase::UniqueID uid = MediaDatabase::fromUidString(file.first());
    const SMediaInfo node = mediaDatabase->readNode(uid);

    htmlParser.setField("ITEM_LENGTH", QTime().addSecs(node.duration().toSec()).toString(videoTimeFormat));

    if (!node.title().isEmpty())
    {
      if (!node.author().isEmpty())
        htmlParser.setField("ITEM_NAME", node.title() + " [" + node.author() + "]");
      else
        htmlParser.setField("ITEM_NAME", node.title());
    }
    else
      htmlParser.setField("ITEM_NAME", node.fileName());

    htmlParser.setField("ITEM_URL", server.toAscii() + MediaDatabase::toUidString(uid) + ".mpeg");
    htmlParser.appendField("ITEMS", htmlParser.parse(m3uPlaylistItem));
  }

  HttpServer::ResponseHeader response(HttpServer::Status_Ok);
  response.setContentType("audio/x-mpegurl");
  response.setField("Cache-Control", "no-cache");
  socket->write(response);
  socket->write(htmlParser.parse(m3uPlaylist));
  return HttpServer::SocketOp_Close;
}

int MediaPlayerServer::countItems(const QString &path)
{
  return countAlbums(path) + mediaDatabase->countAlbumFiles(category, path);
}

QList<MediaPlayerServer::Item> MediaPlayerServer::listItems(const QString &path, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;
  QList<Item> result = listAlbums(path, start, count);

  if (returnAll || (count > 0))
  foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, path, start, count))
    result.append(makeItem(file.uid));

  return result;
}

int MediaPlayerServer::countAlbums(const QString &path)
{
  int albums = 0;
  foreach (const QString &album, mediaDatabase->allAlbums(category))
  if (album.startsWith(path) && (album.mid(path.length()).count('/') == 1))
    albums++;

  return albums;
}

QList<MediaPlayerServer::Item> MediaPlayerServer::listAlbums(const QString &path,  unsigned &start, unsigned &count)
{
  QList<Item> result;

  foreach (const QString &album, mediaDatabase->allAlbums(category))
  if (album.startsWith(path) && (album.mid(path.length()).count('/') == 1))
  {
    Item item;
    item.isDir = true;
    item.title = album.mid(path.length(), album.length() - path.length() - 1);

    foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, album, 0, 8))
    if (!mediaDatabase->readNode(file.uid).thumbnails().isEmpty())
    {
      item.iconUrl = httpPath() + MediaDatabase::toUidString(file.uid) + "-thumb.jpeg?overlay=folder-video";
      break;
    }

    result += item;
  }

  if (count > 0)
  {
    while ((start > 0) && !result.isEmpty())
    {
      result.takeFirst();
      start--;
    }

    if (result.count() >= int(count))
    {
      while (result.count() > int(count))
        result.takeAt(count);

      count = 0;
    }
    else
      count -= result.count();
  }

  return result;
}

MediaPlayerServer::Item MediaPlayerServer::makeItem(MediaDatabase::UniqueID uid)
{
  Item item;
  const SMediaInfo node = mediaDatabase->readNode(uid);
  const SMediaInfo titleNode = (!node.isNull() && node.isDisc()) ? node.titles().first() : node;

  if (!node.isNull() && !titleNode.isNull())
  {
    if (titleNode.containsAudio() || titleNode.containsVideo())
      item.mimeType = "video/mpeg";
    else
      item.mimeType = "image/jpeg";

    if (!item.mimeType.isEmpty())
    {
      item.played = mediaDatabase->lastPlayed(uid).isValid();
      item.title = node.title();
      item.url = MediaDatabase::toUidString(uid) + "." + item.mimeType.right(4);
      item.iconUrl = MediaDatabase::toUidString(uid) + "-thumb.jpeg";
      item.mediaInfo = titleNode;

      const ImdbClient::Entry imdbEntry = mediaDatabase->getImdbEntry(uid);
      if (!imdbEntry.isNull())
      {
        item.title = imdbEntry.title;
        if (imdbEntry.year > 0)
          item.title += " " + QString::number(imdbEntry.year);

        if (imdbEntry.rating > 0)
          item.title += " [" + QString::number(imdbEntry.rating, 'f', 1) + "]";
      }
    }
  }

  return item;
}

HttpServer::SocketOp MediaPlayerServer::handleHttpRequest(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = request.file();

  if (file.endsWith("-thumb.jpeg"))
  {
    QSize size(256, 256);
    if (url.hasQueryItem("size"))
    {
      const QStringList sizeTxt = url.queryItemValue("size").split('x');
      if (sizeTxt.count() >= 2)
        size = QSize(sizeTxt[0].toInt(), sizeTxt[1].toInt());
      else if (sizeTxt.count() >= 1)
        size = QSize(sizeTxt[0].toInt(), sizeTxt[0].toInt());
    }

    const SMediaInfo node = mediaDatabase->readNode(MediaDatabase::fromUidString(file.left(16)));
    if (!node.isNull())
    if (!node.thumbnails().isEmpty())
    {
      if (!url.hasQueryItem("size") && !url.hasQueryItem("overlay"))
      {
        return sendReply(socket, node.thumbnails().first(), "image/jpeg", true);
      }
      else
      {
        QImage image = QImage::fromData(node.thumbnails().first());
        if (url.hasQueryItem("size"))
          image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        if (url.hasQueryItem("overlay"))
        {
          QImage overlayImage(":/mediaplayer/" + url.queryItemValue("overlay") + ".png");
          if (!overlayImage.isNull())
          {
            if (url.hasQueryItem("size"))
              overlayImage = overlayImage.scaled(size / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            QImage sum(size, QImage::Format_ARGB32);
            QPainter p;
            p.begin(&sum);

            p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha

            p.fillRect(sum.rect(), Qt::transparent);

            p.drawImage(
                (sum.width() / 2) - (image.width() / 2),
                (sum.height() / 2) - (image.height() / 2),
                image);

            p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha

            p.drawImage(
                (sum.width() / 2) - (overlayImage.width() / 2),
                (sum.height() / 2) - (overlayImage.height() / 2),
                overlayImage);

            p.end();
            image = sum;
          }
        }

        QBuffer b;
        image.save(&b, "PNG");

        return sendReply(socket, b.data(), "image/png", true);
      }
    }

    QImage image(":/mediaplayer/mediafile.png");
    if (!image.isNull())
    {
      if (url.hasQueryItem("size"))
        image = image.scaled(size / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);

      QImage sum(size, QImage::Format_ARGB32);
      QPainter p;
      p.begin(&sum);

      p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha

      p.fillRect(sum.rect(), Qt::transparent);

      p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha

      p.drawImage(
          (sum.width() / 2) - (image.width() / 2),
          (sum.height() / 2) - (image.height() / 2),
          image);

      p.end();

      QBuffer b;
      sum.save(&b, "PNG");

      return sendReply(socket, b.data(), "image/png", true);
    }

    HttpServer::ResponseHeader response(HttpServer::Status_MovedPermanently);
    response.setField("Location", "http://" + request.host() + "/img/null.png");
    socket->write(response);
    return HttpServer::SocketOp_Close;
  }
  else if (file.endsWith(".html")) // Show player
  {
    const MediaDatabase::UniqueID uid = MediaDatabase::fromUidString(file.left(16));
    const SMediaInfo node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      HttpServer::ResponseHeader response(HttpServer::Status_Ok);
      response.setContentType("text/html;charset=utf-8");
      response.setField("Cache-Control", "no-cache");

      HtmlParser htmlParser;

      htmlParser.setField("PLAYER", buildVideoPlayer(uid, node, url));

      htmlParser.setField("PLAYER_INFOITEMS", QByteArray(""));
      htmlParser.setField("ITEM_NAME", tr("Title"));
      htmlParser.setField("ITEM_VALUE", node.title());
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Duration"));
      htmlParser.setField("ITEM_VALUE", QTime().addSecs(node.duration().toSec()).toString(videoTimeFormat));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Format"));
      htmlParser.setField("ITEM_VALUE", videoFormatString(node));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Filename"));
      htmlParser.setField("ITEM_VALUE", node.fileName());
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));

      htmlParser.setField("PLAYER_DESCRIPTION_NAME", tr("Description"));
      htmlParser.setField("PLAYER_DESCRIPTION", node.comment());

      return sendHtmlContent(socket, url, response, htmlParser.parse(htmlPlayer), headPlayer);
    }
  }

  return MediaServer::handleHttpRequest(request, socket);
}

QString MediaPlayerServer::videoFormatString(const SMediaInfo &mediaInfo)
{
  if (!mediaInfo.videoStreams().isEmpty())
  {
    const SVideoCodec codec = mediaInfo.videoStreams().first().codec;
    if (!codec.size().isNull())
    {
      QString s = QString::number(codec.size().width()) + " x " +
                  QString::number(codec.size().height());

      if (codec.frameRate().isValid())
        s +=  " @ " + QString::number(codec.frameRate().toFrequency(), 'f', 2) + " fps";

      return s;
    }
  }

  return tr("Unknown");
}

QByteArray MediaPlayerServer::buildVideoPlayer(MediaDatabase::UniqueID uid, const SMediaInfo &node, const QUrl &url, const QSize &size)
{
  if (node.isDisc())
    return MediaServer::buildVideoPlayer(MediaDatabase::toUidString(uid), node.titles().first(), url, size);
  else
    return MediaServer::buildVideoPlayer(MediaDatabase::toUidString(uid), node, url, size);
}

QByteArray MediaPlayerServer::buildVideoPlayer(const QByteArray &item, const QString &title, const QUrl &url, const QSize &size)
{
  return MediaServer::buildVideoPlayer(item, title, url, size);
}


MediaPlayerServer::FileStream::FileStream(MediaPlayerServer *parent, const QHostAddress &peer, const QString &url, const QString &fileName, MediaDatabase::UniqueID uid)
  : TranscodeStream(parent, peer, url),
    startTime(QDateTime::currentDateTime()),
    uid(uid),
    file(this, fileName)
{
  connect(&file, SIGNAL(finished()), SLOT(stop()));

  connect(&file, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&file, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&file, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
}

MediaPlayerServer::FileStream::~FileStream()
{
  if (startTime.secsTo(QDateTime::currentDateTime()) >= 120)
    static_cast<MediaPlayerServer *>(parent)->mediaDatabase->setLastPlayed(uid);
}


MediaPlayerServer::DiscStream::DiscStream(MediaPlayerServer *parent, const QHostAddress &peer, const QString &url, const QString &devicePath, MediaDatabase::UniqueID uid)
  : TranscodeStream(parent, peer, url),
    startTime(QDateTime::currentDateTime()),
    uid(uid),
    disc(this, devicePath)
{
  connect(&disc, SIGNAL(finished()), SLOT(stop()));

  connect(&disc, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&disc, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&disc, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
}

MediaPlayerServer::DiscStream::~DiscStream()
{
  if (startTime.secsTo(QDateTime::currentDateTime()) >= 120)
    static_cast<MediaPlayerServer *>(parent)->mediaDatabase->setLastPlayed(uid);
}

} // End of namespace
