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

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char          MediaPlayerServer::dirSplit = '\t';
const QEvent::Type  MediaPlayerServer::responseEventType = QEvent::Type(QEvent::registerEventType());
const int           MediaPlayerServer::maxSongDurationMin = 7;

MediaPlayerServer::MediaPlayerServer(const QString &, QObject *parent)
  : MediaServer(parent),
    masterServer(NULL),
    mediaDatabase(NULL)
{
  QSettings settings;
  settings.beginGroup(Module::pluginName);

  QList<QUrl> paths;
  foreach (const QString &root, settings.value("RootPaths").toStringList())
  if (!root.isEmpty())
  {
    QUrl path = QUrl::fromEncoded(root.toAscii());
    if (path.isValid() && !path.scheme().isEmpty())
    {
      // Lame-decrypt the password.
      if (!path.password().isEmpty())
        path.setPassword(QString::fromUtf8(qUncompress(QByteArray::fromHex(path.password().toAscii()))));

      paths.append(path);
    }
  }

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

  const QUrl filePath = realPath(request.file());
  if (!filePath.isEmpty())
  {
    QUrl rurl;
    rurl.setPath(MediaPlayerSandbox::path);
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

    const QString vFile = virtualFile(request.file());
    if ((vFile.length() > 1) && vFile[1].isNumber())
    {
      rurl.addQueryItem("title", vFile.mid(1));
      rurl.addQueryItem("play", QString::null);
    }
    else if (vFile.length() > 1)
      rurl.addQueryItem("play", vFile.mid(1));
    else
      rurl.addQueryItem("play", QString::null);

    Stream *stream = new Stream(this, sandbox, request.path());
    if (stream->setup(rurl, filePath.toString().toUtf8()))
      return stream; // The graph owns the socket now.

    delete stream;
  }

  disconnect(sandbox, SIGNAL(consoleLine(QString)), this, SLOT(consoleLine(QString)));
  masterServer->recycleSandbox(sandbox);

  return NULL;
}

SHttpServer::ResponseMessage MediaPlayerServer::sendPhoto(const SHttpServer::RequestMessage &request)
{
  const QUrl filePath = realPath(request.file());
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

QList<MediaPlayerServer::Item> MediaPlayerServer::listItems(const QString &virtualPath, int start, int &count)
{
  const bool returnAll = count == 0;
  QList<Item> result;

  if (virtualPath != serverPath())
  {
    const QUrl path = realPath(virtualPath);

    const FileNode node = mediaDatabase->readNode(path);
    if (!node.isNull() && !node.titles().isEmpty())
    {
      const int numTitles = node.titles().count();
      for (int i=start, n=0; (i<numTitles) && (returnAll || (n<count)); i++, n++)
        result += makeItem(node, i);

      count = numTitles;
    }
    else
    {
      if ((start == 0) && (count > 1)) // For play all item
        count--;

      foreach (const FileNode &node, mediaDatabase->listItems(path, start, count))
        result += makeItem(node);

      if (start == 0) // For play all item
        result.prepend(makePlayAllItem(virtualPath));

      count++; // For play all item
    }
  }
  else
  {
    const QStringList paths = rootPaths.keys();
    for (int i=start, n=0; (i<paths.count()) && (returnAll || (n<count)); i++, n++)
    {
      Item item;
      item.isDir = true;
      item.title = paths[i];
      item.path = virtualPath + paths[i] + '/';
      item.iconUrl = "/img/directory.png";

      result += item;
    }

    count = paths.count();
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

void MediaPlayerServer::setRootPaths(const QList<QUrl> &paths)
{
  rootPaths.clear();

  QStringList encodedPaths;
  foreach (QUrl path, paths)
  {
    const QString baseLabel = dirLabel(path.path());
    int index = 1;
    QString label = baseLabel;
    while (rootPaths.contains(label))
      label = baseLabel + '_' + QString::number(++index);

    rootPaths.insert(label, path);

    // Lame-encrypt the password.
    if (!path.password().isEmpty())
      path.setPassword(qCompress(path.password().toUtf8()).toHex());

    encodedPaths += path.toEncoded();
  }

  QSettings settings;
  settings.beginGroup(Module::pluginName);
  settings.setValue("RootPaths", encodedPaths);
}

QString MediaPlayerServer::virtualPath(const QUrl &realPath) const
{
  const QString realPathString = realPath.toString();

  for (QMap<QString, QUrl>::ConstIterator i=rootPaths.begin(); i!=rootPaths.end(); i++)
  {
    QString rootString = i.value().toString();
    if (!rootString.endsWith('/'))
      rootString += '/';

    if (realPathString.startsWith(rootString))
      return serverPath() + i.key() + '/' + realPathString.mid(rootString.length());
  }

  return QString::null;
}

QUrl MediaPlayerServer::realPath(const QString &virtualPath) const
{
  const QString basepath = serverPath();

  if (virtualPath.startsWith(basepath))
  {
    const int s = virtualPath.indexOf('/', basepath.length());
    if (s >= 1)
    {
      QMap<QString, QUrl>::ConstIterator i = rootPaths.find(virtualPath.mid(basepath.length(), s - basepath.length()));
      if (i != rootPaths.end())
      {
        QString rootString = i.value().toString();
        if (!rootString.endsWith('/'))
          rootString += '/';

        QString result = rootString + virtualPath.mid(s + 1);

        // Remove any virtual files
        const int ls = result.lastIndexOf('/');
        if ((ls >= 0) && ((ls + 1) < result.length()) && (result[ls + 1] == '.'))
          result = result.left(ls);

        return result;
      }
    }
  }

  return QUrl();
}

QString MediaPlayerServer::virtualFile(const QString &virtualPath)
{
  const int ls = virtualPath.lastIndexOf('/');
  if ((ls >= 0) && ((ls + 1) < virtualPath.length()) && (virtualPath[ls + 1] == '.'))
    return virtualPath.mid(ls + 1);

  return QString::null;
}

QString MediaPlayerServer::dirLabel(const QString &path)
{
#ifdef Q_OS_WIN
  if (((path.length() == 2) || (path.length() == 3)) &&
      path[0].isLetterOrNumber() && (path[1] == ':'))
  {
    WCHAR szVolumeName[MAX_PATH+1];
    WCHAR szFileSystemName[MAX_PATH+1];
    DWORD dwSerialNumber = 0;
    DWORD dwMaxFileNameLength = MAX_PATH;
    DWORD dwFileSystemFlags = 0;

    if (::GetVolumeInformationW(reinterpret_cast<const WCHAR *>(path.utf16()),
                                szVolumeName, sizeof(szVolumeName) / sizeof(*szVolumeName),
                                &dwSerialNumber,
                                &dwMaxFileNameLength,
                                &dwFileSystemFlags,
                                szFileSystemName, sizeof(szFileSystemName) / sizeof(*szFileSystemName)))
    {
      return QString::fromUtf16((const ushort *)szVolumeName).trimmed();
    }
  }
#else
  if (path == "/")
    return tr("Root");
#endif

  const QStringList dirs = path.split('/', QString::SkipEmptyParts);
  if (!dirs.isEmpty())
    return dirs.last();

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
          item.title = node.baseName();

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
  if (virtualPath != serverPath())
  {
    const QUrl path = realPath(virtualPath);

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
    QMap<QUrl, QPair<SHttpServer::RequestMessage, QIODevice *> >::Iterator i =
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

        SHttpServer::ResponseMessage response(
            i->first,
            SHttpServer::Status_Ok,
            content,
            SHttpEngine::mimeImagePng);

        masterServer->httpServer()->sendHttpResponse(i->first, response, i->second);
      }
      else
      {
        SHttpServer::ResponseMessage response(
            i->first,
            SHttpServer::Status_InternalServerError);

        masterServer->httpServer()->sendHttpResponse(i->first, response, i->second);
      }

      nodeReadQueue.erase(i);
    }
  }
}

void MediaPlayerServer::aborted(void)
{
  for (QMap<QUrl, QPair<SHttpServer::RequestMessage, QIODevice *> >::Iterator i = nodeReadQueue.begin();
       i != nodeReadQueue.end();
       i = nodeReadQueue.erase(i))
  {
    SHttpServer::ResponseMessage response(
        i->first,
        SHttpServer::Status_InternalServerError);

    masterServer->httpServer()->sendHttpResponse(i->first, response, i->second);
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

  sandbox->openRequest(message, &proxy, SLOT(setSource(QIODevice *, SHttpEngine *)), Qt::DirectConnection);

  return true;
}

} } // End of namespaces
