/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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
#include "module.h"
#include <QtConcurrent>
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

  QList<RootPath> paths;
  foreach (const QString &root, settings.value("RootPaths").toStringList())
  if (!root.isEmpty())
  {
    QUrl path;
    RootPath::Type type = RootPath::Auto;

    const int comma = root.indexOf(',');
    if (comma >= 0)
    {
      path = QUrl::fromEncoded(root.mid(comma + 1).toLatin1());
      if (root.left(comma).compare("Music", Qt::CaseInsensitive) == 0)
        type = RootPath::Music;
    }
    else
      path = QUrl::fromEncoded(root.toLatin1());

    if (path.isValid() && !path.scheme().isEmpty())
    {
      // Lame-decrypt the password.
      if (!path.password().isEmpty())
        path.setPassword(QString::fromUtf8(qUncompress(QByteArray::fromHex(path.password().toLatin1()))));

      paths.append(RootPath(path, type));
    }
  }

  setRootPaths(paths);
}

void MediaPlayerServer::initialize(MasterServer *masterServer)
{
  this->masterServer = masterServer;
  this->mediaDatabase = MediaDatabase::createInstance();

  MediaServer::initialize(masterServer);

  foreach (const QString &path, rootPaths.keys())
    mediaDatabase->isEmpty(realPath(serverPath() + path + '/').url);
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

MediaStream * MediaPlayerServer::streamVideo(const QUrl &request)
{
  const QUrlQuery query(request);

  const RootPath filePath = realPath(request.path());
  if (!filePath.url.isEmpty())
  {
    const QString vFile = virtualFile(request.path());
    if (vFile == "all")
    {
      QList<QUrl> files;
      for (QList<QUrl> dirs = QList<QUrl>() << filePath.url; !dirs.isEmpty(); )
      {
        const SMediaFilesystem dir(dirs.takeFirst());
        foreach (const QString &file, dir.entryList(QDir::Files, QDir::Name | QDir::IgnoreCase))
          files += dir.filePath(file);

        foreach (const QString &subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase))
          dirs += dir.filePath(subDir);
      }

      struct T
      {
        static SMediaInfo::ProbeInfo::FileType probeFileType(const QUrl &filePath)
        {
          return SMediaInfo(filePath).fileType();
        }
      };

      QList< QFuture<SMediaInfo::ProbeInfo::FileType> > futures;
      for (int n = files.count(), ni = qMax(1, n / 16), i = ni / 2; i < n; i += ni)
        futures += QtConcurrent::run(&T::probeFileType, files[i]);

      int audio = 0, video = 0, image = 0;
      for (int i=0; i<futures.count(); i++)
      switch (futures[i].result())
      {
      case SMediaInfo::ProbeInfo::FileType_Audio:   audio++; break;
      case SMediaInfo::ProbeInfo::FileType_Video:   video++; break;
      case SMediaInfo::ProbeInfo::FileType_Image:   image++; break;

      case SMediaInfo::ProbeInfo::FileType_None:
      case SMediaInfo::ProbeInfo::FileType_Directory:
      case SMediaInfo::ProbeInfo::FileType_Drive:
      case SMediaInfo::ProbeInfo::FileType_Disc:
      case SMediaInfo::ProbeInfo::FileType_Subtitle:
        break;
      }

      SMediaInfo::ProbeInfo::FileType fileType = SMediaInfo::ProbeInfo::FileType_None;
      if (query.hasQueryItem("music") ||
          ((audio > video) && (audio > image)))
      {
        fileType = (video > audio) && (video > image)
                   ? SMediaInfo::ProbeInfo::FileType_Video
                   : SMediaInfo::ProbeInfo::FileType_Audio;

        // Randomize files.
        QList<QUrl> random;
        while (!files.empty())
          random.append(files.takeAt(qrand() % files.count()));

        qSwap(random, files);
      }
      else if ((image > audio) && (image > video))
      {
        fileType = SMediaInfo::ProbeInfo::FileType_Image;

        // Only play files in this directory.
        files.clear();
        const SMediaFilesystem dir(filePath.url);
        foreach (const QString &file, dir.entryList(QDir::Files, QDir::Name | QDir::IgnoreCase))
          files += dir.filePath(file);
      }
      else if ((video > audio) && (video > image))
        fileType = SMediaInfo::ProbeInfo::FileType_Video;

      if ((fileType == SMediaInfo::ProbeInfo::FileType_Audio) ||
          (fileType == SMediaInfo::ProbeInfo::FileType_Video))
      {
        PlaylistStream * const stream = new PlaylistStream(files, fileType);
        if (stream->setup(request))
        if (stream->start())
          return stream;

        delete stream;
      }
      else if (fileType == SMediaInfo::ProbeInfo::FileType_Image)
      {
        SlideShowStream * const stream = new SlideShowStream(files);
        if (stream->setup(request))
        if (stream->start())
          return stream;

        delete stream;
      }
    }
    else
    {
      FileStream * const stream = new FileStream(filePath.url);
      if (stream->setup(request))
      if (stream->start())
      {
        connect(stream, SIGNAL(playbackProgress(QUrl,int)), SLOT(updatePlaybackProgress(QUrl,int)));

        return stream;
      }

      delete stream;
    }
  }

  return NULL;
}

HttpStatus MediaPlayerServer::sendPhoto(const QUrl &request, QByteArray &contentType, QIODevice *&response)
{
  const RootPath filePath = realPath(request.path());
  if (!filePath.url.isEmpty())
  {
    const QUrlQuery query(request);

    SSize size(4096, 4096);
    if (query.hasQueryItem("resolution"))
      size = SSize::fromString(query.queryItemValue("resolution"));

    QColor backgroundColor = Qt::black;
    if (query.hasQueryItem("bgcolor"))
      backgroundColor.setNamedColor('#' + query.queryItemValue("bgcolor"));

    QString format = "png";
    if (query.hasQueryItem("format"))
      format = query.queryItemValue("format");


    const QString contentFeatures = QByteArray::fromBase64(query.queryItemValue("contentFeatures").toLatin1());
    const MediaProfiles::ImageProfile imageProfile = mediaProfiles().imageProfileFor(contentFeatures);
    if (imageProfile != 0) // DLNA stream.
    {
      MediaProfiles::correctFormat(imageProfile, size);
      format = mediaProfiles().formatFor(imageProfile);
      contentType = mediaProfiles().mimeTypeFor(imageProfile);
    }

    const QImage result = mediaDatabase->readImage(filePath.url, size.size(), backgroundColor);
    if (!result.isNull())
    {
      QBuffer * const buffer = new QBuffer();
      buffer->open(QBuffer::ReadWrite);
      if (result.save(buffer, format.toLatin1(), 80))
      {
        buffer->close();
        contentType = "image/" + format.toLower().toLatin1();
        response = buffer;
        return HttpStatus_Ok;
      }

      delete buffer;
    }
  }

  return HttpStatus_NotFound;
}

QList<MediaPlayerServer::Item> MediaPlayerServer::listItems(const QString &virtualPath, int start, int &count)
{
  const bool returnAll = count == 0;

  QList<Item> result;

  if (virtualPath != serverPath())
  {
    QString virtualFilePath = virtualPath;
    while (virtualFilePath.endsWith('/'))
      virtualFilePath = virtualFilePath.left(virtualFilePath.length() - 1);

    const SMediaInfo node = mediaDatabase->readNodeContent(realPath(virtualFilePath).url);
    if (!node.isNull() && !node.titles().isEmpty())
    {
      const int numTitles = node.titles().count();
      for (int i=start, n=0; (i<numTitles) && (n<count); i++, n++)
        result += makeItem(SMediaInfo::ProbeInfo::FileType_Directory, node, i);

      count = numTitles;
    }
    else
    {
      const DirType type = dirType(virtualPath);
      if ((type.pathType == RootPath::Music) ||
          (type.fileType == SMediaInfo::ProbeInfo::FileType_Audio) ||
          (type.fileType == SMediaInfo::ProbeInfo::FileType_Video) ||
          (type.fileType == SMediaInfo::ProbeInfo::FileType_Image))
      {
        // Add play all item
        int first = start;
        if ((start == 0) && (count > 1))
          count--;
        else if (start > 0)
          first--;

        foreach (const SMediaInfo &node, mediaDatabase->listItems(realPath(virtualPath).url, first, count))
          result += makeItem(type, node);

        if (start == 0)
          result.prepend(makePlayAllItem(virtualPath));

        count++;
      }
      else
      {
        foreach (const SMediaInfo &node, mediaDatabase->listItems(realPath(virtualPath).url, start, count))
          result += makeItem(type, node);
      }
    }
  }
  else
  {
    QStringList paths = rootPaths.keys();
    for (QStringList::Iterator i=paths.begin(); i!=paths.end(); )
    if (mediaDatabase->isEmpty(realPath(virtualPath + *i + '/').url))
      i = paths.erase(i);
    else
      i++;

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
  const QString vFile = virtualFile(virtualPath);
  if (vFile == "all")
    return makePlayAllItem(virtualPath.left(virtualPath.length() - 4));
  else if (!vFile.isEmpty() && vFile[0].isNumber())
    return makeItem(dirType(virtualPath), mediaDatabase->readNodeContent(realPath(virtualPath).url), vFile.toInt());
  else
    return makeItem(dirType(virtualPath), mediaDatabase->readNodeContent(realPath(virtualPath).url));
}

MediaPlayerServer::ListType MediaPlayerServer::listType(const QString &virtualPath)
{
  switch (dirType(virtualPath).fileType)
  {
  case SMediaInfo::ProbeInfo::FileType_Disc:
  case SMediaInfo::ProbeInfo::FileType_Audio:
  case SMediaInfo::ProbeInfo::FileType_Video:
  case SMediaInfo::ProbeInfo::FileType_Subtitle:
    return ListType_Details;

  case SMediaInfo::ProbeInfo::FileType_None:
  case SMediaInfo::ProbeInfo::FileType_Directory:
  case SMediaInfo::ProbeInfo::FileType_Drive:
  case SMediaInfo::ProbeInfo::FileType_Image:
    return ListType_Thumbnails;
  }

  return MediaServer::listType(virtualPath);
}

void MediaPlayerServer::setRootPaths(const QList<RootPath> &paths)
{
  rootPaths.clear();

  QStringList encodedPaths;
  foreach (RootPath path, paths)
  {
    const QString baseLabel = dirLabel(path.url.path());
    int index = 1;
    QString label = baseLabel;
    while (rootPaths.contains(label))
      label = baseLabel + '_' + QString::number(++index);

    rootPaths.insert(label, path);

    // Lame-encrypt the password.
    if (!path.url.password().isEmpty())
      path.url.setPassword(qCompress(path.url.password().toUtf8()).toHex());

    switch (path.type)
    {
    case RootPath::Auto:  encodedPaths += "Auto,"   + path.url.toEncoded();   break;
    case RootPath::Music: encodedPaths += "Music,"  + path.url.toEncoded();   break;
    }
  }

  QSettings settings;
  settings.beginGroup(Module::pluginName);
  settings.setValue("RootPaths", encodedPaths);
}

QString MediaPlayerServer::virtualPath(const QUrl &realPath) const
{
  const QString realPathString = realPath.toString();

  for (QMap<QString, RootPath>::ConstIterator i=rootPaths.begin(); i!=rootPaths.end(); i++)
  {
    QString rootString = i.value().url.toString();
    if (!rootString.endsWith('/'))
      rootString += '/';

    if (realPathString.startsWith(rootString))
      return serverPath() + i.key() + '/' + realPathString.mid(rootString.length());
  }

  return QString::null;
}

MediaPlayerServer::RootPath MediaPlayerServer::realPath(const QString &virtualPath) const
{
  const QString basepath = serverPath();

  if (virtualPath.startsWith(basepath))
  {
    const int s = virtualPath.indexOf('/', basepath.length());
    if (s >= 1)
    {
      QMap<QString, RootPath>::ConstIterator i = rootPaths.find(virtualPath.mid(basepath.length(), s - basepath.length()));
      if (i != rootPaths.end())
      {
        QString rootString = i->url.toString();
        if (!rootString.endsWith('/'))
          rootString += '/';

        QString result = rootString + virtualPath.mid(s + 1);

        // Remove any virtual files
        const int ls = result.lastIndexOf('/');
        if ((ls >= 0) && ((ls + 1) < result.length()) && (result[ls + 1] == '.'))
          result = result.left(ls);

        return RootPath(result, i->type);
      }
    }
  }

  return RootPath();
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

MediaPlayerServer::Item MediaPlayerServer::makeItem(DirType dirType, const SMediaInfo &node, int titleId)
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
        item.type = (dirType.pathType == RootPath::Music)
                    ? Item::Type_Music
                    : Item::Type_Audio;

        break;

      case SMediaInfo::ProbeInfo::FileType_Video:
      case SMediaInfo::ProbeInfo::FileType_Disc:
        item.type = (dirType.pathType == RootPath::Music)
                    ? Item::Type_MusicVideo
                    : Item::Type_Movie;

        break;

      case SMediaInfo::ProbeInfo::FileType_Image:
        item.type = Item::Type_Image;
        break;

      case SMediaInfo::ProbeInfo::FileType_None:
      case SMediaInfo::ProbeInfo::FileType_Directory:
      case SMediaInfo::ProbeInfo::FileType_Drive:
      case SMediaInfo::ProbeInfo::FileType_Subtitle:
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
        item.url = item.path;
        item.iconUrl = item.url;
        QUrlQuery q(item.iconUrl);
        q.addQueryItem("thumbnail", QString::null);
        item.iconUrl.setQuery(q);

        if (item.title.isEmpty())
          item.title = node.baseName();

        item.artist = node.metadata("author").toString();
        item.album = node.metadata("album").toString();
        item.track = node.metadata("track").toInt();

        item.lastPosition = mediaDatabase->getLastPlaybackPosition(node.filePath());
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

  const DirType type = dirType(virtualPath);
  switch (type.pathType)
  {
  case RootPath::Auto:
    switch (type.fileType)
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
    case SMediaInfo::ProbeInfo::FileType_Subtitle:
      item.type = Item::Type_VideoBroadcast;
      item.title = tr("Play all");
      break;

    case SMediaInfo::ProbeInfo::FileType_Image:
      item.type = Item::Type_VideoBroadcast;
      item.title = tr("Slideshow");
      break;
    }

    break;

  case RootPath::Music:
    switch (type.fileType)
    {
    case SMediaInfo::ProbeInfo::FileType_Audio:
    case SMediaInfo::ProbeInfo::FileType_Image:
      item.type = Item::Type_Music;
      item.title = tr("Play all");
      break;

    case SMediaInfo::ProbeInfo::FileType_None:
    case SMediaInfo::ProbeInfo::FileType_Disc:
    case SMediaInfo::ProbeInfo::FileType_Directory:
    case SMediaInfo::ProbeInfo::FileType_Drive:
    case SMediaInfo::ProbeInfo::FileType_Video:
    case SMediaInfo::ProbeInfo::FileType_Subtitle:
      item.type = Item::Type_MusicVideo;
      item.title = tr("Play all");
      break;
    }

    break;
  }

  return item;
}

MediaPlayerServer::DirType MediaPlayerServer::dirType(const QString &virtualPath)
{
  if (virtualPath != serverPath())
  {
    const RootPath path = realPath(virtualPath);

    int audio = 0, video = 0, image = 0, count = numRepresentativeItems;
    foreach (const SMediaInfo &node, mediaDatabase->representativeItems(path.url, count))
    if (!node.isNull())
    switch (node.fileType())
    {
    case SMediaInfo::ProbeInfo::FileType_Audio:     audio++; break;
    case SMediaInfo::ProbeInfo::FileType_Disc:
    case SMediaInfo::ProbeInfo::FileType_Video:     video++; break;
    case SMediaInfo::ProbeInfo::FileType_Image:     image++; break;

    case SMediaInfo::ProbeInfo::FileType_Directory:
    case SMediaInfo::ProbeInfo::FileType_Drive:
    case SMediaInfo::ProbeInfo::FileType_Subtitle:
    case SMediaInfo::ProbeInfo::FileType_None:
      break;
    }

    if ((audio > video) && (audio > image) && (audio >= (numRepresentativeItems / 2)))
      return DirType(SMediaInfo::ProbeInfo::FileType_Audio, path.type);
    else if ((video > audio) && (video > image) && (video >= (numRepresentativeItems / 2)))
      return DirType(SMediaInfo::ProbeInfo::FileType_Video, path.type);
    else if ((image > audio) && (image > video) && (image >= (numRepresentativeItems / 2)))
      return DirType(SMediaInfo::ProbeInfo::FileType_Image, path.type);

    return DirType(SMediaInfo::ProbeInfo::FileType_None, path.type);
  }

  return SMediaInfo::ProbeInfo::FileType_None;
}

void MediaPlayerServer::updatePlaybackProgress(const QUrl &filePath, int position)
{
  mediaDatabase->setLastPlaybackPosition(filePath, qMax(0, ((position - 29) / 60) * 60));
}


FileStream::FileStream(const QUrl &filePath)
  : MediaTranscodeStream(),
    file(this, filePath),
    filePath(filePath),
    lastPosition(0)
{
  connect(&file, SIGNAL(finished()), SLOT(stop()));

  connect(&file, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&file, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&file, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));

  connect(&playbackProgressTimer, SIGNAL(timeout()), SLOT(updatePlaybackProgress()));
  playbackProgressTimer.setInterval(30000);
  playbackProgressTimer.setTimerType(Qt::VeryCoarseTimer);
}

FileStream::~FileStream()
{
  proxy.close();
  stop();
}

bool FileStream::setup(const QUrl &request)
{
  return MediaTranscodeStream::setup(request, &file);
}

bool FileStream::start(void)
{
  if (MediaTranscodeStream::start())
  {
    playbackProgressTimer.start();
    return true;
  }

  return false;
}

void FileStream::stop(void)
{
  playbackProgressTimer.stop();

  MediaTranscodeStream::stop();
}

void FileStream::updatePlaybackProgress(void)
{
  const int position = file.position().toSec();
  if (position != lastPosition)
  {
    emit playbackProgress(filePath, position);

    lastPosition = position;
  }
}


PlaylistStream::PlaylistStream(const QList<QUrl> &files, SMediaInfo::ProbeInfo::FileType fileType)
  : MediaTranscodeStream(),
    playlistNode(this, files, fileType)
{
}

PlaylistStream::~PlaylistStream()
{
  proxy.close();
  stop();
}

bool PlaylistStream::setup(const QUrl &request)
{
  if (MediaTranscodeStream::setup(
        request,
        &playlistNode,
        STime(),
        true))
  {
    connect(&playlistNode, SIGNAL(finished()), SLOT(stop()));
    connect(&playlistNode, SIGNAL(opened(QUrl)), SLOT(opened(QUrl)));
    connect(&playlistNode, SIGNAL(closed(QUrl)), SLOT(closed(QUrl)));
    connect(&playlistNode, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
    connect(&playlistNode, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
    connect(&playlistNode, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));

    return true;
  }

  return false;
}

void PlaylistStream::opened(const QUrl &filePath)
{
  currentFile = filePath;
}

void PlaylistStream::closed(const QUrl &filePath)
{
  if (currentFile == filePath)
    currentFile = QString::null;
}


SlideShowStream::SlideShowStream(const QList<QUrl> &files)
  : MediaStream(),
    slideShow(this, files)
{
}

SlideShowStream::~SlideShowStream()
{
  proxy.close();
  stop();
}

bool SlideShowStream::setup(const QUrl &request)
{
  QUrlQuery query(request);

  if (query.hasQueryItem("slideduration"))
    slideShow.setSlideDuration(STime::fromMSec(query.queryItemValue("slideduration").toInt()));

  if (MediaStream::setup(
          request,
          slideShow.duration(),
          SAudioFormat(SAudioFormat::Format_Invalid, SAudioFormat::Channels_Stereo, 48000),
          SVideoFormat(SVideoFormat::Format_Invalid, slideShow.size(), SInterval::fromFrequency(slideShow.frameRate)),
          false,
          SInterfaces::AudioEncoder::Flag_None,
          SInterfaces::VideoEncoder::Flag_Slideshow,
          slideShow.framesPerSlide()))
  {
    connect(&slideShow, SIGNAL(finished()), SLOT(stop()));
    connect(&slideShow, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
    connect(&slideShow, SIGNAL(output(SVideoBuffer)), &video->subtitleRenderer, SLOT(input(SVideoBuffer)));

    slideShow.setSize(video->resizer.size());

    return true;
  }
  else
    return false;
}

} } // End of namespaces
