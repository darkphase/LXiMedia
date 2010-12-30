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

namespace LXiMediaCenter {

const int MediaServer::seekBySecs = 120;

MediaServer::MediaServer(const char *name, MediaDatabase *mediaDatabase, Plugin *plugin, BackendServer::MasterServer *server)
            :VideoServer(name, plugin, server),
             mediaDatabase(mediaDatabase)
{
}

bool MediaServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = url.path().mid(url.path().lastIndexOf('/') + 1);

  if (file.isEmpty() || file.endsWith(".html"))
  {
    return handleHtmlRequest(url, file, socket);
  }
  else if (file.endsWith("-thumb.jpeg"))
  {
    const MediaDatabase::Node node = mediaDatabase->readNode(MediaDatabase::fromUidString(file.left(16)));
    if (!node.isNull())
    if (!node.mediaInfo.thumbnails().isEmpty())
    {
      if (!url.hasQueryItem("size"))
      {
        return sendReply(socket, node.mediaInfo.thumbnails().first(), "image/jpeg", true);
      }
      else
      {
        const QStringList sizeTxt = url.queryItemValue("size").split('x');
        QSize size(256, 256);
        if (sizeTxt.count() >= 2)
          size = QSize(sizeTxt[0].toInt(), sizeTxt[1].toInt());
        else if (sizeTxt.count() >= 1)
          size = QSize(sizeTxt[0].toInt(), sizeTxt[0].toInt());

        QImage image = QImage::fromData(node.mediaInfo.thumbnails().first());
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

  return VideoServer::handleConnection(request, socket);
}

void MediaServer::enableDlna(void)
{
  VideoServer::enableDlna();
}

void MediaServer::addVideoFile(DlnaServerDir *dir, const MediaDatabase::Node &node, const QString &name, int sortOrder) const
{
  if (!node.mediaInfo.duration().isValid() || (node.mediaInfo.duration().toSec() < 10 * 60))
  {
    DlnaServer::File file(dir->server());
    file.date = node.lastModified;
    file.url = httpPath() + MediaDatabase::toUidString(node.uid) + ".mpeg";
    file.iconUrl = httpPath() + MediaDatabase::toUidString(node.uid) + "-thumb.jpeg";
    file.mimeType = "video/mpeg";
    file.sortOrder = sortOrder;

    dir->addFile(name, file);
  }
  else
    addVideoFile(dir, QList<MediaDatabase::Node>() << node, name, sortOrder);
}

void MediaServer::addVideoFile(DlnaServerDir *dir, const QList<MediaDatabase::Node> &nodes, const QString &name, int sortOrder) const
{
  if (!nodes.isEmpty())
  {
    MediaServerFileDir * const fileRootDir = new MediaServerFileDir(dir->server());

    const QList<SInterfaces::FormatProber::AudioStreamInfo> audioStreams = nodes.first().mediaInfo.audioStreams();
    const QList<SInterfaces::FormatProber::VideoStreamInfo> videoStreams = nodes.first().mediaInfo.videoStreams();
    QList<SInterfaces::FormatProber::DataStreamInfo> dataStreams = nodes.first().mediaInfo.dataStreams();
    dataStreams += SIOInputNode::DataStreamInfo(0xFFFF, NULL, SDataCodec());

    for (int a=0, an=audioStreams.count(); a < an; a++)
    for (int d=0, dn=dataStreams.count(); d < dn; d++)
    {
      DlnaServerDir * const fileDir = ((an + dn) == 2) ? fileRootDir : new DlnaServerDir(fileRootDir->server());
      DlnaServerDir * const seekDir = new DlnaServerDir(fileDir->server());

      typedef QPair<QString, MediaDatabase::Node> VideoFile;
      QList<VideoFile> videoFiles;
      if (nodes.count() > 1)
      {
        unsigned i = 1;
        foreach (const MediaDatabase::Node &node, nodes)
          videoFiles += VideoFile(tr("Play part") + " " + QString::number(i++), node);
      }
      else
        videoFiles += VideoFile(tr("Play"), nodes.first());

      int seekSec = 0, seekOfs = 0;
      foreach (const VideoFile &videoFile, videoFiles)
      if (!videoFile.second.isNull())
      {
        QString url = httpPath() + MediaDatabase::toUidString(videoFile.second.uid) + ".mpeg";
        url += "?language=" + QString::number(audioStreams[a].streamId, 16) + "&subtitles=";
        if (dataStreams[d].streamId != 0xFFFF)
          url += QString::number(dataStreams[d].streamId, 16);

        DlnaServer::File file(dir->server());
        file.date = videoFile.second.lastModified;
        file.duration = videoFile.second.mediaInfo.duration();
        file.url = url;
        file.iconUrl = httpPath() + MediaDatabase::toUidString(videoFile.second.uid) + "-thumb.jpeg";
        file.mimeType = "video/mpeg";
        //file.description = video.plot;
        fileDir->addFile(videoFile.first, file);

        seekOfs += videoFile.second.mediaInfo.duration().toSec();
        for (; seekSec<seekOfs; seekSec+=seekBySecs)
        {
          DlnaServer::File file(dir->server());
          file.duration = videoFile.second.mediaInfo.duration() - STime::fromSec(seekSec);
          file.url = url + (url.contains('?') ? "&position=" : "?position=") +
                      QString::number(seekSec);
          file.mimeType = "video/mpeg";
          seekDir->addFile(tr("Play from") + " " + QString::number(seekSec / 3600) +
                           ":" + ("0" + QString::number((seekSec / 60) % 60)).right(2),
                           file);
        }

        fileDir->date = fileDir->date.isValid()
                      ? qMax(fileDir->date, videoFile.second.lastModified)
                      : videoFile.second.lastModified;
      }

      if (seekDir->count() > 0)
        fileDir->addDir(tr("Seek"), seekDir);
      else
        delete seekDir;

      if (fileDir != fileRootDir)
      {
        fileDir->sortOrder = (a + 1) * 2;
        QString name;
        if (an == 1) // One audio stream
        {
          if (dataStreams[d].streamId != 0xFFFF)
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

          if (dataStreams[d].streamId != 0xFFFF)
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

    foreach (const MediaDatabase::Node &node, nodes)
      fileRootDir->uids += node.uid;

    fileRootDir->sortOrder = sortOrder;
    dir->addDir(name, fileRootDir);
  }
}

bool MediaServer::streamVideo(const QHttpRequestHeader &request, QAbstractSocket *socket)
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
    const MediaDatabase::Node node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      QImage thumb;
      if (!node.mediaInfo.thumbnails().isEmpty())
        thumb = QImage::fromData(node.mediaInfo.thumbnails().first());

      // Create a new stream
      FileStream *stream = new FileStream(this, socket->peerAddress(), request.path(), node.path, node.uid);
      if (stream->file.open())
      if (stream->setup(request, socket, &stream->file, node.mediaInfo.duration(), STime::fromSec(url.queryItemValue("position").toInt()), node.title(), thumb))
      if (stream->start())
        return true; // The graph owns the socket now.

      delete stream;
    }
  }

  socket->write(QHttpResponseHeader(404).toString().toUtf8());

  qWarning() << "Failed to start stream" << request.path();
  return false;
}

bool MediaServer::buildPlaylist(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');

  HtmlParser htmlParser;
  htmlParser.setField("ITEMS", QByteArray(""));

  const QString server = "http://" + request.value("Host") + httpPath();

  if (file.count() >= 2)
  {
    MediaDatabase::Node node = mediaDatabase->readNode(MediaDatabase::fromUidString(file.first()));

    htmlParser.setField("ITEM_LENGTH", QTime().addSecs(node.mediaInfo.duration().toSec()).toString(videoTimeFormat));

    if (!node.mediaInfo.title().isEmpty())
    {
      if (!node.mediaInfo.author().isEmpty())
        htmlParser.setField("ITEM_NAME", node.mediaInfo.title() + " [" + node.mediaInfo.author() + "]");
      else
        htmlParser.setField("ITEM_NAME", node.mediaInfo.title());
    }
    else
      htmlParser.setField("ITEM_NAME", node.fileName());

    htmlParser.setField("ITEM_URL", server.toAscii() + MediaDatabase::toUidString(node.uid) + ".mpeg");
    htmlParser.appendField("ITEMS", htmlParser.parse(m3uPlaylistItem));
  }

  QHttpResponseHeader response(200);
  response.setContentType("audio/x-mpegurl");
  response.setValue("Cache-Control", "no-cache");
  socket->write(response.toString().toUtf8());
  socket->write(htmlParser.parse(m3uPlaylist));

  return false;
}

bool MediaServer::handleHtmlRequest(const QUrl &url, const QString &file, QAbstractSocket *socket)
{
  QHttpResponseHeader response(200);
  response.setContentType("text/html;charset=utf-8");
  response.setValue("Cache-Control", "no-cache");

  HtmlParser htmlParser;

  SDebug::MutexLocker l(&dlnaDir.server()->mutex, __FILE__, __LINE__);

  if (file.isEmpty() || file.endsWith("-dir.html"))
  {
    const DlnaServerDir *dir = &dlnaDir;
    if (!file.isEmpty())
      dir = dir->server()->getDir(file.left(file.length() - 9).toInt(NULL, 16));

    if (dir)
    {
      ThumbnailListItemMap items;

      const DlnaServerDir::ConstDirMap *dirMap = &(dir->listDirs());
      const DlnaServerAlphaDir * const alphaDir = qobject_cast<const DlnaServerAlphaDir *>(dir);
      if (alphaDir)
        dirMap = &(alphaDir->listAllDirs());

      for (DlnaServerDir::ConstDirMap::ConstIterator i=dirMap->begin(); i!=dirMap->end(); i++)
      {
        const MediaServerFileDir * const mediaFileDir = qobject_cast<const MediaServerFileDir *>(*i);
        if (mediaFileDir)
        {
          int count = 0;
          foreach (MediaDatabase::UniqueID uid, mediaFileDir->uids)
          {
            ThumbnailListItem item;
            item.title = i.key();
            item.iconurl = MediaDatabase::toUidString(uid) + "-thumb.jpeg";
            item.url = MediaDatabase::toUidString(uid) + ".html";

            if (mediaFileDir->uids.count() > 1)
              item.title += ", " + tr("Part") + " " + QString::number(++count);

            items.insert(("00000000" + QString::number(dir->sortOrder, 16)).right(8) + item.title + MediaDatabase::toUidString(uid), item);
          }
        }
        else
        {
          const DlnaServerDir * const subDir = qobject_cast<const DlnaServerDir *>(*i);
          if (subDir)
          {
            ThumbnailListItem item;
            item.title = i.key();
            item.subtitle = QString::number(subDir->count()) + " " + tr("items");
            item.iconurl = subDir->findIcon();
            item.url = QString::number(subDir->id, 16) + "-dir.html";

            items.insert(("00000000" + QString::number(subDir->sortOrder, 16)).right(8) + item.title, item);
          }
        }
      }

      const DlnaServerDir::FileMap &fileMap = dir->listFiles();
      for (DlnaServerDir::FileMap::ConstIterator i=fileMap.begin(); i!=fileMap.end(); i++)
      {
        ThumbnailListItem item;
        item.title = i.key();
        item.iconurl = i->iconUrl;
        item.url = i->url + ".html";

        items.insert(("00000000" + QString::number(i->sortOrder, 16)).right(8) + item.title, item);
      }

      return sendHtmlContent(socket, url, response, buildThumbnailView("", items, url), headList);
    }
  }
  else if (file.endsWith(".html")) // Show player
  {
    const MediaDatabase::Node node = mediaDatabase->readNode(MediaDatabase::fromUidString(file.left(16)));
    if (!node.isNull())
    {
      htmlParser.setField("PLAYER", buildVideoPlayer(node, url));

      htmlParser.setField("PLAYER_INFOITEMS", QByteArray(""));
      htmlParser.setField("ITEM_NAME", tr("Title"));
      htmlParser.setField("ITEM_VALUE", node.mediaInfo.title());
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Duration"));
      htmlParser.setField("ITEM_VALUE", QTime().addSecs(node.mediaInfo.duration().toSec()).toString(videoTimeFormat));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Format"));
      htmlParser.setField("ITEM_VALUE", videoFormatString(node.mediaInfo));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Filename"));
      htmlParser.setField("ITEM_VALUE", node.fileName());
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));

      htmlParser.setField("PLAYER_DESCRIPTION_NAME", tr("Description"));
      htmlParser.setField("PLAYER_DESCRIPTION", node.mediaInfo.comment());

      return sendHtmlContent(socket, url, response, htmlParser.parse(htmlPlayer), headPlayer);
    }
  }

  response.setStatusLine(404);
  socket->write(response.toString().toUtf8());
  return false;
}

QString MediaServer::videoFormatString(const SMediaInfo &mediaInfo)
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

QByteArray MediaServer::buildVideoPlayer(const MediaDatabase::Node &node, const QUrl &url, const QSize &size)
{
  return VideoServer::buildVideoPlayer(MediaDatabase::toUidString(node.uid), node.mediaInfo, url, size);
}


MediaServer::FileStream::FileStream(MediaServer *parent, const QHostAddress &peer, const QString &url, const QString &fileName, MediaDatabase::UniqueID uid)
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

} // End of namespace
