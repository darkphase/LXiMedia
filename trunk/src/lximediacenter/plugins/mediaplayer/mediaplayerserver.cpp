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

const QEvent::Type  MediaPlayerServer::responseEventType = QEvent::Type(QEvent::registerEventType());
const int           MediaPlayerServer::maxSongDurationMin = 7;

MediaPlayerServer::MediaPlayerServer(const QString &, QObject *parent)
  : MediaServer(parent),
    masterServer(NULL),
    mediaDatabase(NULL)
{
  QSettings settings;
  settings.beginGroup(Module::pluginName);

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

  connect(mediaDatabase, SIGNAL(nodeRead(FileNode)), SLOT(nodeRead(FileNode)), Qt::QueuedConnection);
  connect(mediaDatabase, SIGNAL(aborted()), SLOT(aborted()), Qt::QueuedConnection);

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

MediaPlayerServer::Stream * MediaPlayerServer::streamVideo(const SHttpServer::RequestMessage &request)
{
  SSandboxClient::Priority priority = SSandboxClient::Priority_Normal;
  if (request.url().queryItemValue("priority") == "low")
    priority = SSandboxClient::Priority_Low;
  else if (request.url().queryItemValue("priority") == "high")
    priority = SSandboxClient::Priority_High;

  SSandboxClient * const sandbox = masterServer->createSandbox(priority);
  connect(sandbox, SIGNAL(consoleLine(QString)), SLOT(consoleLine(QString)));
  sandbox->ensureStarted();

  const QString filePath = realPath(request.file());
  if (!filePath.isEmpty())
  {
    QUrl rurl;
    rurl.setPath(MediaPlayerSandbox::path);
    rurl.addQueryItem("play", QString::null);
    typedef QPair<QString, QString> QStringPair;
    foreach (const QStringPair &queryItem, request.url().queryItems())
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

    const QString titleFile = virtualFile(request.file());
    if (titleFile.length() > 1)
      rurl.addQueryItem("title", titleFile.mid(1));

    Stream *stream = new Stream(this, sandbox, request.path());
    if (stream->setup(rurl, filePath.toUtf8()))
      return stream; // The graph owns the socket now.

    delete stream;
  }

  disconnect(sandbox, SIGNAL(consoleLine(QString)), this, SLOT(consoleLine(QString)));
  masterServer->recycleSandbox(sandbox);

  return NULL;
}

SHttpServer::ResponseMessage MediaPlayerServer::sendPhoto(const SHttpServer::RequestMessage &request)
{
  const QString filePath = realPath(request.file());
  if (!filePath.isEmpty())
  {
    SSize size(4096, 4096);
    if (request.url().hasQueryItem("resolution"))
      size = SSize::fromString(request.url().queryItemValue("resolution"));

    QColor backgroundColor;
    if (request.url().hasQueryItem("bgcolor"))
      backgroundColor.setNamedColor('#' + request.url().queryItemValue("bgcolor"));

    QString format = "png";
    if (request.url().hasQueryItem("format"))
      format = request.url().queryItemValue("format");

    QString contentType = "image/" + format.toLower();

    const QString contentFeatures = QByteArray::fromBase64(request.url().queryItemValue("contentFeatures").toAscii());
    const MediaProfiles::ImageProfile imageProfile = mediaProfiles().imageProfileFor(contentFeatures);
    if (imageProfile != 0) // DLNA stream.
    {
      MediaProfiles::correctFormat(imageProfile, size);
      format = mediaProfiles().formatFor(imageProfile);
      contentType = mediaProfiles().mimeTypeFor(imageProfile);
    }

    SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
    response.setContentType(contentType);
    if (!contentFeatures.isEmpty())
    {
      response.setField("transferMode.dlna.org", "Interactive");
      response.setField("contentFeatures.dlna.org", contentFeatures);
    }

    if (!request.isHead())
    {
      const QByteArray result = mediaDatabase->readImage(filePath, size.size(), backgroundColor, format);
      if (!result.isEmpty())
        response.setContent(result);
      else
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
    }

    return response;
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

int MediaPlayerServer::countItems(const QString &virtualPath)
{
  if (virtualPath != serverPath())
  {
    const QString path = realPath(virtualPath);

    const FileNode node = mediaDatabase->readNode(path);
    if (!node.titles().isEmpty())
    {
      return node.titles().count();
    }
    else
    {
      int result = mediaDatabase->countItems(realPath(virtualPath));
      if (result > 1)
        result++; // "Play all" item

      return result;
    }
  }
  else
    return rootPaths.count();
}

QList<MediaPlayerServer::Item> MediaPlayerServer::listItems(const QString &virtualPath, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;
  QList<Item> result;

  if (virtualPath != serverPath())
  {
    if (returnAll || (count > 0))
    {
      const QString path = realPath(virtualPath);

      const FileNode node = mediaDatabase->readNode(path);
      if (!node.isNull())
      {
        if (!node.titles().isEmpty())
        {
          const int numTitles = node.titles().count();
          for (unsigned i=start, n=0; (int(i)<numTitles) && (returnAll || (n<count)); i++, n++)
            result += makeItem(node, i);
        }
        else
        {
          if (mediaDatabase->countItems(path) > 1)
          {
            if (start == 0)
            {
              result += makePlayAllItem(virtualPath);
              count--;
            }
            else
              start--;
          }

          if (returnAll || (count > 0))
          {
            foreach (const FileNode &node, mediaDatabase->listItems(path, start, count))
              result += makeItem(node);
          }
        }
      }
    }
  }
  else
  {
    const QStringList paths = rootPaths.keys();
    for (unsigned i=start, n=0; (int(i)<paths.count()) && (returnAll || (n<count)); i++, n++)
    {
      Item item;
      item.isDir = true;
      item.title = paths[i];
      item.path = virtualPath + paths[i] + '/';
      item.iconUrl = "/img/directory.png";

      result += item;
    }
  }

  return result;
}

MediaPlayerServer::Item MediaPlayerServer::getItem(const QString &virtualPath)
{
  if (virtualPath.endsWith("/.all"))
    return makePlayAllItem(virtualPath.left(virtualPath.length() - 4));
  else
    return makeItem(mediaDatabase->readNode(realPath(virtualPath)));
}

MediaPlayerServer::ListType MediaPlayerServer::listType(const QString &virtualPath)
{
  switch (dirType(virtualPath))
  {
  case SMediaInfo::ProbeInfo::FileType_Disc:
  case SMediaInfo::ProbeInfo::FileType_Audio:
  case SMediaInfo::ProbeInfo::FileType_Video:
    return ListType_Details;

  case SMediaInfo::ProbeInfo::FileType_None:
  case SMediaInfo::ProbeInfo::FileType_Directory:
  case SMediaInfo::ProbeInfo::FileType_Drive:
  case SMediaInfo::ProbeInfo::FileType_Image:
    return ListType_Thumbnails;
  }

  return MediaServer::listType(virtualPath);
}

void MediaPlayerServer::customEvent(QEvent *e)
{
  if (e->type() == responseEventType)
  {
    ResponseEvent * const event = static_cast<ResponseEvent *>(e);

    SSandboxServer::sendHttpResponse(event->request, event->response, event->socket, false);
  }
  else
    MediaServer::customEvent(e);
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

  QSettings settings;
  settings.beginGroup(Module::pluginName);
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
      {
        QString result = i.value() + virtualPath.mid(s + 1);
        while (result.endsWith('/'))
          result = result.left(result.length() - 1);

        // Remove any virtual files
        const int ls = result.lastIndexOf('/');
        if ((ls >= 0) && ((ls + 1) < result.length()) && (result[ls + 1] == '.'))
          result = result.left(ls);

        return result;
      }
    }
  }

  return QString::null;
}

QString MediaPlayerServer::virtualFile(const QString &virtualPath)
{
  const int ls = virtualPath.lastIndexOf('/');
  if ((ls >= 0) && ((ls + 1) < virtualPath.length()) && (virtualPath[ls + 1] == '.'))
    return virtualPath.mid(ls + 1);

  return QString::null;
}

MediaPlayerServer::Item MediaPlayerServer::makeItem(const FileNode &node, int titleId)
{
  Item item;

  if (!node.isNull())
  {
    if ((node.fileType() == SMediaInfo::ProbeInfo::FileType_Directory))
    {
      item.isDir = true;
      item.title = node.fileName();
      item.path = virtualPath(node.filePath()) + '/';
      item.iconUrl = "/img/directory.png";
    }
    else if ((node.fileType() == SMediaInfo::ProbeInfo::FileType_Drive))
    {
      item.isDir = true;
      item.title = node.fileName();
      item.path = virtualPath(node.filePath()) + '/';
      item.iconUrl = "/img/drive.png";
    }
    else if ((node.fileType() == SMediaInfo::ProbeInfo::FileType_Disc) && (titleId < 0))
    {
      item.isDir = true;
      item.title = node.fileName();
      item.path = virtualPath(node.filePath()) + '/';
      item.iconUrl = "/img/media-optical.png";
    }
    else
    {
      item.isDir = false;
      item.path = virtualPath(node.filePath());

      if (node.isFormatProbed())
      {
        switch (node.fileType())
        {
        case SMediaInfo::ProbeInfo::FileType_Audio:
          item.type = node.isComplexFile() ? Item::Type_Audio : Item::Type_Music;
          break;

        case SMediaInfo::ProbeInfo::FileType_Video:
          item.type = node.isComplexFile() ? Item::Type_Movie : Item::Type_Video;
          break;

        case SMediaInfo::ProbeInfo::FileType_Disc:
          item.type = Item::Type_Movie;
          break;

        case SMediaInfo::ProbeInfo::FileType_Image:
          item.type = Item::Type_Image;
          break;

        case SMediaInfo::ProbeInfo::FileType_None:
        case SMediaInfo::ProbeInfo::FileType_Directory:
        case SMediaInfo::ProbeInfo::FileType_Drive:
          break;
        }
      }

      if (node.isContentProbed())
      {
        const QList<FileNode::ProbeInfo::Title> &titles = node.titles();

        if (!titles.isEmpty() && (titleId < titles.count()))
        {
          const FileNode::ProbeInfo::Title &title = titles[qMax(0, titleId)];

          if (item.type != Item::Type_None)
          {
            item.duration = title.duration.toSec();

            for (int a=0, an=title.audioStreams.count(); a < an; a++)
            for (int d=0, dn=title.dataStreams.count(); d <= dn; d++)
            {
              Item::Stream stream;

              stream.title = QString::number(a + 1) + ". " + title.audioStreams[a].fullName();
              stream.queryItems += qMakePair(QString("language"), title.audioStreams[a].toString());

              if (d < dn)
              {
                stream.title += ", " + QString::number(d + 1) + ". " + title.dataStreams[d].fullName() + " " + tr("subtitles");
                stream.queryItems += qMakePair(QString("subtitles"), title.dataStreams[d].toString());
              }
              else
                stream.queryItems += qMakePair(QString("subtitles"), QString());

              item.streams += stream;
            }

            foreach (const SMediaInfo::Chapter &chapter, title.chapters)
              item.chapters += Item::Chapter(chapter.title, chapter.begin.toSec());

            if (!title.audioStreams.isEmpty())
            {
              const SAudioCodec &codec = title.audioStreams.first().codec;
              item.audioFormat.setChannelSetup(codec.channelSetup());
              item.audioFormat.setSampleRate(codec.sampleRate());
            }

            if (!title.videoStreams.isEmpty())
            {
              const SVideoCodec &codec = title.videoStreams.first().codec;
              item.videoFormat.setSize(codec.size());
              item.videoFormat.setFrameRate(codec.frameRate());
            }

            if (!title.imageCodec.isNull())
              item.imageSize = title.imageCodec.size();
          }

          if (titles.count() > 1)
          {
            item.path += "/." + QString::number(qMax(0, titleId));
            item.title = tr("Title") + ' ' + QString::number(qMax(0, titleId) + 1);
          }
        }
      }

      if (item.type != Item::Type_None)
      {
        item.played = mediaDatabase->lastPlayed(node).isValid();
        item.url = item.path;
        item.iconUrl = item.url;
        item.iconUrl.addQueryItem("thumbnail", QString::null);

        if (item.title.isEmpty())
        {
          item.title = node.metadata("title").toString();
          if (item.title.isEmpty())
            item.title = node.fileName();
        }

        item.artist = node.metadata("author").toString();
        item.album = node.metadata("album").toString();
        item.track = node.metadata("track").toInt();
      }
      else
      {
        item.url = "";
        item.iconUrl = "/img/null.png";
        item.title = node.fileName();
      }
    }
  }

  return item;
}

MediaPlayerServer::Item MediaPlayerServer::makePlayAllItem(const QString &virtualPath)
{
  Item item;
  item.isDir = false;
  item.path = virtualPath + ".all";
  item.url = item.path;
  item.iconUrl = "/img/play-all.png";

  switch (dirType(virtualPath))
  {
  case SMediaInfo::ProbeInfo::FileType_Audio:
    item.type = Item::Type_AudioBroadcast;
    item.title = tr("Play all");
    break;

  case FileNode::ProbeInfo::FileType_None:
  case FileNode::ProbeInfo::FileType_Disc:
  case FileNode::ProbeInfo::FileType_Directory:
  case FileNode::ProbeInfo::FileType_Drive:
  case FileNode::ProbeInfo::FileType_Video:
    item.type = Item::Type_VideoBroadcast;
    item.title = tr("Play all");
    break;

  case FileNode::ProbeInfo::FileType_Image:
    item.type = Item::Type_VideoBroadcast;
    item.title = tr("Slideshow");
    break;
  }

  return item;
}

FileNode::ProbeInfo::FileType MediaPlayerServer::dirType(const QString &virtualPath)
{
  const QString path = realPath(virtualPath);
  const int numItems = mediaDatabase->countItems(path);

  if (numItems > 0)
  {
    int audio = 0, video = 0, image = 0;
    foreach (const FileNode &node, mediaDatabase->representativeItems(path))
    if (!node.isNull())
    switch (node.fileType())
    {
    case FileNode::ProbeInfo::FileType_Audio:     audio++; break;
    case FileNode::ProbeInfo::FileType_Disc:
    case FileNode::ProbeInfo::FileType_Video:     video++; break;
    case FileNode::ProbeInfo::FileType_Image:     image++; break;

    case FileNode::ProbeInfo::FileType_Directory:
    case FileNode::ProbeInfo::FileType_Drive:
    case FileNode::ProbeInfo::FileType_None:
      break;
    }

    if ((audio > video) && (audio > image))
      return FileNode::ProbeInfo::FileType_Audio;
    else if ((video > audio) && (video > image))
      return FileNode::ProbeInfo::FileType_Video;
    else if ((image > audio) && (image > video))
      return FileNode::ProbeInfo::FileType_Image;
  }
  else
  {
    const FileNode node = mediaDatabase->readNode(path);
    if (!node.isNull())
      return node.fileType();
  }

  return FileNode::ProbeInfo::FileType_None;
}

void MediaPlayerServer::consoleLine(const QString &line)
{
  if (line.startsWith("#PLAYED:"))
  {
    mediaDatabase->setLastPlayed(
        mediaDatabase->readNode(
            QString::fromUtf8(QByteArray::fromHex(line.mid(8).toAscii()))));
  }
}

void MediaPlayerServer::nodeRead(const FileNode &node)
{
  if (!node.isNull())
  {
    QMap<QString, QPair<SHttpServer::RequestMessage, QIODevice *> >::Iterator i =
        nodeReadQueue.find(node.filePath());

    if (i != nodeReadQueue.end())
    {
      if (i->first.url().hasQueryItem("thumbnail"))
      {
        const QSize size = SSize::fromString(i->first.url().queryItemValue("thumbnail")).size();
        QByteArray content;

        if (!node.titles().isEmpty())
        {
          int titleId = 0;

          const QString titleFile = virtualFile(i->first.file());
          if (titleFile.length() > 1)
            titleId = qBound(0, titleFile.mid(1).toInt(), node.titles().count());

          const FileNode::ProbeInfo::Title &title = node.titles()[titleId];
          if (!title.thumbnail.isNull())
            content = makeThumbnail(size, SImage(title.thumbnail), i->first.url().queryItemValue("overlay"));
        }

        if (content.isEmpty())
        {
          QString defaultIcon = ":/img/null.png";
          switch (node.fileType())
          {
          case FileNode::ProbeInfo::FileType_None:      defaultIcon = ":/img/null.png";           break;
          case FileNode::ProbeInfo::FileType_Audio:     defaultIcon = ":/img/audio-file.png";     break;
          case FileNode::ProbeInfo::FileType_Video:     defaultIcon = ":/img/video-file.png";     break;
          case FileNode::ProbeInfo::FileType_Image:     defaultIcon = ":/img/image-file.png";     break;
          case FileNode::ProbeInfo::FileType_Directory: defaultIcon = ":/img/directory.png";      break;
          case FileNode::ProbeInfo::FileType_Drive:     defaultIcon = ":/img/drive.png";          break;
          case FileNode::ProbeInfo::FileType_Disc:      defaultIcon = ":/img/media-optical.png";  break;
          }

          content = makeThumbnail(size, QImage(defaultIcon));
        }

        SSandboxServer::ResponseMessage response(
            i->first,
            SSandboxServer::Status_Ok,
            content,
            SHttpEngine::mimeImagePng);

        SHttpServerEngine::sendHttpResponse(i->first, response, i->second);
      }
      else
      {
        SSandboxServer::ResponseMessage response(
            i->first,
            SSandboxServer::Status_InternalServerError);

        SHttpServerEngine::sendHttpResponse(i->first, response, i->second);
      }

      nodeReadQueue.erase(i);
    }
  }
}

void MediaPlayerServer::aborted(void)
{
  for (QMap<QString, QPair<SHttpServer::RequestMessage, QIODevice *> >::Iterator i = nodeReadQueue.begin();
       i != nodeReadQueue.end();
       i = nodeReadQueue.erase(i))
  {
    SSandboxServer::ResponseMessage response(
        i->first,
        SSandboxServer::Status_InternalServerError);

    SHttpServerEngine::sendHttpResponse(i->first, response, i->second);
  }
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
