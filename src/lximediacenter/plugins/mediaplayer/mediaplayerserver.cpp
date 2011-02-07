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
  setRoot(new MediaPlayerServerDir(this, "/"));
}

bool MediaPlayerServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = url.path().mid(url.path().lastIndexOf('/') + 1);

  if (file.endsWith("-thumb.jpeg"))
  {
    const SMediaInfo node = mediaDatabase->readNode(MediaDatabase::fromUidString(file.left(16)));
    if (!node.isNull())
    if (!node.thumbnails().isEmpty())
    {
      if (!url.hasQueryItem("size"))
      {
        return sendReply(socket, node.thumbnails().first(), "image/jpeg", true);
      }
      else
      {
        const QStringList sizeTxt = url.queryItemValue("size").split('x');
        QSize size(256, 256);
        if (sizeTxt.count() >= 2)
          size = QSize(sizeTxt[0].toInt(), sizeTxt[1].toInt());
        else if (sizeTxt.count() >= 1)
          size = QSize(sizeTxt[0].toInt(), sizeTxt[0].toInt());

        QImage image = QImage::fromData(node.thumbnails().first());
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QBuffer b;
        image.save(&b, "JPEG", 50);

        return sendReply(socket, b.data(), "image/jpeg", true);
      }
    }

    QHttpResponseHeader response(301);
    response.setValue("Location", "http://" + request.value("Host") + "/img/null.png");
    socket->write(response.toString().toUtf8());
    return false;
  }
  else if (file.endsWith(".html")) // Show player
  {
    const MediaDatabase::UniqueID uid = MediaDatabase::fromUidString(file.left(16));
    const SMediaInfo node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      QHttpResponseHeader response(200);
      response.setContentType("text/html;charset=utf-8");
      response.setValue("Cache-Control", "no-cache");

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

  return MediaServer::handleConnection(request, socket);
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

bool MediaPlayerServer::streamVideo(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());

  MediaDatabase::UniqueID uid = 0;
  if (url.hasQueryItem("item"))
  {
    uid = MediaDatabase::fromUidString(url.queryItemValue("item"));
  }
  else
  {
    const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');
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
          return true; // The graph owns the socket now.

        delete stream;
      }
      else
      {
        FileStream *stream = new FileStream(this, socket->peerAddress(), request.path(), node.filePath(), uid);
        if (stream->file.open())
        if (stream->setup(request, socket, &stream->file, node.duration(), node.title(), thumb))
        if (stream->start())
          return true; // The graph owns the socket now.

        delete stream;
      }
    }
  }

  socket->write(QHttpResponseHeader(404).toString().toUtf8());

  qWarning() << "Failed to start stream" << request.path();
  return false;
}

bool MediaPlayerServer::buildPlaylist(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');

  HtmlParser htmlParser;
  htmlParser.setField("ITEMS", QByteArray(""));

  const QString server = "http://" + request.value("Host") + httpPath();

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

  QHttpResponseHeader response(200);
  response.setContentType("audio/x-mpegurl");
  response.setValue("Cache-Control", "no-cache");
  socket->write(response.toString().toUtf8());
  socket->write(htmlParser.parse(m3uPlaylist));

  return false;
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


MediaPlayerServerDir::MediaPlayerServerDir(MediaPlayerServer *parent, const QString &albumPath)
  : MediaServerDir(parent),
    albumPath(albumPath.endsWith('/') ? albumPath : (albumPath + '/'))
{
}

QStringList MediaPlayerServerDir::listDirs(void)
{
  QSet<QString> albums;
  foreach (const QString &album, server()->mediaDatabase->allAlbums(server()->category))
  if (album.startsWith(albumPath))
  {
    const QString subAlbum = album.mid(albumPath.length()).split('/').first();
    if (!subAlbum.isEmpty())
      albums.insert(subAlbum);
  }

  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  QStringList addAlbums = albums.toList();
  QStringList delAlbums;

  foreach (const QString &dirName, MediaServerDir::listDirs())
  {
    if (!albums.contains(dirName))
      delAlbums.append(dirName);
    else
      addAlbums.removeAll(dirName);
  }

  foreach (const QString &album, delAlbums)
    removeDir(album);

  foreach (const QString &album, addAlbums)
    addDir(album, createDir(server(), albumPath + album));

  return MediaServerDir::listDirs();
}

QStringList MediaPlayerServerDir::listFiles(void)
{
  QMap<QString, File> files;
  foreach (MediaDatabase::UniqueID uid, server()->mediaDatabase->allAlbumFiles(server()->category, albumPath))
  {
    const SMediaInfo node = server()->mediaDatabase->readNode(uid);
    const SMediaInfo titleNode = (!node.isNull() && node.isDisc()) ? node.titles().first() : node;

    if (!node.isNull() && !titleNode.isNull())
    {
      File file;
      if (titleNode.containsAudio() || titleNode.containsVideo())
        file.mimeType = "video/mpeg";
      else
        file.mimeType = "image/jpeg";

      if (!file.mimeType.isEmpty())
      {
        file.played = server()->mediaDatabase->lastPlayed(uid).isValid();
        file.url = server()->httpPath() + MediaDatabase::toUidString(uid) + "." + file.mimeType.right(4);
        file.iconUrl = server()->httpPath() + MediaDatabase::toUidString(uid) + "-thumb.jpeg";
        file.mediaInfo = titleNode;

        files.insert(node.title(), file);
      }
    }
  }

  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  QStringList addFiles = files.keys();
  QStringList delFiles;

  foreach (const QString &fileName, MediaServerDir::listFiles())
  {
    if (!files.contains(fileName))
      delFiles.append(fileName);
    else
      addFiles.removeAll(fileName);
  }

  foreach (const QString &fileName, delFiles)
    removeFile(fileName);

  foreach (const QString &fileName, addFiles)
    addFile(fileName, files[fileName]);

  return MediaServerDir::listFiles();
}

QString MediaPlayerServerDir::getIcon(void) const
{
  for (unsigned i=0; i<16; i++)
  {
    const MediaDatabase::UniqueID uid = server()->mediaDatabase->getAlbumFile(server()->category, albumPath, i);
    if (uid == 0)
      break;
    else if (!server()->mediaDatabase->readNode(uid).isNull())
      return server()->httpPath() + MediaDatabase::toUidString(uid) + "-thumb.jpeg";
  }

  return QString::null;
}

MediaPlayerServerDir * MediaPlayerServerDir::createDir(MediaPlayerServer *parent, const QString &albumPath)
{
  return new MediaPlayerServerDir(parent, albumPath);
}

} // End of namespace
