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

MediaPlayerServer::SearchResultList MediaPlayerServer::search(const QStringList &rawQuery) const
{
  SearchResultList list;

//  foreach (const MediaDatabase::File &file, mediaDatabase->queryAlbums(category, rawQuery))
//  {
//    const SMediaInfo info = mediaDatabase->readNode(file.uid);
//    if (!info.isNull())
//    {
//      SearchResult result;
//      result.relevance = SStringParser::computeMatch(SStringParser::toRawName(info.title()), rawQuery);
//      result.headline = info.title();
//      result.location = MediaDatabase::toUidString(file.uid) + ".html";
//      result.thumbLocation = MediaDatabase::toUidString(file.uid) + "-thumb.png";
//
//      list += result;
//    }
//  }

  return list;
}

MediaPlayerServer::Stream * MediaPlayerServer::streamVideo(const SHttpServer::RequestMessage &request)
{
  const MediaServer::File file(request);

  SSandboxClient::Priority priority = SSandboxClient::Priority_Normal;
  if (file.url().queryItemValue("priority") == "low")
    priority = SSandboxClient::Priority_Low;
  else if (file.url().queryItemValue("priority") == "high")
    priority = SSandboxClient::Priority_High;

  SSandboxClient * const sandbox = masterServer->createSandbox(priority);
  connect(sandbox, SIGNAL(consoleLine(QString)), SLOT(consoleLine(QString)));
  sandbox->ensureStarted();

  const MediaDatabase::UniqueID uid = file.url().hasQueryItem("item")
    ? MediaDatabase::fromUidString(file.url().queryItemValue("item"))
    : MediaDatabase::fromUidString(file.fileName().split('.').first());

  if (uid != 0)
  {
    const FileNode node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      QUrl rurl;
      rurl.setPath(MediaPlayerSandbox::path + file.fileName());
      rurl.addQueryItem("playfile", QString::null);
      typedef QPair<QString, QString> QStringPair;
      foreach (const QStringPair &queryItem, file.url().queryItems())
        rurl.addQueryItem(queryItem.first, queryItem.second);

      if (request.hasField("timeSeekRange.dlna.org"))
      {
        int pos = rurl.queryItemValue("position").toInt();
        rurl.removeAllQueryItems("position");

        QString ntp = request.field("timeSeekRange.dlna.org");
        ntp = ntp.mid(ntp.indexOf("ntp=") + 4);
        ntp = ntp.left(ntp.indexOf('-'));
        pos += int(ntp.toFloat() + 0.5f);

        rurl.addQueryItem("position", QString::number(pos));
      }

      Stream *stream = new Stream(this, sandbox, request.path());
      if (stream->setup(rurl, node.toByteArray(-1)))
        return stream; // The graph owns the socket now.

      delete stream;
    }
  }

  disconnect(sandbox, SIGNAL(consoleLine(QString)), this, SLOT(consoleLine(QString)));
  masterServer->recycleSandbox(sandbox);

  return NULL;
}

int MediaPlayerServer::countItems(const QString &path)
{
  const bool hasAlbum = mediaDatabase->hasAlbum(category, path);
  int result = countAlbums(path) + mediaDatabase->countAlbumFiles(category, path);

  if ((result == 0) && !hasAlbum)
  {
    const QString dir = path.mid(path.left(path.length() - 1).lastIndexOf('/'));
    const QString basePath = path.left(path.left(path.length() - 1).lastIndexOf('/') + 1);
    foreach (MediaDatabase::UniqueID uid, mediaDatabase->getAlbumFiles(category, basePath))
    if (makeItem(uid).title == dir.mid(1, dir.length() - 2))
    {
      const FileNode node = mediaDatabase->readNode(uid);
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
    foreach (MediaDatabase::UniqueID uid, mediaDatabase->getAlbumFiles(category, basePath))
    if (makeItem(uid).title == dir.mid(1, dir.length() - 2))
    {
      const FileNode node = mediaDatabase->readNode(uid);
      if (!node.isNull())
      {
        if (returnAll || (count > 0))
        for (int i=start, n=0; (i<node.programs().count()) && (returnAll || (n<int(count))); i++, n++)
          result.append(makeItem(uid, node.programs()[i].programId));
      }

      break;
    }

    return result;
  }

  if (returnAll || (count > 0))
  foreach (MediaDatabase::UniqueID uid, mediaDatabase->getAlbumFiles(category, path, start, count))
    result.append(makeItem(uid));

  return result;
}

bool MediaPlayerServer::isEmpty(const QString &path)
{
  if (mediaDatabase->countAlbumFiles(category, path) == 0)
  if (countAlbums(path) == 0)
    return true;

  return false;
}

int MediaPlayerServer::countAlbums(const QString &path)
{
  return mediaDatabase->countAlbums(category, path);
}

QList<MediaPlayerServer::Item> MediaPlayerServer::listAlbums(const QString &path, unsigned &start, unsigned &count)
{
  const Item::Type itemType = defaultItemType();
  QList<Item> result;

  foreach (const QString &album, mediaDatabase->getAlbums(category, path, start, count))
  {
    Item item;
    item.isDir = true;
    item.type = itemType;
    item.title = album;
    item.iconUrl = findAlbumIcon(path + '/' + item.title);

    result += item;
    if (count > 0)
      count--;
  }

  start = unsigned(qMax(0, int(start) - mediaDatabase->countAlbums(category, path)));

  return result;
}

MediaPlayerServer::Item MediaPlayerServer::makeItem(MediaDatabase::UniqueID uid, int programId)
{
  Item item;

  const FileNode node = mediaDatabase->readNode(uid);
  if (!node.isNull())
  {
    if ((programId >= 0) || (node.programs().count() == 1))
    {
      foreach (const SMediaInfo::Program &program, node.programs())
      if ((program.programId == programId) || (node.programs().count() == 1))
      {
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
          item.seekable = true;
          item.type = defaultItemType(Item::Type(item.type));
          item.url = MediaDatabase::toUidString(uid) + "?pid=" + QByteArray::number(program.programId);
          item.iconUrl = MediaDatabase::toUidString(uid) + "-thumb.png?pid=" + QByteArray::number(program.programId);
  
          item.title = !program.title.isEmpty() ? program.title : (tr("Title") + " " + QString::number(program.programId + 1));
          item.artist = node.author();
          item.album = node.album();
          item.track = node.track();
  
          item.duration = program.duration.toSec();
  
          for (int a=0, an=program.audioStreams.count(); a < an; a++)
          for (int d=0, dn=program.dataStreams.count(); d <= dn; d++)
          {
            Item::Stream stream;
  
            stream.title = QString::number(a + 1) + ". " + program.audioStreams[a].fullName();
            stream.queryItems += qMakePair(QString("language"), program.audioStreams[a].toString());
  
            if (d < dn)
            {
              stream.title += ", " + QString::number(d + 1) + ". " + program.dataStreams[d].fullName() + " " + tr("subtitles");
              stream.queryItems += qMakePair(QString("subtitles"), program.dataStreams[d].toString());
            }
            else
              stream.queryItems += qMakePair(QString("subtitles"), QString());
  
            item.streams += stream;
          }
  
          foreach (const SMediaInfo::Chapter &chapter, program.chapters)
            item.chapters += Item::Chapter(chapter.title, chapter.begin.toSec());
  
          if (!program.audioStreams.isEmpty())
          {
            const SAudioCodec &codec = program.audioStreams.first().codec;
            item.audioFormat.setChannelSetup(codec.channelSetup());
            item.audioFormat.setSampleRate(codec.sampleRate());
          }
  
          if (!program.videoStreams.isEmpty())
          {
            const SVideoCodec &codec = program.videoStreams.first().codec;
            item.videoFormat.setSize(codec.size());
            item.videoFormat.setFrameRate(codec.frameRate());
          }
  
          if (!program.imageCodec.isNull())
            item.imageSize = program.imageCodec.size();
        }
        
        break;
      }
    }
    else if (!node.programs().isEmpty())
    {
      item.isDir = true;
      item.type = defaultItemType();
      item.title = node.title();
      item.iconUrl = MediaDatabase::toUidString(uid) + "-thumb.png?overlay=folder-video";
    }

    if (programId == -1)
    {
//      const ImdbClient::Entry imdbEntry = mediaDatabase->getImdbEntry(uid);
//      if (!imdbEntry.isNull())
//      {
//        item.title = imdbEntry.title;
//        if (imdbEntry.year > 0)
//          item.title += " " + QString::number(imdbEntry.year);
//
//        if (imdbEntry.rating > 0)
//          item.title += " [" + QString::number(imdbEntry.rating, 'f', 1) + "]";
//      }
//      else
        item.title = node.title();
    }
  }

  return item;
}

QUrl MediaPlayerServer::findAlbumIcon(const QString &path)
{
  // First check files.
  foreach (MediaDatabase::UniqueID uid, mediaDatabase->getAlbumFiles(category, path, 0, QThread::idealThreadCount()))
    return QUrl(serverPath() + MediaDatabase::toUidString(uid) + "-thumb.png?overlay=folder-video");

  // Recursively check albums
  foreach (const QString &album, mediaDatabase->getAlbums(category, path))
  {
    QUrl result = findAlbumIcon(path + '/' + album);
    if (!result.isEmpty())
      return result;
  }

  return QUrl();
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

SHttpServer::ResponseMessage MediaPlayerServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isGet())
  {
    const MediaServer::File file(request);

    int pid = -1;
    if (file.url().hasQueryItem("pid"))
      pid = file.url().queryItemValue("pid").toUShort();

    if (file.fileName().endsWith("-thumb.png"))
    {
      QSize size(128, 128);
      if (file.url().hasQueryItem("resolution"))
      {
        const QStringList sizeTxt = file.url().queryItemValue("resolution").split('x');
        if (sizeTxt.count() >= 2)
          size = QSize(sizeTxt[0].toInt(), sizeTxt[1].toInt());
        else if (sizeTxt.count() >= 1)
          size = QSize(sizeTxt[0].toInt(), sizeTxt[0].toInt());
      }

      const MediaDatabase::UniqueID uid = MediaDatabase::fromUidString(file.fileName());
      const FileNode node = mediaDatabase->readNode(uid);
      if (!node.isNull())
      foreach (const SMediaInfo::Program &program, node.programs())
      if ((pid == -1) || (program.programId == pid))
      {
        if (!program.thumbnail.isEmpty())
        {
          QImage image = QImage::fromData(program.thumbnail);
          if (!image.isNull())
            image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

          QImage result(size, QImage::Format_ARGB32);
          QPainter p;
          p.begin(&result);
          p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha
          p.fillRect(result.rect(), Qt::transparent);
          p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha
          p.drawImage(
              (result.width() / 2) - (image.width() / 2),
              (result.height() / 2) - (image.height() / 2),
              image);

          if (file.url().hasQueryItem("overlay"))
          {
            QImage overlayImage(":/lximediacenter/images/" + file.url().queryItemValue("overlay") + ".png");
            if (!overlayImage.isNull())
            {
              overlayImage = overlayImage.scaled(size / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);

              p.drawImage(
                  (result.width() / 2) - (overlayImage.width() / 2),
                  (result.height() / 2) - (overlayImage.height() / 2),
                  overlayImage);
            }
          }

          p.end();

          QBuffer b;
          result.save(&b, "PNG");

          return makeResponse(request, b.data(), SHttpEngine::mimeImagePng, true);
        }

        break;
      }

      QImage image(":/lximediacenter/images/video-template.png");
      if (!image.isNull())
      {
        if (file.url().hasQueryItem("resolution"))
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

        return makeResponse(request, b.data(), SHttpEngine::mimeImagePng, true);
      }

      SHttpServer::ResponseMessage response(request, SHttpServer::Status_MovedPermanently);
      response.setField("Location", "http://" + request.host() + "/img/null.png");
      return response;
    }
    else if (file.fileName().endsWith(".html", Qt::CaseInsensitive)) // Show player
    {
      const MediaDatabase::UniqueID uid = MediaDatabase::fromUidString(file.fileName());
      const FileNode node = mediaDatabase->readNode(uid);
      if (!node.isNull())
      foreach (const SMediaInfo::Program &program, node.programs())
      if ((pid == -1) || (program.programId == pid))
        return makeHtmlContent(request, file.url(), buildVideoPlayer(uid, node.title(), program, file.url()), headPlayer);
    }
  }

  return MediaServer::httpRequest(request, socket);
}

QByteArray MediaPlayerServer::buildVideoPlayer(MediaDatabase::UniqueID uid, const QString &title, const SMediaInfo::Program &program, const QUrl &url, const QSize &size)
{
  return MediaServer::buildVideoPlayer(MediaDatabase::toUidString(uid), title, program, url, size);
}

QByteArray MediaPlayerServer::buildVideoPlayer(const QByteArray &item, const QString &title, const QUrl &url, const QSize &size)
{
  return MediaServer::buildVideoPlayer(item, title, url, size);
}

void MediaPlayerServer::consoleLine(const QString &line)
{
  if (line.startsWith("#PLAYED:"))
    mediaDatabase->setLastPlayed(QString::fromUtf8(QByteArray::fromHex(line.mid(8).toAscii())));
}


MediaPlayerServer::Stream::Stream(MediaPlayerServer *parent, SSandboxClient *sandbox, const QString &url)
  : MediaServer::Stream(parent, url),
    sandbox(sandbox)
{
}

MediaPlayerServer::Stream::~Stream()
{
  disconnect(sandbox, SIGNAL(consoleLine(QString)), parent, SLOT(consoleLine(QString)));
  static_cast<MediaPlayerServer *>(parent)->masterServer->recycleSandbox(sandbox);
}

bool MediaPlayerServer::Stream::setup(const QUrl &url, const QByteArray &content)
{
  SHttpEngine::RequestMessage message(sandbox);
  message.setRequest("POST", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));
  message.setContent(content);

  sandbox->openRequest(message, &proxy, SLOT(setSource(QIODevice *)));

  return true;
}

} } // End of namespaces
