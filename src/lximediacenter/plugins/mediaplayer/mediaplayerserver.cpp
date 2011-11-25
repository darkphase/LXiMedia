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

const char MediaPlayerServer::dirSplit =
#if defined(Q_OS_UNIX)
    ':';
#elif  defined(Q_OS_WIN)
    ';';
#else
#error Not implemented.
#endif

const Qt::CaseSensitivity MediaPlayerServer::caseSensitivity =
#if defined(Q_OS_UNIX)
    Qt::CaseSensitive;
#elif  defined(Q_OS_WIN)
    Qt::CaseInsensitive;
#else
#error Not implemented.
#endif

MediaPlayerServer::MediaPlayerServer(const QString &, QObject *parent)
  : MediaServer(parent),
    masterServer(NULL),
    mediaDatabase(NULL)
{
  PluginSettings settings(Module::pluginName);

  QStringList paths;
  foreach (const QString &root, settings.value("RootPaths").toStringList())
  if (!root.trimmed().isEmpty() && !isHidden(root) && QDir(root).exists())
    paths.append(root);

  setRootPaths(paths);
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

QString MediaPlayerServer::serverName(void) const
{
  return Module::pluginName;
}

QString MediaPlayerServer::serverIconPath(void) const
{
  return "/img/media-tape.png";
}

MediaPlayerServer::SearchResultList MediaPlayerServer::search(const QStringList &rawQuery) const
{
  SearchResultList list;

//  foreach (const MediaDatabase::File &file, mediaDatabase->queryAlbums(rawQuery))
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

  const QString filePath = realPath(file.file());
  if (!filePath.isEmpty())
  {
    QUrl rurl;
    rurl.setPath(MediaPlayerSandbox::path);
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
    if (stream->setup(rurl, filePath.toUtf8()))
      return stream; // The graph owns the socket now.

    delete stream;
  }

  disconnect(sandbox, SIGNAL(consoleLine(QString)), this, SLOT(consoleLine(QString)));
  masterServer->recycleSandbox(sandbox);

  return NULL;
}

int MediaPlayerServer::countItems(const QString &virtualPath)
{
  if (virtualPath != serverPath())
  {
    const QString path = realPath(virtualPath);

    return countAlbums(path) + mediaDatabase->countAlbumFiles(path);
  }
  else
    return rootPaths.count();
}

QList<MediaPlayerServer::Item> MediaPlayerServer::listItems(const QString &virtualPath, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;

  if (virtualPath != serverPath())
  {
    const QString path = realPath(virtualPath);
    QList<Item> result = listAlbums(path, start, count);

    if (returnAll || (count > 0))
    foreach (const FileNode &node, mediaDatabase->getAlbumFiles(path, start, count))
      result.append(makeItem(node));

    return result;
  }
  else
  {
    QList<Item> result;

    const QStringList paths = rootPaths.keys();
    for (unsigned i=start, n=0; (int(i)<paths.count()) && (returnAll || (n<count)); i++, n++)
    {
      Item item;
      item.isDir = true;
      item.title = paths[i];
      item.iconUrl = "/img/folder-video.png";

      result += item;
    }

    return result;
  }
}

bool MediaPlayerServer::isHidden(const QString &path)
{
  static QSet<QString> hiddenDirs;
  if (hiddenDirs.isEmpty())
  {
    const QDir root = QDir::root();

#if defined(Q_OS_UNIX)
    hiddenDirs += root.absoluteFilePath("bin");
    hiddenDirs += root.absoluteFilePath("boot");
    hiddenDirs += root.absoluteFilePath("dev");
    hiddenDirs += root.absoluteFilePath("etc");
    hiddenDirs += root.absoluteFilePath("lib");
    hiddenDirs += root.absoluteFilePath("proc");
    hiddenDirs += root.absoluteFilePath("sbin");
    hiddenDirs += root.absoluteFilePath("sys");
    hiddenDirs += root.absoluteFilePath("tmp");
    hiddenDirs += root.absoluteFilePath("usr");
    hiddenDirs += root.absoluteFilePath("var");
#endif

#if defined(Q_OS_MACX)
    hiddenDirs += root.absoluteFilePath("Applications");
    hiddenDirs += root.absoluteFilePath("cores");
    hiddenDirs += root.absoluteFilePath("Developer");
    hiddenDirs += root.absoluteFilePath("private");
    hiddenDirs += root.absoluteFilePath("System");
#endif

#if defined(Q_OS_WIN)
    hiddenDirs += root.absoluteFilePath("Program Files");
    hiddenDirs += root.absoluteFilePath("Program Files (x86)");
    hiddenDirs += root.absoluteFilePath("WINDOWS");
#endif

    foreach (const QFileInfo &drive, QDir::drives())
    {
      hiddenDirs += QDir(drive.absoluteFilePath()).absoluteFilePath("lost+found");

#if defined(Q_OS_WIN)
      hiddenDirs += QDir(drive.absoluteFilePath()).absoluteFilePath("RECYCLER");
      hiddenDirs += QDir(drive.absoluteFilePath()).absoluteFilePath("System Volume Information");
#endif
    }
  }

  const QFileInfo info(path);

  QString absoluteFilePath = info.absolutePath();
  if (!absoluteFilePath.endsWith('/')) absoluteFilePath += '/';

  QString canonicalFilePath = (info.exists() ? info.canonicalFilePath() : info.absolutePath());
  if (!canonicalFilePath.endsWith('/')) canonicalFilePath += '/';

  foreach (const QString &hidden, hiddenDirs)
  {
    const QString path = hidden.endsWith('/') ? hidden : (hidden + '/');
    if (absoluteFilePath.startsWith(path, caseSensitivity) || canonicalFilePath.startsWith(path, caseSensitivity))
      return true;
  }

  return false;
}

void MediaPlayerServer::setRootPaths(const QStringList &paths)
{
  struct T
  {
    static QString albumName(const QString &path, int recurse)
    {
      QStringList dirs = path.split('/', QString::SkipEmptyParts);

      QString result;
      for (int r=0; (r<recurse) && !dirs.isEmpty(); r++)
        result = dirs.takeLast() + (result.isEmpty() ? "" : "_") + result;

      return result;
    }
  };

  PluginSettings settings(Module::pluginName);

  settings.setValue("RootPaths", paths);

  rootPaths.clear();
  foreach (QString path, paths)
  {
    if (!path.endsWith('/'))
      path += '/';

    const QString album = T::albumName(path, 1);
    rootPaths.insert(album, path);
  }
}

QString MediaPlayerServer::virtualPath(const QString &realPath) const
{
  for (QMap<QString, QString>::ConstIterator i=rootPaths.begin(); i!=rootPaths.end(); i++)
  if (realPath.startsWith(i.value(), caseSensitivity))
    return serverPath() + i.key() + '/' + realPath.mid(i.value().length());

  return QString::null;
}

QString MediaPlayerServer::realPath(const QString &virtualPath) const
{
  const QString basepath = serverPath();

  if (virtualPath.startsWith(basepath))
  {
    const int s = virtualPath.indexOf('/', basepath.length());
    if (s >= 1)
    {
      QMap<QString, QString>::ConstIterator i = rootPaths.find(virtualPath.mid(basepath.length(), s - basepath.length()));
      if (i != rootPaths.end())
        return i.value() + virtualPath.mid(s + 1);
    }
  }

  return QString::null;
}

bool MediaPlayerServer::isEmpty(const QString &path)
{
  if (mediaDatabase->countAlbumFiles(path) == 0)
  if (countAlbums(path) == 0)
    return true;

  return false;
}

int MediaPlayerServer::countAlbums(const QString &path)
{
  return mediaDatabase->countAlbums(path);
}

QList<MediaPlayerServer::Item> MediaPlayerServer::listAlbums(const QString &path, unsigned &start, unsigned &count)
{
  QList<Item> result;

  foreach (const QString &album, mediaDatabase->getAlbums(path, start, count))
  {
    Item item;
    item.isDir = true;
    item.title = album;
    item.iconUrl = "/img/folder-video.png";

    result += item;
    if (count > 0)
      count--;
  }

  start = unsigned(qMax(0, int(start) - mediaDatabase->countAlbums(path)));

  return result;
}

MediaPlayerServer::Item MediaPlayerServer::makeItem(const FileNode &node)
{
  Item item;

  if (!node.isNull())
  {
    if (!node.audioStreams().isEmpty() && !node.videoStreams().isEmpty())
      item.type = SUPnPContentDirectory::Item::Type_Video;
    else if (!node.audioStreams().isEmpty())
      item.type = SUPnPContentDirectory::Item::Type_Audio;
    else if (!node.imageCodec().isNull())
      item.type = SUPnPContentDirectory::Item::Type_Image;
    else
      item.type = SUPnPContentDirectory::Item::Type_None;
  
    if (item.type != SUPnPContentDirectory::Item::Type_None)
    {

      item.played = mediaDatabase->lastPlayed(node.filePath()).isValid();
      item.seekable = true;
      item.url = virtualPath(node.filePath());
      item.iconUrl = item.url;
      item.iconUrl.addQueryItem("thumbnail", QString::null);

      item.title = node.metadata("title").toString();
      item.artist = node.metadata("author").toString();
      item.album = node.metadata("album").toString();
      item.track = node.metadata("track").toInt();

      item.duration = node.duration().toSec();

      for (int a=0, an=node.audioStreams().count(); a < an; a++)
      for (int d=0, dn=node.dataStreams().count(); d <= dn; d++)
      {
        Item::Stream stream;

        stream.title = QString::number(a + 1) + ". " + node.audioStreams()[a].fullName();
        stream.queryItems += qMakePair(QString("language"), node.audioStreams()[a].toString());

        if (d < dn)
        {
          stream.title += ", " + QString::number(d + 1) + ". " + node.dataStreams()[d].fullName() + " " + tr("subtitles");
          stream.queryItems += qMakePair(QString("subtitles"), node.dataStreams()[d].toString());
        }
        else
          stream.queryItems += qMakePair(QString("subtitles"), QString());

        item.streams += stream;
      }

      foreach (const SMediaInfo::Chapter &chapter, node.chapters())
        item.chapters += Item::Chapter(chapter.title, chapter.begin.toSec());

      if (!node.audioStreams().isEmpty())
      {
        const SAudioCodec &codec = node.audioStreams().first().codec;
        item.audioFormat.setChannelSetup(codec.channelSetup());
        item.audioFormat.setSampleRate(codec.sampleRate());
      }

      if (!node.videoStreams().isEmpty())
      {
        const SVideoCodec &codec = node.videoStreams().first().codec;
        item.videoFormat.setSize(codec.size());
        item.videoFormat.setFrameRate(codec.frameRate());
      }

      if (!node.imageCodec().isNull())
        item.imageSize = node.imageCodec().size();
    }
    else
    {
      item.url = "";
      item.iconUrl = "/img/misc.png";
    }
  }

  return item;
}

SHttpServer::ResponseMessage MediaPlayerServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isGet())
  {
    const MediaServer::File file(request);

    if (file.parentDir() == ".Settings")
    {
      return handleHtmlRequest(request, file);
    }
    else if (file.url().hasQueryItem("thumbnail"))
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

      const FileNode node = mediaDatabase->readNode(realPath(file.file()));
      if (!node.isNull())
      {
        if (!node.thumbnail().isNull())
        {
          SImage image = node.thumbnail();
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
    else if (file.url().hasQueryItem("player"))
    {
//      const FileNode node = mediaDatabase->readNode(realPath(file.file()));
//      if (!node.isNull())
//      foreach (const SMediaInfo::Program &program, node.programs())
//      if ((pid == -1) || (program.programId == pid))
//        return makeHtmlContent(request, file.url(), buildVideoPlayer(node, file.url()), headPlayer);
    }
  }

  return MediaServer::httpRequest(request, socket);
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
