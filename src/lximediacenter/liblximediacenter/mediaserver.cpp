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
#include "globalsettings.h"
#include "htmlparser.h"
#include "mediastream.h"
#include <LXiStreamGui>

namespace LXiMediaCenter {

struct MediaServer::Data
{
  static const int              maxStreams = 64;

  MasterServer                * masterServer;
  QList<Stream *>               streams;
  QList<Stream *>               reusableStreams;
  QTimer                        cleanStreamsTimer;
};

const qint32  MediaServer::defaultDirSortOrder  = -65536;
const qint32  MediaServer::defaultFileSortOrder = 0;
const int     MediaServer::seekBySecs = 120;


MediaServer::MediaServer(QObject *parent)
  : BackendServer(parent),
    d(new Data())
{
  d->masterServer = NULL;

  connect(&d->cleanStreamsTimer, SIGNAL(timeout()), SLOT(cleanStreams()));
}

MediaServer::~MediaServer()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void MediaServer::initialize(MasterServer *masterServer)
{
  d->masterServer = masterServer;

  BackendServer::initialize(masterServer);

  d->masterServer->httpServer()->registerCallback(serverPath(), this);
  d->masterServer->contentDirectory()->registerCallback(serverPath(), this);

  d->cleanStreamsTimer.start(30000);
}

void MediaServer::close(void)
{
  d->cleanStreamsTimer.stop();

  BackendServer::close();

  d->masterServer->httpServer()->unregisterCallback(this);
  d->masterServer->contentDirectory()->unregisterCallback(this);
}

MediaProfiles & MediaServer::mediaProfiles(void)
{
  static MediaProfiles p;

  return p;
}

QSet<QString> & MediaServer::activeClients(void)
{
  static QSet<QString> c;

  return c;
}

void MediaServer::cleanStreams(void)
{
  QList<Stream *> obsolete;
  foreach (Stream *stream, d->streams)
  if (!stream->proxy.isConnected())
    obsolete += stream;

  foreach (Stream *stream, obsolete)
    delete stream;
}

SHttpServer::ResponseMessage MediaServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isGet())
  {
    const MediaServer::File file(request);
    if (file.fileName().isEmpty())
    {
      if (file.url().hasQueryItem("items"))
      {
        const QStringList range = file.url().queryItemValue("items").split(',');
        unsigned start = 0, count = 0;
        if (!range.isEmpty())
        {
          start = range[0].toUInt();
          if (range.count() >= 2)
            count = range[1].toUInt();
        }

        ThumbnailListItemList thumbItems;
        foreach (const SUPnPContentDirectory::Item &item, listItems(basePath(file.url().path()), start, count))
        {
          if (item.isDir)
          {
            ThumbnailListItem thumbItem;
            thumbItem.title = item.title;
            thumbItem.iconurl = item.iconUrl;
            thumbItem.url = item.title + '/';

            thumbItems.append(thumbItem);
          }
          else
          {
            ThumbnailListItem thumbItem;
            thumbItem.title = item.title;
            thumbItem.iconurl = item.iconUrl;
            thumbItem.url = item.url;
            thumbItem.url.setPath(thumbItem.url.path() + ".html");
            thumbItem.played = item.played;

            if (item.played)
            {
              thumbItem.iconurl.removeQueryItem("overlay");
              thumbItem.iconurl.addQueryItem("overlay", "played");
            }

            thumbItems.append(thumbItem);
          }
        }

        return makeResponse(request, buildThumbnailItems(thumbItems), "text/html;charset=utf-8", false);
      }
      else
        return makeHtmlContent(request, file.url(), buildThumbnailLoader(dirName(file.url().path())), headList);
    }
    else if (!request.isHead())
    {
      foreach (Stream *stream, d->streams)
      if (stream->url == request.path())
      if (stream->proxy.addSocket(socket))
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_None);

      Stream * const stream = streamVideo(request);
      if (stream)
      {
        stream->proxy.addSocket(socket);
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_None);
      }
      else
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
    }
    else // Return head only.
    {
      SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
      response.setField("transferMode.dlna.org", "Streaming");
      if (file.url().hasQueryItem("contentFeatures"))
        response.setField("contentFeatures.dlna.org", QByteArray::fromHex(file.url().queryItemValue("contentFeatures").toAscii()));

      return response;
    }
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

int MediaServer::countContentDirItems(const QString &peer, const QString &dirPath)
{
  if (!activeClients().contains(peer))
    activeClients().insert(peer);

  QString subPath = dirPath.mid(serverPath().length());
  subPath = subPath.startsWith('/') ? subPath : ('/' + subPath);

  return countItems(subPath);
}

QList<SUPnPContentDirectory::Item> MediaServer::listContentDirItems(const QString &peer, const QString &dirPath, unsigned start, unsigned count)
{
  if (!activeClients().contains(peer))
    activeClients().insert(peer);

  QString subPath = dirPath.mid(serverPath().length());
  subPath = subPath.startsWith('/') ? subPath : ('/' + subPath);

  QString basePath = serverPath();
  basePath = basePath.endsWith('/') ? basePath.left(basePath.length() - 1) : basePath;
  basePath += subPath;
  basePath = basePath.endsWith('/') ? basePath.left(basePath.length() - 1) : basePath;

  QList<SUPnPContentDirectory::Item> result;
  foreach (Item item, listItems(subPath, start, count))
  {
    const QString itemPath = item.url.path(), iconPath = item.iconUrl.path();

    if (!itemPath.isEmpty())
      item.url.setPath(itemPath.startsWith('/') ? itemPath : (basePath + '/' + itemPath));

    if (!iconPath.isEmpty())
      item.iconUrl.setPath(iconPath.startsWith('/') ? iconPath : (basePath + '/' + iconPath));

    if (item.isImage())
    {
      item.protocols = mediaProfiles().listProtocols(item.imageSize);
    }
    else
    {
      int addVideo = 0;
      SAudioFormat audioFormat = audioFormatFor(peer, item, addVideo);
      if (!item.isMusic() && (audioFormat.numChannels() > 2) &&
          ((item.audioFormat.channelSetup() == SAudioFormat::Channels_Mono) ||
           (item.audioFormat.channelSetup() == SAudioFormat::Channels_Stereo))
          )
      {
        audioFormat.setChannelSetup(SAudioFormat::Channels_Stereo);
      }

      SVideoFormat videoFormat = videoFormatFor(peer, item);
      videoFormat.setFrameRate(item.videoFormat.frameRate());

      if ((item.isVideo() && (addVideo >= 0)) || (addVideo > 0))
      {
        if ((item.type == Item::Type_Audio) || (item.type == Item::Type_AudioBook))
          item.type = Item::Type_Video;
        else if (item.type == Item::Type_Music)
          item.type = Item::Type_MusicVideo;
        else if (item.type == Item::Type_AudioBroadcast)
          item.type = Item::Type_VideoBroadcast;

        item.protocols = mediaProfiles().listProtocols(audioFormat, videoFormat);
      }
      else
      {
        if ((item.type == Item::Type_Video) || (item.type == Item::Type_Movie))
          item.type = Item::Type_Audio;
        else if (item.type == Item::Type_MusicVideo)
          item.type = Item::Type_Music;
        else if (item.type == Item::Type_VideoBroadcast)
          item.type = Item::Type_AudioBroadcast;

        item.protocols = mediaProfiles().listProtocols(audioFormat);
      }
    }

    setQueryItemsFor(peer, item.url);

    result += item;
  }

  return result;
}

SAudioFormat MediaServer::audioFormatFor(const QString &peer, const Item &item, int &addVideo)
{
  GlobalSettings settings;
  settings.beginGroup("DLNA");

  const QString genericTranscodeChannels =
      settings.value("TranscodeChannels", settings.defaultTranscodeChannelName()).toString();
  const QString genericTranscodeMusicChannels =
      settings.value("TranscodeMusicChannels", settings.defaultTranscodeMusicChannelName()).toString();
  const QString genericMusicMode =
      settings.value("MusicMode", settings.defaultMusicModeName()).toString();

  SAudioFormat result;
  result.setSampleRate(48000);

  foreach (const QString &group, settings.childGroups() << QString::null)
  if (group.isEmpty() || group == ("Client_" + peer))
  {
    if (!group.isEmpty())
      settings.beginGroup(group);

    if ((item.type == Item::Type_Music) || (item.type == Item::Type_MusicVideo))
    {
      const QString transcodeMusicChannels = settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString();
      foreach (const GlobalSettings::TranscodeChannel &channel, GlobalSettings::allTranscodeChannels())
      if (channel.name == transcodeMusicChannels)
      {
        result.setChannelSetup(channel.channels);
        break;
      }

      const QString musicMode = settings.value("MusicMode", genericMusicMode).toString();
      if (musicMode.compare("AddVideoBlack", Qt::CaseInsensitive) == 0)
        addVideo = 1;
      else if (musicMode.compare("RemoveVideo", Qt::CaseInsensitive) == 0)
        addVideo = -1;
      else
        addVideo = 0;
    }
    else
    {
      const QString transcodeChannels = settings.value("TranscodeChannels", genericTranscodeChannels).toString();
      foreach (const GlobalSettings::TranscodeChannel &channel, GlobalSettings::allTranscodeChannels())
      if (channel.name == transcodeChannels)
      {
        result.setChannelSetup(channel.channels);
        break;
      }
    }

    break;
  }

  return result;
}

SVideoFormat MediaServer::videoFormatFor(const QString &peer, const Item &)
{
  GlobalSettings settings;
  settings.beginGroup("DLNA");

  const QString genericTranscodeSize =
      settings.value("TranscodeSize", settings.defaultTranscodeSizeName()).toString();

  SVideoFormat result;

  foreach (const QString &group, settings.childGroups() << QString::null)
  if (group.isEmpty() || group == ("Client_" + peer))
  {
    if (!group.isEmpty())
      settings.beginGroup(group);

    const QString transcodeSize = settings.value("TranscodeSize", genericTranscodeSize).toString();
    foreach (const GlobalSettings::TranscodeSize &size, GlobalSettings::allTranscodeSizes())
    if (size.name == transcodeSize)
    {
      result.setSize(size.size);
      break;
    }

    break;
  }

  return result;
}

void MediaServer::setQueryItemsFor(const QString &peer, QUrl &url)
{
  GlobalSettings settings;
  settings.beginGroup("DLNA");

  const QString genericTranscodeSize =
      settings.value("TranscodeSize", settings.defaultTranscodeSizeName()).toString();
  const QString genericTranscodeCrop =
      settings.value("TranscodeCrop", settings.defaultTranscodeCropName()).toString();
  const QString genericEncodeMode =
      settings.value("EncodeMode", settings.defaultEncodeModeName()).toString();
  const QString genericTranscodeChannels =
      settings.value("TranscodeChannels", settings.defaultTranscodeChannelName()).toString();
  const QString genericTranscodeMusicChannels =
      settings.value("TranscodeMusicChannels", settings.defaultTranscodeMusicChannelName()).toString();
  const QString genericMusicMode =
      settings.value("MusicMode", settings.defaultMusicModeName()).toString();

  foreach (const QString &group, settings.childGroups() << QString::null)
  if (group.isEmpty() || group == ("Client_" + peer))
  {
    if (!group.isEmpty())
      settings.beginGroup(group);

    QMap<QString, QString> queryItems;

    const QString transcodeSize = settings.value("TranscodeSize", genericTranscodeSize).toString();
    const QString transcodeCrop = settings.value("TranscodeCrop", genericTranscodeCrop).toString();
    foreach (const GlobalSettings::TranscodeSize &size, GlobalSettings::allTranscodeSizes())
    if (size.name == transcodeSize)
    {
      QString sizeStr =
          QString::number(size.size.width()) + 'x' +
          QString::number(size.size.height()) + 'x' +
          QString::number(size.size.aspectRatio(), 'f', 3);

      if (!transcodeCrop.isEmpty())
        sizeStr += ',' + transcodeCrop.toLower();

      url.addQueryItem("resolution", sizeStr);
      break;
    }

    QString channels = QString::number(SAudioFormat::Channels_Stereo, 16);
    const QString transcodeChannels = settings.value("TranscodeChannels", genericTranscodeChannels).toString();
    foreach (const GlobalSettings::TranscodeChannel &channel, GlobalSettings::allTranscodeChannels())
    if (channel.name == transcodeChannels)
    {
      channels = QString::number(channel.channels, 16);
      break;
    }

    const QString transcodeMusicChannels = settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString();
    foreach (const GlobalSettings::TranscodeChannel &channel, GlobalSettings::allTranscodeChannels())
    if (channel.name == transcodeMusicChannels)
    {
      channels += "," + QString::number(channel.channels, 16);
      break;
    }

    url.addQueryItem("channels", channels);
    url.addQueryItem("priority", "high");

    const QString encodeMode = settings.value("EncodeMode", genericEncodeMode).toString();
    if (!encodeMode.isEmpty())
      url.addQueryItem("encodemode", encodeMode.toLower());
    else
      url.addQueryItem("encodemode", "fast");

    const QString musicMode = settings.value("MusicMode", genericMusicMode).toString();
    if (!musicMode.isEmpty())
      url.addQueryItem("musicmode", musicMode.toLower());
    else
      url.addQueryItem("musicmode", "leavevideo");

    break;
  }
}

void MediaServer::addStream(Stream *stream)
{
  connect(&stream->proxy, SIGNAL(disconnected()), SLOT(cleanStreams()), Qt::QueuedConnection);

  d->streams += stream;
}

void MediaServer::removeStream(Stream *stream)
{
  d->streams.removeAll(stream);
  d->reusableStreams.removeAll(stream);
}


MediaServer::Item::Item(void)
{
}

MediaServer::Item::~Item()
{
}


MediaServer::File::File(const SHttpServer::RequestMessage &request)
{
  const QUrl url(request.path());

  QByteArray query;
  if (SUPnPContentDirectory::fromQueryPath(request.file(), d.fileName, query))
  {
    const QString path = url.path();
    d.url = path.left(path.lastIndexOf('/') + 1) + d.fileName + '?' + query;
  }
  else
  {
    d.fileName = request.file();
    d.url = url;
  }

  QString path = url.path();
  path = path.left(path.lastIndexOf('/'));
  d.parentDir = path.mid(path.lastIndexOf('/') + 1);
}

MediaServer::File::~File()
{
}

MediaServer::Stream::Stream(MediaServer *parent, const QString &url)
  : parent(parent),
    url(url),
    proxy()
{
  parent->addStream(this);
}

MediaServer::Stream::~Stream()
{
  parent->removeStream(this);
}


MediaServer::ThumbnailListItem::ThumbnailListItem(void)
  : played(false)
{
}

MediaServer::ThumbnailListItem::~ThumbnailListItem()
{
}


MediaServer::DetailedListItem::DetailedListItem(void)
  : played(false)
{
}

MediaServer::DetailedListItem::~DetailedListItem(void)
{
}


MediaServer::DetailedListItem::Column::Column(QString title, QUrl iconurl, QUrl url)
  : title(title),
    iconurl(iconurl),
    url(url)
{
}

MediaServer::DetailedListItem::Column::~Column()
{
}

} // End of namespace
