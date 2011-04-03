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
#include "mediaplayersandbox.h"
#include "module.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

MediaPlayerServer::MediaPlayerServer(MediaDatabase::Category category, QObject *parent)
  : MediaServer(parent),
    category(category),
    masterServer(NULL),
    mediaDatabase(NULL)
{
}

void MediaPlayerServer::initialize(MasterServer *masterServer)
{
  this->masterServer = masterServer;
  this->mediaDatabase = MediaDatabase::createInstance(masterServer);

  MediaServer::initialize(masterServer);
}

void MediaPlayerServer::close(void)
{
  MediaServer::close();
}

QString MediaPlayerServer::pluginName(void) const
{
  return Module::pluginName;
}

MediaPlayerServer::Stream * MediaPlayerServer::streamVideo(const SHttpServer::RequestHeader &request)
{
  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  MediaDatabase::UniqueID uid;
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

  if (uid.fid != 0)
  {
    const SMediaInfo node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    if (uid.pid < node.programs().count())
    {
      QUrl rurl;
      rurl.setPath(MediaPlayerSandbox::path + request.file());
      rurl.addQueryItem("playfile", QString::null);
      rurl.addQueryItem("pid", QString::number(uid.pid));
      typedef QPair<QString, QString> QStringPair;
      foreach (const QStringPair &queryItem, url.queryItems())
        rurl.addQueryItem(queryItem.first, queryItem.second);

      Stream *stream = new Stream(this, request.path());
      if (stream->setup(rurl, node.toByteArray(-1)))
        return stream; // The graph owns the socket now.

      delete stream;
    }
  }

  return NULL;
}

bool MediaPlayerServer::isEmpty(const QString &path)
{
  if (mediaDatabase->countAlbumFiles(category, path) == 0)
  if (countAlbums(path) == 0)
    return true;

  return false;
}

int MediaPlayerServer::countItems(const QString &path)
{
  const bool hasAlbum = mediaDatabase->hasAlbum(category, path);
  int result = countAlbums(path) + mediaDatabase->countAlbumFiles(category, path);

  if ((result == 0) && !hasAlbum)
  {
    const QString dir = path.mid(path.left(path.length() - 1).lastIndexOf('/'));
    const QString basePath = path.left(path.left(path.length() - 1).lastIndexOf('/') + 1);
    foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, basePath))
    if (makeItem(file.uid).title == dir.mid(1, dir.length() - 2))
    {
      const SMediaInfo node = mediaDatabase->readNode(file.uid);
      if (!node.isNull())
        return node.programs().count();

      break;
    }
  }

  return result;
}

QList<MediaPlayerServer::Item> MediaPlayerServer::listItems(const QString &path, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;
  const bool hasAlbum = mediaDatabase->hasAlbum(category, path);
  QList<Item> result = listAlbums(path, start, count);

  if (result.isEmpty() && !hasAlbum)
  {
    const QString dir = path.mid(path.left(path.length() - 1).lastIndexOf('/'));
    const QString basePath = path.left(path.left(path.length() - 1).lastIndexOf('/') + 1);
    foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, basePath))
    if (makeItem(file.uid).title == dir.mid(1, dir.length() - 2))
    {
      const SMediaInfo node = mediaDatabase->readNode(file.uid);
      if (!node.isNull())
      {
        if (returnAll || (count > 0))
        for (int i=start, n=0; (i<node.programs().count()) && (returnAll || (n<int(count))); i++, n++)
          result.append(makeItem(MediaDatabase::UniqueID(file.uid.fid, i), false));
      }

      break;
    }

    return result;
  }

  if (returnAll || (count > 0))
  foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, path, start, count))
    result.append(makeItem(file.uid));

  return result;
}

int MediaPlayerServer::countAlbums(const QString &path)
{
  QSet<QString> names;
  foreach (const QString &album, mediaDatabase->allAlbums(category))
  if (album.startsWith(path))
  {
    const QString sub = album.mid(path.length());
    const int slash = sub.indexOf('/');
    if (slash > 0)
      names.insert(sub.left(slash));
  }

  return names.count();
}

QList<MediaPlayerServer::Item> MediaPlayerServer::listAlbums(const QString &path,  unsigned &start, unsigned &count)
{
  const bool returnAll = count == 0;
  const Item::Type itemType = defaultItemType();
  QList<Item> result;
  QSet<QString> names;

  foreach (const QString &album, mediaDatabase->allAlbums(category))
  if (album.startsWith(path))
  {
    const QString sub = album.mid(path.length());
    const int slash = sub.indexOf('/');
    if (slash > 0)
    {
      const QString name = sub.left(slash);
      if (!names.contains(name))
      {
        names.insert(name);

        if (returnAll || (count > 0))
        {
          if (start == 0)
          {
            Item item;
            item.isDir = true;
            item.type = itemType;
            item.title = name;

            foreach (const MediaDatabase::File &file, mediaDatabase->getAlbumFiles(category, path + item.title + '/', 0, 8))
            {
              item.iconUrl = serverPath() + MediaDatabase::toUidString(file.uid) + "-thumb.png?overlay=folder-video";
              break;
            }

            result += item;
            if (count > 0)
              count--;
          }
          else
            start--;
        }
      }
    }
  }

  return result;
}

MediaPlayerServer::Item MediaPlayerServer::makeItem(MediaDatabase::UniqueID uid, bool recursePrograms)
{
  Item item;

  const SMediaInfo node = mediaDatabase->readNode(uid);
  if (!node.isNull())
  {
    if ((uid.pid < node.programs().count()) && (!recursePrograms || (node.programs().count() == 1)))
    {
      const SMediaInfo::Program program = node.programs().at(uid.pid);

      if (!program.audioStreams.isEmpty() && !program.videoStreams.isEmpty())
        item.type = SUPnPContentDirectory::Item::Type_Video;
      else if (!program.audioStreams.isEmpty())
        item.type = SUPnPContentDirectory::Item::Type_Audio;
      else if (!program.imageCodec.isNull())
        item.type = SUPnPContentDirectory::Item::Type_Image;
      else
        item.type = SUPnPContentDirectory::Item::Type_None;

      if (item.type != SUPnPContentDirectory::Item::Type_None)
      {
        item.played = mediaDatabase->lastPlayed(uid).isValid();
        item.type = defaultItemType(Item::Type(item.type));
        item.url = MediaDatabase::toUidString(uid);

        item.title = !program.title.isEmpty() ? program.title : (tr("Title") + " " + QString::number(uid.pid + 1));
        item.artist = node.author();
        item.album = node.album();
        item.track = node.track();

        item.duration = program.duration.toSec();

        foreach (const SMediaInfo::Chapter &chapter, program.chapters)
          item.chapters += Item::Chapter(chapter.title, chapter.begin.toSec());

        foreach (const SMediaInfo::AudioStreamInfo &stream, program.audioStreams)
          item.audioStreams += Item::Stream(stream, SStringParser::iso639Language(stream.language));

        foreach (const SMediaInfo::VideoStreamInfo &stream, program.videoStreams)
          item.videoStreams += Item::Stream(stream, SStringParser::iso639Language(stream.language));

        foreach (const SMediaInfo::DataStreamInfo &stream, program.dataStreams)
          item.subtitleStreams += Item::Stream(stream, SStringParser::iso639Language(stream.language));

        if (!program.thumbnail.isEmpty())
          item.iconUrl = MediaDatabase::toUidString(uid) + "-thumb.jpeg";
      }
    }
    else if (!node.programs().isEmpty())
    {
      item.isDir = true;
      item.type = defaultItemType();
      item.title = node.title();
      item.iconUrl = MediaDatabase::toUidString(uid) + "-thumb.png?overlay=folder-video";
    }

    if (recursePrograms)
    {
      const ImdbClient::Entry imdbEntry = mediaDatabase->getImdbEntry(uid);
      if (!imdbEntry.isNull())
      {
        item.title = imdbEntry.title;
        if (imdbEntry.year > 0)
          item.title += " " + QString::number(imdbEntry.year);

        if (imdbEntry.rating > 0)
          item.title += " [" + QString::number(imdbEntry.rating, 'f', 1) + "]";
      }
      else
        item.title = node.title();
    }
  }

  return item;
}

MediaPlayerServer::Item::Type MediaPlayerServer::defaultItemType(Item::Type type) const
{
  switch (category)
  {
  default:
  case MediaDatabase::Category_None:
    return Item::Type_None;

  case MediaDatabase::Category_Movies:
    return ((type == Item::Type_None) || (type == Item::Type_Video)) ? Item::Type_Movie : type;

  case MediaDatabase::Category_TVShows:
    return ((type == Item::Type_None) || (type == Item::Type_Video)) ? Item::Type_VideoBroadcast : type;

  case MediaDatabase::Category_Clips:
    return (type == Item::Type_None) ? Item::Type_Video : type;

  case MediaDatabase::Category_HomeVideos:
    return (type == Item::Type_None) ? Item::Type_Video : type;

  case MediaDatabase::Category_Photos:
    return ((type == Item::Type_None) || (type == Item::Type_Image)) ? Item::Type_Photo : type;

  case MediaDatabase::Category_Music:
    if ((type == Item::Type_None) || (type == Item::Type_Audio))
      return Item::Type_Music;
    else if (type == Item::Type_Video)
      return Item::Type_MusicVideo;
    else
      return type;
  }
}

SHttpServer::SocketOp MediaPlayerServer::handleHttpRequest(const SHttpServer::RequestHeader &request, QIODevice *socket)
{
  const QUrl url(request.path());
  const QString file = request.file();

  if (file.endsWith("-thumb.jpeg") || file.endsWith("-thumb.png"))
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

    const MediaDatabase::UniqueID uid = MediaDatabase::fromUidString(file);
    const SMediaInfo node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    if (uid.pid < node.programs().count())
    {
      const SMediaInfo::Program program = node.programs().at(uid.pid);
      if (!program.thumbnail.isEmpty())
      {
        if (!url.hasQueryItem("size") && !url.hasQueryItem("overlay") && !file.endsWith(".png"))
        {
          return sendResponse(request, socket, program.thumbnail, "image/jpeg", true);
        }
        else
        {
          QImage image = QImage::fromData(program.thumbnail);
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

          return sendResponse(request, socket, b.data(), "image/png", true);
        }
      }
    }

    QImage image(":/mediaplayer/video-template.png");
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

      return sendResponse(request, socket, b.data(), "image/png", true);
    }

    return SHttpServer::sendRedirect(request, socket, "http://" + request.host() + "/img/null.png");
  }
  else if (file.endsWith(".html")) // Show player
  {
    const MediaDatabase::UniqueID uid = MediaDatabase::fromUidString(file);
    const SMediaInfo node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    if (uid.pid < node.programs().count())
    {
      const SMediaInfo::Program program = node.programs().at(uid.pid);

      SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
      response.setContentType("text/html;charset=utf-8");
      response.setField("Cache-Control", "no-cache");

      HtmlParser htmlParser;

      htmlParser.setField("PLAYER", buildVideoPlayer(uid, program, url));

      htmlParser.setField("PLAYER_INFOITEMS", QByteArray(""));
      htmlParser.setField("ITEM_NAME", tr("Title"));
      htmlParser.setField("ITEM_VALUE", node.title());
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Duration"));
      htmlParser.setField("ITEM_VALUE", QTime().addSecs(program.duration.toSec()).toString(videoTimeFormat));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Format"));
      htmlParser.setField("ITEM_VALUE", videoFormatString(program));
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

QString MediaPlayerServer::videoFormatString(const SMediaInfo::Program &program)
{
  if (!program.videoStreams.isEmpty())
  {
    const SVideoCodec codec = program.videoStreams.first().codec;
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

QByteArray MediaPlayerServer::buildVideoPlayer(MediaDatabase::UniqueID uid, const SMediaInfo::Program &program, const QUrl &url, const QSize &size)
{
  return MediaServer::buildVideoPlayer(MediaDatabase::toUidString(uid), program, url, size);
}

QByteArray MediaPlayerServer::buildVideoPlayer(const QByteArray &item, const QString &title, const QUrl &url, const QSize &size)
{
  return MediaServer::buildVideoPlayer(item, title, url, size);
}

void MediaPlayerServer::consoleLine(const QString &line)
{
  if (!line.startsWith('%'))
    sApp->logLineToActiveLogFile(line);
  else
    mediaDatabase->setLastPlayed(QString::fromUtf8(QByteArray::fromHex(line.mid(1).toAscii())));
}


MediaPlayerServer::Stream::Stream(MediaPlayerServer *parent, const QString &url)
  : MediaServer::Stream(parent, url),
    sandbox(NULL)
{
}

MediaPlayerServer::Stream::~Stream()
{
  if (sandbox)
  {
    disconnect(sandbox, SIGNAL(consoleLine(QString)), parent, SLOT(consoleLine(QString)));
    static_cast<MediaPlayerServer *>(parent)->masterServer->recycleSandbox(sandbox);
  }
}

bool MediaPlayerServer::Stream::setup(const QUrl &url, const QByteArray &content)
{
  if (sandbox)
  {
    disconnect(sandbox, SIGNAL(consoleLine(QString)), parent, SLOT(consoleLine(QString)));
    static_cast<MediaPlayerServer *>(parent)->masterServer->recycleSandbox(sandbox);
  }

  sandbox = static_cast<MediaPlayerServer *>(parent)->masterServer->createSandbox(
      (url.queryItemValue("priority") == "low") ? SSandboxClient::Mode_Nice : SSandboxClient::Mode_Normal);

  connect(sandbox, SIGNAL(consoleLine(QString)), parent, SLOT(consoleLine(QString)));

  SHttpEngine::RequestMessage message(sandbox);
  message.setRequest("POST", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));
  message.setHost(sandbox->serverName());
  message.setContent(content);

  sandbox->openRequest(message, &proxy, SLOT(setSource(QIODevice *)));

  return true;
}

} } // End of namespaces
