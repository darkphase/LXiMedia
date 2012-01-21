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

QByteArray MediaServer::makeThumbnail(QSize size, const QImage &image, const QString &overlay)
{
  if (size.isNull())
    size = QSize(128, 128);

  QImage result(size, QImage::Format_ARGB32);
  QPainter p;
  p.begin(&result);

  if (!image.isNull())
  {
    const QImage baseImage = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha
    p.fillRect(result.rect(), Qt::transparent);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha
    p.drawImage(
        (result.width() / 2) - (baseImage.width() / 2),
        (result.height() / 2) - (baseImage.height() / 2),
        baseImage);
  }
  else
  {
    p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha
    p.fillRect(result.rect(), Qt::transparent);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha
  }

  if (!overlay.isEmpty())
  {
    QImage overlayImage(":/img/" + overlay + ".png");
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

  QBuffer buffer;
  buffer.open(QBuffer::WriteOnly);
  result.save(&buffer, "PNG");
  buffer.close();

  return buffer.data();
}

QSet<QString> & MediaServer::fileProtocols(void)
{
  static QSet<QString> p;

  return p;
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

QList<MediaServer::TranscodeSize> MediaServer::allTranscodeSizes(void)
{
  QList<TranscodeSize> sizes;
  sizes << TranscodeSize("QVGA",            SSize(320,  240,  1.0f))
        << TranscodeSize("Video CD 4:3",    SSize(352,  288,  1.0f))
        << TranscodeSize("Video CD 16:9",   SSize(352,  288,  1.45455f))
        << TranscodeSize("DVD/NTSC 4:3",    SSize(640,  480,  1.0f))
        << TranscodeSize("DVD/NTSC 16:9",   SSize(704,  480,  1.21307f))
        << TranscodeSize("DVD/PAL 4:3",     SSize(720,  576,  1.06666f))
        << TranscodeSize("DVD/PAL 16:9",    SSize(720,  576,  1.42222f))
        << TranscodeSize("HDTV 720p 16:9",  SSize(1280, 720,  1.0f))
        << TranscodeSize("HDTV 720p 21:9",  SSize(1280, 544,  1.0f))
        << TranscodeSize("HDTV 1080p 16:9", SSize(1920, 1080, 1.0f))
        << TranscodeSize("HDTV 1080p 21:9", SSize(1920, 832,  1.0f));

  return sizes;
}

QString MediaServer::defaultTranscodeSizeName(void)
{
  // Use DVD for compatibility, as the HD profiles get hairy.
  return "DVD/PAL 16:9";
}

QString MediaServer::defaultTranscodeCropName(void)
{
  return "Box";
}

QString MediaServer::defaultEncodeModeName(void)
{
  if (QThread::idealThreadCount() > 3)
    return "Slow";
  else
    return "Fast";
}

QList<MediaServer::TranscodeChannel> MediaServer::allTranscodeChannels(void)
{
  QList<TranscodeChannel> channels;
  channels << TranscodeChannel("2.0 Stereo",        SAudioFormat::Channels_Stereo)
           << TranscodeChannel("3.0 Surround",      SAudioFormat::Channels_Surround_3_0)
           << TranscodeChannel("4.0 Quadraphonic",  SAudioFormat::Channels_Quadraphonic)
           << TranscodeChannel("5.0 Surround",      SAudioFormat::Channels_Surround_5_0)
           << TranscodeChannel("5.1 Surround",      SAudioFormat::Channels_Surround_5_1)
           << TranscodeChannel("7.1 Surround",      SAudioFormat::Channels_Surround_7_1);

  return channels;
}

QString MediaServer::defaultTranscodeChannelName(void)
{
  return "2.0 Stereo";
}

QString MediaServer::defaultTranscodeMusicChannelName(void)
{
  return "2.0 Stereo";
}

bool MediaServer::defaultMusicAddBlackVideo(void)
{
  return false;
}

QList<MediaServer::SubtitleSize> MediaServer::allSubtitleSizes(void)
{
  QList<SubtitleSize> sizes;
  sizes << SubtitleSize("Tiny",     0.025)
        << SubtitleSize("Small",    0.04)
        << SubtitleSize("Normal",   0.05)
        << SubtitleSize("Large",    0.0625)
        << SubtitleSize("Huge",     0.08);

  return sizes;
}

QString MediaServer::defaultSubtitleSizeName(void)
{
  return "Normal";
}

int MediaServer::loadItemCount(void)
{
  return qBound(1, QThread::idealThreadCount(), 16) * 4;
}

MediaServer::ListType MediaServer::listType(const QString &)
{
  return ListType_Thumbnails;
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
    if (request.file().endsWith('/'))
    {
      if (request.url().hasQueryItem("items"))
      {
        const QStringList range = request.url().queryItemValue("items").split(',');
        int start = 0, count = 0;
        if (!range.isEmpty())
        {
          start = range[0].toInt();
          if (range.count() >= 2)
            count = range[1].toInt();
        }

        QMap<int, QString> funcs;
        if (request.url().hasQueryItem("func"))
        {
          const QStringList list = request.url().queryItemValue("func").split(',');
          for (int i=0; i+1<list.count(); i+=2)
            funcs.insert(list[i].toInt(), list[i+1]);
        }

        ThumbnailListItemList thumbItems;
        foreach (const SUPnPContentDirectory::Item &item, listItems(request.url().path(), start, count))
        {
          if (funcs.isEmpty() && item.isDir)
          {
            ThumbnailListItem thumbItem;
            thumbItem.title = item.title;
            thumbItem.iconurl = item.iconUrl;
            thumbItem.url = item.path;

            thumbItems.append(thumbItem);
          }
          else if (funcs.isEmpty() || funcs.contains(item.type / 10))
          {
            ThumbnailListItem thumbItem;
            thumbItem.title = item.title;

            if (!item.artist.isEmpty())
              thumbItem.text += tr("Artist") + ": " + item.artist;

            if (!item.album.isEmpty())
              thumbItem.text += tr("Album") + ": " + item.album;

            if (item.track > SMediaInfo::tvShowSeason)
            {
              thumbItem.text +=
                  tr("Track") + ": " +
                  QString::number(item.track / SMediaInfo::tvShowSeason) + 'x' +
                  ('0' + QString::number(item.track % SMediaInfo::tvShowSeason)).right(2);
            }
            else if (item.track > 0)
              thumbItem.text += tr("Track") + ": " + QString::number(item.track);

            if (item.duration > 0)
              thumbItem.text += tr("Duration") + ": " + QTime(0, 0, 0).addSecs(item.duration).toString("h:mm:ss");

            thumbItem.iconurl = item.iconUrl;
            thumbItem.played = item.played;
            thumbItem.url = item.url;
            if (!thumbItem.url.isEmpty())
              thumbItem.url.addQueryItem("player", QString::number(item.type));

            if (item.played)
            {
              thumbItem.iconurl.removeQueryItem("overlay");
              thumbItem.iconurl.addQueryItem("overlay", "played");
            }

            if (!funcs.isEmpty())
              thumbItem.func = funcs[item.type / 10];

            thumbItems.append(thumbItem);
          }
        }

        return makeResponse(request, buildListItems(thumbItems), SHttpEngine::mimeTextHtml, false);
      }
      else
        return makeHtmlContent(request, buildListLoader(request.file(), listType(request.file())), htmlListHead);
    }
    else if (request.url().hasQueryItem("player"))
    {
      const SUPnPContentDirectory::Item::Type playerType =
          SUPnPContentDirectory::Item::Type(request.url().queryItemValue("player").toInt());

      switch (playerType)
      {
      case SUPnPContentDirectory::Item::Type_None:
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);

      case SUPnPContentDirectory::Item::Type_Audio:
      case SUPnPContentDirectory::Item::Type_Music:
      case SUPnPContentDirectory::Item::Type_AudioBroadcast:
      case SUPnPContentDirectory::Item::Type_AudioBook:
        return buildAudioPlayer(request);

      case SUPnPContentDirectory::Item::Type_Video:
      case SUPnPContentDirectory::Item::Type_Movie:
      case SUPnPContentDirectory::Item::Type_VideoBroadcast:
      case SUPnPContentDirectory::Item::Type_MusicVideo:
      case SUPnPContentDirectory::Item::Type_Image:
      case SUPnPContentDirectory::Item::Type_Photo:
        return buildPlayer(request);
      }
    }
    else if (request.url().hasQueryItem("audioplayer"))
    {
      SStringParser htmlParser;
      htmlParser.setField("SOURCES", "");

      const Item item = getItem(request.file());
      htmlParser.setField("TITLE", item.title);
      if (!item.artist.isEmpty())
        htmlParser.appendField("TITLE", " [" + item.artist + "]");
      if (item.duration > 0)
        htmlParser.appendField("TITLE", " (" + QTime(0, 0, 0).addSecs(item.duration).toString("m:ss") + ")");

      if (mediaProfiles().isProfileEnabled(MediaProfiles::VORBIS_NONSTD) ||
          mediaProfiles().isProfileEnabled(MediaProfiles::FLAC_NONSTD))
      {
        QUrl url(request.file());
        url.addQueryItem("format", "oga");

        htmlParser.setField("SOURCE_URL", url);
        htmlParser.setField("SOURCE_MIME", SHttpEngine::mimeAudioOgg);
        htmlParser.appendField("SOURCES", htmlParser.parse(htmlAudioPlayerSource));
      }

      if (mediaProfiles().isProfileEnabled(MediaProfiles::MP3))
      {
        QUrl url(request.file());
        url.addQueryItem("format", "mp3");

        htmlParser.setField("SOURCE_URL", url);
        htmlParser.setField("SOURCE_MIME", SHttpEngine::mimeAudioMp3);
        htmlParser.appendField("SOURCES", htmlParser.parse(htmlAudioPlayerSource));
      }

      if (mediaProfiles().isProfileEnabled(MediaProfiles::MP2))
      {
        QUrl url(request.file());
        url.addQueryItem("format", "mp2");

        htmlParser.setField("SOURCE_URL", url);
        htmlParser.setField("SOURCE_MIME", SHttpEngine::mimeAudioMpeg);
        htmlParser.appendField("SOURCES", htmlParser.parse(htmlAudioPlayerSource));
      }

      if (mediaProfiles().isProfileEnabled(MediaProfiles::WAV_NONSTD))
      {
        QUrl url(request.file());
        url.addQueryItem("format", "wav");

        htmlParser.setField("SOURCE_URL", url);
        htmlParser.setField("SOURCE_MIME", SHttpEngine::mimeAudioWave);
        htmlParser.appendField("SOURCES", htmlParser.parse(htmlAudioPlayerSource));
      }

      return makeResponse(request, htmlParser.parse(htmlAudioPlayerElement), SHttpEngine::mimeTextHtml, false);
    }
    else if (request.url().hasQueryItem("videoplayer"))
    {
      SStringParser htmlParser;
      htmlParser.setField("SOURCES", "");

      if (mediaProfiles().isProfileEnabled(MediaProfiles::OGG_THEORA_VORBIS_SD_NONSTD) ||
          mediaProfiles().isProfileEnabled(MediaProfiles::OGG_THEORA_FLAC_SD_NONSTD))
      {
        QUrl url(request.file());
        url.addQueryItem("format", "ogv");

        htmlParser.setField("SOURCE_URL", url);
        htmlParser.setField("SOURCE_MIME", SHttpEngine::mimeVideoOgg);
        htmlParser.appendField("SOURCES", htmlParser.parse(htmlVideoPlayerSource));
      }

      if (mediaProfiles().isProfileEnabled(MediaProfiles::MPEG_PS_SD_EU_NONSTD) ||
          mediaProfiles().isProfileEnabled(MediaProfiles::MPEG_PS_SD_NA_NONSTD))
      {
        QUrl url(request.file());
        url.addQueryItem("format", "vob");

        htmlParser.setField("SOURCE_URL", url);
        htmlParser.setField("SOURCE_MIME", SHttpEngine::mimeVideoMpeg);
        htmlParser.appendField("SOURCES", htmlParser.parse(htmlVideoPlayerSource));
      }

      if (mediaProfiles().isProfileEnabled(MediaProfiles::MPEG4_P2_MATROSKA_MP3_SD_NONSTD) ||
          mediaProfiles().isProfileEnabled(MediaProfiles::MPEG4_P2_MATROSKA_AAC_SD_NONSTD) ||
          mediaProfiles().isProfileEnabled(MediaProfiles::MPEG4_P2_MATROSKA_AC3_SD_NONSTD))
      {
        QUrl url(request.file());
        url.addQueryItem("format", "matroska");

        htmlParser.setField("SOURCE_URL", url);
        htmlParser.setField("SOURCE_MIME", SHttpEngine::mimeVideoMatroska);
        htmlParser.appendField("SOURCES", htmlParser.parse(htmlVideoPlayerSource));
      }

      return makeResponse(request, htmlParser.parse(htmlVideoPlayerElement), SHttpEngine::mimeTextHtml, false);
    }
    else // Stream file
    {
      const QString contentFeatures = QByteArray::fromBase64(request.url().queryItemValue("contentFeatures").toAscii());

      if (!request.isHead())
      {
        // Check for image
        const MediaProfiles::ImageProfile imageProfile = mediaProfiles().imageProfileFor(contentFeatures);
        const QString format = request.url().queryItemValue("format");
        if ((imageProfile != 0) || (format == "jpeg") || (format == "png"))
        {
          SHttpServer::ResponseMessage response = sendPhoto(request);
          response.setField("transferMode.dlna.org", "Streaming");
          if (!contentFeatures.isEmpty())
            response.setField("contentFeatures.dlna.org", contentFeatures);

          return response;
        }

        // Else Audio/Video stream
        foreach (Stream *stream, d->streams)
        if (stream->url == request.path())
        if (stream->proxy.addSocket(socket, masterServer()->httpServer()))
          return SHttpServer::ResponseMessage(request, SHttpServer::Status_None);

        Stream * const stream = streamVideo(request);
        if (stream)
        {
          stream->proxy.addSocket(socket, masterServer()->httpServer());
          return SHttpServer::ResponseMessage(request, SHttpServer::Status_None);
        }
        else
          return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
      }
      else // Return head only.
      {
        SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
        response.setField("transferMode.dlna.org", "Streaming");
        if (!contentFeatures.isEmpty())
          response.setField("contentFeatures.dlna.org", contentFeatures);

        return response;
      }
    }
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

QList<SUPnPContentDirectory::Item> MediaServer::listContentDirItems(const QString &client, const QString &dirPath, int start, int &count)
{
  if (!activeClients().contains(client))
    activeClients().insert(client);

  QList<SUPnPContentDirectory::Item> result;
  foreach (Item item, listItems(dirPath, start, count))
  {
    processItem(client, item);

    result += item;
  }

  return result;
}

SUPnPContentDirectory::Item MediaServer::getContentDirItem(const QString &client, const QString &path)
{
  if (!activeClients().contains(client))
    activeClients().insert(client);

  Item item = getItem(path);
  if (!item.isNull())
    processItem(client, item);

  return item;
}

SAudioFormat MediaServer::audioFormatFor(const QString &client, const Item &item, bool &addVideo)
{
  QSettings settings;
  settings.beginGroup("DLNA");

  const QString genericTranscodeChannels =
      settings.value("TranscodeChannels", defaultTranscodeChannelName()).toString();
  const QString genericTranscodeMusicChannels =
      settings.value("TranscodeMusicChannels", defaultTranscodeMusicChannelName()).toString();
  const bool genericMusicAddBlackVideo =
      settings.value("MusicAddBlackVideo", defaultMusicAddBlackVideo()).toBool();

  SAudioFormat result;
  result.setSampleRate(48000);

  const QString clientTag = SStringParser::toCleanName(client).replace(' ', '_');

  foreach (const QString &group, settings.childGroups() << QString::null)
  if (group.isEmpty() || group == ("Client_" + clientTag))
  {
    if (!group.isEmpty())
      settings.beginGroup(group);

    if ((item.type == Item::Type_Music) ||
        (item.type == Item::Type_AudioBroadcast) || // For play-all
        (item.type == Item::Type_MusicVideo))
    {
      const QString transcodeMusicChannels = settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString();
      foreach (const TranscodeChannel &channel, allTranscodeChannels())
      if (channel.name == transcodeMusicChannels)
      {
        result.setChannelSetup(channel.channels);
        break;
      }

      addVideo = settings.value("MusicAddBlackVideo", genericMusicAddBlackVideo).toBool();
    }
    else
    {
      const QString transcodeChannels = settings.value("TranscodeChannels", genericTranscodeChannels).toString();
      foreach (const TranscodeChannel &channel, allTranscodeChannels())
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

SVideoFormat MediaServer::videoFormatFor(const QString &client, const Item &)
{
  QSettings settings;
  settings.beginGroup("DLNA");

  const QString genericTranscodeSize =
      settings.value("TranscodeSize", defaultTranscodeSizeName()).toString();

  SVideoFormat result;

  const QString clientTag = SStringParser::toCleanName(client).replace(' ', '_');

  foreach (const QString &group, settings.childGroups() << QString::null)
  if (group.isEmpty() || group == ("Client_" + clientTag))
  {
    if (!group.isEmpty())
      settings.beginGroup(group);

    const QString transcodeSize = settings.value("TranscodeSize", genericTranscodeSize).toString();
    foreach (const TranscodeSize &size, allTranscodeSizes())
    if (size.name == transcodeSize)
    {
      result.setSize(size.size);
      break;
    }

    break;
  }

  return result;
}

void MediaServer::processItem(const QString &client, Item &item)
{
  if (!item.isNull())
  {
    if (item.isImage())
    {
      item.protocols = mediaProfiles().listProtocols(client, item.imageSize);
    }
    else
    {
      bool addVideo = false;
      SAudioFormat audioFormat = audioFormatFor(client, item, addVideo);
      if (!item.isMusic() && (audioFormat.numChannels() > 2) &&
          ((item.audioFormat.channelSetup() == SAudioFormat::Channels_Mono) ||
           (item.audioFormat.channelSetup() == SAudioFormat::Channels_Stereo))
          )
      {
        audioFormat.setChannelSetup(SAudioFormat::Channels_Stereo);
      }

      SVideoFormat videoFormat = videoFormatFor(client, item);
      videoFormat.setFrameRate(item.videoFormat.frameRate());

      if (item.isVideo() || addVideo)
      {
        if ((item.type == Item::Type_Audio) || (item.type == Item::Type_AudioBook))
          item.type = Item::Type_Video;
        else if (item.type == Item::Type_Music)
          item.type = Item::Type_MusicVideo;
        else if (item.type == Item::Type_AudioBroadcast)
          item.type = Item::Type_VideoBroadcast;

        item.protocols = mediaProfiles().listProtocols(client, audioFormat, videoFormat);
      }
      else
      {
        if ((item.type == Item::Type_Video) || (item.type == Item::Type_Movie))
          item.type = Item::Type_Audio;
        else if (item.type == Item::Type_MusicVideo)
          item.type = Item::Type_Music;
        else if (item.type == Item::Type_VideoBroadcast)
          item.type = Item::Type_AudioBroadcast;

        item.protocols = mediaProfiles().listProtocols(client, audioFormat);
      }
    }

    setQueryItemsFor(client, item.url, item.isMusic());
  }
}

void MediaServer::setQueryItemsFor(const QString &client, QUrl &url, bool isMusic)
{
  QSettings settings;
  settings.beginGroup("DLNA");

  const QString genericTranscodeSize =
      settings.value("TranscodeSize", defaultTranscodeSizeName()).toString();
  const QString genericTranscodeCrop =
      settings.value("TranscodeCrop", defaultTranscodeCropName()).toString();
  const QString genericEncodeMode =
      settings.value("EncodeMode", defaultEncodeModeName()).toString();
  const QString genericTranscodeChannels =
      settings.value("TranscodeChannels", defaultTranscodeChannelName()).toString();
  const QString genericTranscodeMusicChannels =
      settings.value("TranscodeMusicChannels", defaultTranscodeMusicChannelName()).toString();
  const bool genericMusicAddBlackVideo =
      settings.value("MusicAddBlackVideo", defaultMusicAddBlackVideo()).toBool();
  const QString genericSubtitleSize =
      settings.value("SubtitleSize", defaultSubtitleSizeName()).toString();

  const QString clientTag = SStringParser::toCleanName(client).replace(' ', '_');

  foreach (const QString &group, settings.childGroups() << QString::null)
  if (group.isEmpty() || group == ("Client_" + clientTag))
  {
    if (!group.isEmpty())
      settings.beginGroup(group);

    const QString transcodeSize = settings.value("TranscodeSize", genericTranscodeSize).toString();
    const QString transcodeCrop = settings.value("TranscodeCrop", genericTranscodeCrop).toString();
    foreach (const TranscodeSize &size, allTranscodeSizes())
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
    if (!isMusic)
    {
      const QString transcodeChannels = settings.value("TranscodeChannels", genericTranscodeChannels).toString();
      foreach (const TranscodeChannel &channel, allTranscodeChannels())
      if (channel.name == transcodeChannels)
      {
        channels = QString::number(channel.channels, 16);
        break;
      }
    }
    else
    {
      const QString transcodeMusicChannels = settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString();
      foreach (const TranscodeChannel &channel, allTranscodeChannels())
      if (channel.name == transcodeMusicChannels)
      {
        channels = QString::number(channel.channels, 16);
        break;
      }
    }

    url.addQueryItem("channels", channels);
    url.addQueryItem("priority", "high");

    const QString encodeMode = settings.value("EncodeMode", genericEncodeMode).toString();
    if (!encodeMode.isEmpty())
      url.addQueryItem("encodemode", encodeMode.toLower());
    else
      url.addQueryItem("encodemode", "fast");

    if (settings.value("MusicAddBlackVideo", genericMusicAddBlackVideo).toBool())
      url.addQueryItem("addvideo", QString::null);

    const QString subtitleSize = settings.value("SubtitleSize", genericSubtitleSize).toString();
    foreach (const SubtitleSize &size, allSubtitleSizes())
    if (size.name == subtitleSize)
    {
      url.addQueryItem("subtitlesize", QString::number(size.ratio, 'f', 3));
      break;
    }

    if (isMusic)
      url.addQueryItem("music", QString::null);

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
