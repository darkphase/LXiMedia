/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

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

  MediaServer::initialize(masterServer);

  foreach (const QString &path, rootPaths.keys())
    mediaDatabase->isEmpty(realPath(serverPath() + path + '/'));
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
    if (!vFile.isEmpty() && vFile[0].isNumber())
    {
      rurl.addQueryItem("title", vFile.mid(1));
      rurl.addQueryItem("play", QString::null);
    }
    else if (!vFile.isEmpty())
      rurl.addQueryItem("play", vFile);
    else
      rurl.addQueryItem("play", QString::null);

    Stream *stream = new Stream(this, sandbox, request.path());
    if (stream->setup(rurl, filePath.toString().toUtf8()))
      return stream; // The graph owns the socket now.

    delete stream;
  }

  disconnect(sandbox, SIGNAL(consoleLine(QString)), this, SLOT(consoleLine(QString)));
  delete sandbox;

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

    QColor backgroundColor = Qt::black;
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
  QList<Item> result;

  // Limit the maximum number of returned results to prevent probing too long
  // and not responding timely.
  if ((count == 0) || (count > mediaDatabase->preferredItemCount()))
    count = mediaDatabase->preferredItemCount();

  if (virtualPath != serverPath())
  {
    QString virtualFilePath = virtualPath;
    while (virtualFilePath.endsWith('/'))
      virtualFilePath = virtualFilePath.left(virtualFilePath.length() - 1);

    const SMediaInfo node = mediaDatabase->readNodeContent(realPath(virtualFilePath));
    if (!node.isNull() && !node.titles().isEmpty())
    {
      const int numTitles = node.titles().count();
      for (int i=start, n=0; (i<numTitles) && (n<count); i++, n++)
        result += makeItem(SMediaInfo::ProbeInfo::FileType_Directory, node, i);

      count = numTitles;
    }
    else
    {
      const SMediaInfo::ProbeInfo::FileType type = dirType(virtualPath);
      if ((type == SMediaInfo::ProbeInfo::FileType_Audio) ||
          (type == SMediaInfo::ProbeInfo::FileType_Video) ||
          (type == SMediaInfo::ProbeInfo::FileType_Image))
      {
        // Add play all item
        int first = start;
        if ((start == 0) && (count > 1))
          count--;
        else if (start > 0)
          first--;

        foreach (const SMediaInfo &node, mediaDatabase->listItems(realPath(virtualPath), first, count))
          result += makeItem(type, node);

        if (start == 0)
          result.prepend(makePlayAllItem(virtualPath));

        count++;
      }
      else
      {
        foreach (const SMediaInfo &node, mediaDatabase->listItems(realPath(virtualPath), start, count))
          result += makeItem(type, node);
      }
    }
  }
  else
  {
    QStringList paths = rootPaths.keys();
    for (QStringList::Iterator i=paths.begin(); i!=paths.end(); )
    if (mediaDatabase->isEmpty(realPath(virtualPath + *i + '/')))
      i = paths.erase(i);
    else
      i++;

    for (int i=start, n=0; (i<paths.count()) && (n<count); i++, n++)
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
  const QString vFile = virtualFile(virtualPath);
  if (vFile == "all")
    return makePlayAllItem(virtualPath.left(virtualPath.length() - 4));
  else if (!vFile.isEmpty() && vFile[0].isNumber())
    return makeItem(dirType(virtualPath), mediaDatabase->readNodeContent(realPath(virtualPath)), vFile.toInt());
  else
    return makeItem(dirType(virtualPath), mediaDatabase->readNodeContent(realPath(virtualPath)));
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
    return virtualPath.mid(ls + 2);

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

MediaPlayerServer::Item MediaPlayerServer::makeItem(SMediaInfo::ProbeInfo::FileType dirType, const SMediaInfo &node, int titleId)
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

      switch (node.fileType())
      {
      case SMediaInfo::ProbeInfo::FileType_Audio:
        item.type = (dirType == SMediaInfo::ProbeInfo::FileType_Audio)
                    ? Item::Type_Music
                    : Item::Type_Audio;

        break;

      case SMediaInfo::ProbeInfo::FileType_Video:
        item.type = Item::Type_Movie;
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

      const QList<SMediaInfo::ProbeInfo::Title> &titles = node.titles();
      if (!titles.isEmpty() && (titleId < titles.count()))
      {
        const SMediaInfo::ProbeInfo::Title &title = titles[qMax(0, titleId)];

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

        if ((node.fileType() == SMediaInfo::ProbeInfo::FileType_Disc) || (titles.count() > 1))
        {
          item.path += "/." + QString::number(qMax(0, titleId));
          item.title = tr("Title") + ' ' + QString::number(qMax(0, titleId) + 1);
        }
      }

      if (item.type != Item::Type_None)
      {
        item.played = mediaDatabase->lastPlayed(node.filePath()).first.isValid();
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

  case SMediaInfo::ProbeInfo::FileType_None:
  case SMediaInfo::ProbeInfo::FileType_Disc:
  case SMediaInfo::ProbeInfo::FileType_Directory:
  case SMediaInfo::ProbeInfo::FileType_Drive:
  case SMediaInfo::ProbeInfo::FileType_Video:
    item.type = Item::Type_VideoBroadcast;
    item.title = tr("Play all");
    break;

  case SMediaInfo::ProbeInfo::FileType_Image:
    item.type = Item::Type_VideoBroadcast;
    item.title = tr("Slideshow");
    break;
  }

  return item;
}

SMediaInfo::ProbeInfo::FileType MediaPlayerServer::dirType(const QString &virtualPath)
{
  if (virtualPath != serverPath())
  {
    const QUrl path = realPath(virtualPath);

    int audio = 0, video = 0, image = 0, count = numRepresentativeItems;
    foreach (const SMediaInfo &node, mediaDatabase->representativeItems(path, count))
    if (!node.isNull())
    switch (node.fileType())
    {
    case SMediaInfo::ProbeInfo::FileType_Audio:     audio++; break;
    case SMediaInfo::ProbeInfo::FileType_Disc:
    case SMediaInfo::ProbeInfo::FileType_Video:     video++; break;
    case SMediaInfo::ProbeInfo::FileType_Image:     image++; break;

    case SMediaInfo::ProbeInfo::FileType_Directory:
    case SMediaInfo::ProbeInfo::FileType_Drive:
    case SMediaInfo::ProbeInfo::FileType_None:
      break;
    }

    if ((audio > video) && (audio > image) && (audio >= (numRepresentativeItems / 2)))
      return SMediaInfo::ProbeInfo::FileType_Audio;
    else if ((video > audio) && (video > image) && (video >= (numRepresentativeItems / 2)))
      return SMediaInfo::ProbeInfo::FileType_Video;
    else if ((image > audio) && (image > video) && (image >= (numRepresentativeItems / 2)))
      return SMediaInfo::ProbeInfo::FileType_Image;
  }

  return SMediaInfo::ProbeInfo::FileType_None;
}

void MediaPlayerServer::consoleLine(const QString &line)
{
  if (line.startsWith("#PLAYED:"))
    mediaDatabase->setLastPlayed(QUrl::fromEncoded(QByteArray::fromHex(line.mid(8).toAscii())));
}


MediaPlayerServer::Stream::Stream(MediaPlayerServer *parent, SSandboxClient *sandbox, const QString &url)
  : MediaServer::Stream(parent, url),
    sandbox(sandbox)
{
}

MediaPlayerServer::Stream::~Stream()
{
  disconnect(sandbox, SIGNAL(consoleLine(QString)), parent, SLOT(consoleLine(QString)));
  delete sandbox;
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
