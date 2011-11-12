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

#include "mediaprofiles.h"

namespace LXiMediaCenter {

struct MediaProfiles::Data
{
  QSettings * settings;
  QMap<QString, QString> bestClientCache;

  QMultiMap<int, AudioProfile>  audioProfiles;
  QMultiMap<int, VideoProfile>  videoProfiles;
  QMultiMap<int, ImageProfile>  imageProfiles;

  static const int hiddenPriority = 128;
  static const int priorityBoost = 32;
  static QMap<QByteArray, AudioProfile> audioProfileNames;
  static QMap<QByteArray, VideoProfile> videoProfileNames;
  static QMap<QByteArray, ImageProfile> imageProfileNames;
};

QMap<QByteArray, MediaProfiles::AudioProfile> MediaProfiles::Data::audioProfileNames;
QMap<QByteArray, MediaProfiles::VideoProfile> MediaProfiles::Data::videoProfileNames;
QMap<QByteArray, MediaProfiles::ImageProfile> MediaProfiles::Data::imageProfileNames;

MediaProfiles::MediaProfiles(void)
  : d(new Data())
{
  d->settings = NULL;

#define INSERT_PROFILE_NAME(m, n) m.insert(#n, n)

  if (d->audioProfileNames.isEmpty())
  {
    INSERT_PROFILE_NAME(d->audioProfileNames, LPCM);
    INSERT_PROFILE_NAME(d->audioProfileNames, MP2);
    INSERT_PROFILE_NAME(d->audioProfileNames, MP3);
    INSERT_PROFILE_NAME(d->audioProfileNames, AAC_ADTS);
    INSERT_PROFILE_NAME(d->audioProfileNames, AC3);
    INSERT_PROFILE_NAME(d->audioProfileNames, WMABASE);
    INSERT_PROFILE_NAME(d->audioProfileNames, VORBIS);
  }

  if (d->videoProfileNames.isEmpty())
  {
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG1);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_PS_PAL);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_PS_PAL_XAC3);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_PS_NTSC);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_PS_NTSC_XAC3);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_TS_SD_EU);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_TS_SD_EU_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_TS_HD_EU);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_TS_HD_EU_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_TS_SD_NA);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_TS_SD_NA_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_TS_HD_NA);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_TS_HD_NA_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_SP_AAC);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_SP_AAC_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_SP_MPEG1_L3);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_SP_MPEG1_L3_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_SP_MPEG2_L2);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_SP_MPEG2_L2_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_SP_AC3_L3);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_SP_AC3_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_ASP_AAC);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_ASP_AAC_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_ASP_MPEG1_L3);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_ASP_MPEG1_L3_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_ASP_AC3_L3);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_TS_ASP_AC3_ISO);
    INSERT_PROFILE_NAME(d->videoProfileNames, WMVMED_BASE);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_PS_SD_EU_NONSTD);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_PS_HD_EU_NONSTD);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_PS_SD_NA_NONSTD);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG_PS_HD_NA_NONSTD);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_MATROSKA_MP3_SD_NONSTD);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_MATROSKA_MP3_HD_NONSTD);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_MATROSKA_AAC_SD_NONSTD);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_MATROSKA_AAC_HD_NONSTD);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_MATROSKA_AC3_SD_NONSTD);
    INSERT_PROFILE_NAME(d->videoProfileNames, MPEG4_P2_MATROSKA_AC3_HD_NONSTD);
  }

  if (d->imageProfileNames.isEmpty())
  {
    INSERT_PROFILE_NAME(d->imageProfileNames, JPEG_TN);
    INSERT_PROFILE_NAME(d->imageProfileNames, JPEG_SM);
    INSERT_PROFILE_NAME(d->imageProfileNames, JPEG_MED);
    INSERT_PROFILE_NAME(d->imageProfileNames, JPEG_LRG);
    INSERT_PROFILE_NAME(d->imageProfileNames, PNG_TN);
    INSERT_PROFILE_NAME(d->imageProfileNames, PNG_LRG);
  }

#undef INSERT_PROFILE_NAME
}

MediaProfiles::~MediaProfiles()
{
  delete d->settings;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void MediaProfiles::openDeviceConfig(const QString &filename)
{
  delete d->settings;
  d->settings = new QSettings(filename, QSettings::IniFormat);
  d->bestClientCache.clear();
}

void MediaProfiles::setCodecs(const QSet<QString> &audioCodecs, const QSet<QString> &videoCodecs, const QSet<QString> &imageCodecs, const QSet<QString> &formats)
{
  foreach (AudioProfile profile, d->audioProfileNames)
  if (audioCodecs.contains(audioCodecFor(profile, SAudioFormat::Channels_Stereo)) &&
      audioCodecs.contains(audioCodecFor(profile, SAudioFormat::Channels_Surround_5_1)) &&
      formats.contains(formatFor(profile)))
  {
    d->audioProfiles.insert(profilePriority(profile), profile);
  }

  foreach (VideoProfile profile, d->videoProfileNames)
  if (audioCodecs.contains(audioCodecFor(profile, SAudioFormat::Channels_Stereo)) &&
      audioCodecs.contains(audioCodecFor(profile, SAudioFormat::Channels_Surround_5_1)) &&
      videoCodecs.contains(videoCodecFor(profile)) &&
      formats.contains(formatFor(profile)))
  {
    d->videoProfiles.insert(profilePriority(profile), profile);
  }

  foreach (ImageProfile profile, d->imageProfileNames)
  if (imageCodecs.contains(imageCodecFor(profile)))
    d->imageProfiles.insert(profilePriority(profile), profile);
}

QStringList MediaProfiles::enabledAudioProfiles(void)
{
  QStringList result;
  foreach (AudioProfile profile, d->audioProfiles)
    result += profileName(profile);

  return result;
}

QStringList MediaProfiles::enabledVideoProfiles(void)
{
  QStringList result;
  foreach (VideoProfile profile, d->videoProfiles)
    result += profileName(profile);

  return result;
}

QStringList MediaProfiles::enabledImageProfiles(void)
{
  QStringList result;
  foreach (ImageProfile profile, d->imageProfiles)
    result += profileName(profile);

  return result;
}

SUPnPBase::ProtocolList MediaProfiles::listProtocols(const QString &client)
{
  QMultiMap<int, SUPnPBase::Protocol> result;

  const QStringList supportedAudioProfiles = this->supportedAudioProfiles(client);
  for (QMultiMap<int, AudioProfile>::Iterator i = d->audioProfiles.begin();
       i != d->audioProfiles.end();
       i++)
  {
    const QByteArray name = profileName(i.value());
    if (supportedAudioProfiles.isEmpty() || supportedAudioProfiles.contains(name))
    {
      result.insert(i.key(), SUPnPBase::Protocol(
          "http-get",
          mimeTypeFor(i.value()),
          true, false, true,
          name,
          suffixFor(i.value())));
    }
  }

  const QStringList supportedVideoProfiles = this->supportedVideoProfiles(client);
  for (QMultiMap<int, VideoProfile>::Iterator i = d->videoProfiles.begin();
       i != d->videoProfiles.end();
       i++)
  {
    const QByteArray name = profileName(i.value());
    if (supportedVideoProfiles.isEmpty() || supportedVideoProfiles.contains(name))
    {
      result.insert(i.key(), SUPnPBase::Protocol(
          "http-get",
          mimeTypeFor(i.value()),
          true, false, true,
          name,
          suffixFor(i.value())));
    }
  }

  const QStringList supportedImageProfiles = this->supportedImageProfiles(client);
  for (QMultiMap<int, ImageProfile>::Iterator i = d->imageProfiles.begin();
       i != d->imageProfiles.end();
       i++)
  {
    const QByteArray name = profileName(i.value());
    if (supportedImageProfiles.isEmpty() || supportedImageProfiles.contains(name))
    {
      result.insert(i.key(), SUPnPBase::Protocol(
          "http-get",
          mimeTypeFor(i.value()),
          true, false, false,
          name,
          suffixFor(i.value())));
    }
  }

  return result.values();
}

SUPnPBase::ProtocolList MediaProfiles::listProtocols(const QString &client, const SAudioFormat &audioFormat, bool seekable)
{
  QMultiMap<int, SUPnPBase::Protocol> result;

  const QStringList supportedAudioProfiles = this->supportedAudioProfiles(client);
  for (QMultiMap<int, AudioProfile>::Iterator i = d->audioProfiles.begin();
       i != d->audioProfiles.end();
       i++)
  {
    const QByteArray name = profileName(i.value());
    if (supportedAudioProfiles.isEmpty() || supportedAudioProfiles.contains(name))
    {
      SAudioFormat correctedAudioFormat = audioFormat;
      const int priority = i.key() + correctFormat(*i,correctedAudioFormat);

      result.insert(priority, SUPnPBase::Protocol(
          "http-get",
          mimeTypeFor(i.value()),
          true, false, seekable,
          name,
          suffixFor(i.value()),
          audioFormat.sampleRate(), audioFormat.numChannels()));
    }
  }

  return result.values();
}

SUPnPBase::ProtocolList MediaProfiles::listProtocols(const QString &client, const SAudioFormat &audioFormat, const SVideoFormat &videoFormat, bool seekable)
{
  QMultiMap<int, SUPnPBase::Protocol> result;

  const QStringList supportedVideoProfiles = this->supportedVideoProfiles(client);
  for (QMultiMap<int, VideoProfile>::Iterator i = d->videoProfiles.begin();
       i != d->videoProfiles.end();
       i++)
  {
    const QByteArray name = profileName(i.value());
    if (supportedVideoProfiles.isEmpty() || supportedVideoProfiles.contains(name))
    {
      SAudioFormat correctedAudioFormat = audioFormat;
      SVideoFormat correctedVideoFormat = videoFormat;
      const int priority =
          i.key() +
          correctFormat(*i, correctedAudioFormat) +
          correctFormat(*i, correctedVideoFormat);

      if (priority < d->hiddenPriority)
      {
        result.insert(priority, SUPnPBase::Protocol(
            "http-get",
            mimeTypeFor(i.value()),
            true, false, seekable,
            name,
            suffixFor(i.value()),
            correctedAudioFormat.sampleRate(), correctedAudioFormat.numChannels(),
            correctedVideoFormat.size().size()));
      }
    }
  }

  return result.values();
}

SUPnPBase::ProtocolList MediaProfiles::listProtocols(const QString &client, const SSize &imageSize)
{
  QMultiMap<int, SUPnPBase::Protocol> result;

  const QStringList supportedImageProfiles = this->supportedImageProfiles(client);
  for (QMultiMap<int, ImageProfile>::Iterator i = d->imageProfiles.begin();
       i != d->imageProfiles.end();
       i++)
  {
    const QByteArray name = profileName(i.value());
    if (supportedImageProfiles.isEmpty() || supportedImageProfiles.contains(name))
    {
      SSize correctedImageSize = imageSize;
      const int priority = i.key() + correctFormat(*i, correctedImageSize);

      const quint64 estimatedSize =
          1024 + // Estimated header
          ((quint64(correctedImageSize.absoluteWidth()) *
            quint64(correctedImageSize.absoluteHeight())) /
           (name.startsWith("JPEG") ? 2 : 1));

      if (priority < d->hiddenPriority)
      {
        result.insert(priority, SUPnPBase::Protocol(
            "http-get",
            mimeTypeFor(i.value()),
            true, false, false,
            name,
            suffixFor(i.value()),
            0, 0,
            correctedImageSize.size(),
            estimatedSize));
      }
    }
  }

  return result.values();
}

QStringList MediaProfiles::supportedAudioProfiles(const QString &client)
{
  const QString bestClient = findBestClient(client);

  QStringList result;
  if (d->settings && !bestClient.isEmpty())
  {
    d->settings->beginGroup(bestClient);
    result = d->settings->value("AudioProfiles", result).toStringList();
    d->settings->endGroup();
  }

  GlobalSettings settings;
  settings.beginGroup("DLNA");
  settings.beginGroup("Client_" + SStringParser::toCleanName(client).replace(' ', '_'));
  return settings.value("SupportedAudioProfiles", result).toStringList();
}

QStringList MediaProfiles::supportedVideoProfiles(const QString &client)
{
  const QString bestClient = findBestClient(client);

  QStringList result;
  if (d->settings && !bestClient.isEmpty())
  {
    d->settings->beginGroup(bestClient);
    result = d->settings->value("VideoProfiles", result).toStringList();
    d->settings->endGroup();
  }

  GlobalSettings settings;
  settings.beginGroup("DLNA");
  settings.beginGroup("Client_" + SStringParser::toCleanName(client).replace(' ', '_'));
  return settings.value("SupportedVideoProfiles", result).toStringList();
}

QStringList MediaProfiles::supportedImageProfiles(const QString &client)
{
  const QString bestClient = findBestClient(client);

  QStringList result;
  if (d->settings && !bestClient.isEmpty())
  {
    d->settings->beginGroup(bestClient);
    result = d->settings->value("ImageProfiles", result).toStringList();
    d->settings->endGroup();
  }

  GlobalSettings settings;
  settings.beginGroup("DLNA");
  settings.beginGroup("Client_" + SStringParser::toCleanName(client).replace(' ', '_'));
  return settings.value("SupportedImageProfiles", result).toStringList();
}

QString MediaProfiles::findBestClient(const QString &client)
{
  QString result;
  if (!client.isEmpty())
  {
    QMap<QString, QString>::Iterator i = d->bestClientCache.find(client);
    if (i != d->bestClientCache.end())
      return *i;

    QString clientUserAgent, clientVersion;
    {
      const int at = client.indexOf('@');
      clientUserAgent = (at >= 0) ? client.left(at) : client;

      const int sl = clientUserAgent.lastIndexOf('/');
      if (sl > 0)
      {
        clientVersion = clientUserAgent.mid(sl + 1).toUpper();
        clientUserAgent = clientUserAgent.left(sl);
      }
    }

    QString bestVersion;
    if (d->settings && !clientUserAgent.isEmpty())
    foreach (const QString &groupUserAgent, d->settings->childGroups())
    {
      if (clientUserAgent.compare(groupUserAgent, Qt::CaseInsensitive) == 0)
      {
        d->settings->beginGroup(groupUserAgent);

        foreach (const QString &groupVersion, d->settings->childGroups())
        {
          if (bestVersion.isEmpty() ||
              ((clientVersion >= groupVersion) && (groupVersion > bestVersion)) ||
              ((clientVersion <= groupVersion) && (groupVersion < bestVersion)))
          {
            bestVersion = groupVersion;
          }
        }

        if (!bestVersion.isEmpty())
          result = groupUserAgent + '/' + bestVersion;
        else
          result = groupUserAgent;

        d->settings->endGroup();
      }
    }

    d->bestClientCache.insert(client, result);
  }

  return result;
}

int MediaProfiles::correctFormat(AudioProfile profile, SAudioFormat &format)
{
  switch (profile)
  {
  case LPCM:
    format.setSampleRate(48000);
    if ((format.numChannels() == 1) || (format.numChannels() == 2))
    {
      format.setChannelSetup(SAudioFormat::Channels_Stereo);
      return -1;
    }
    else
    {
      format.setChannelSetup(SAudioFormat::Channels_Stereo);
      return 1;
    }

  case MP2:
  case MP3:
  case AAC_ADTS:
  case WMABASE:
    format.setSampleRate(44100);
    return correctFormatStereo(format);

  case VORBIS:
    format.setSampleRate(48000);
    return correctFormatStereo(format);

  case AC3:
    format.setSampleRate(48000);
    return correctFormatSurround51(format);
  }

  return 0;
}

int MediaProfiles::correctFormat(VideoProfile profile, SAudioFormat &format)
{
  switch (profile)
  {
  case MPEG1:
  case MPEG_PS_PAL:
  case MPEG_PS_NTSC:
  case MPEG4_P2_TS_SP_AAC:
  case MPEG4_P2_TS_SP_AAC_ISO:
  case MPEG4_P2_TS_SP_MPEG1_L3:
  case MPEG4_P2_TS_SP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_SP_MPEG2_L2:
  case MPEG4_P2_TS_SP_MPEG2_L2_ISO:
  case MPEG4_P2_TS_ASP_AAC:
  case MPEG4_P2_TS_ASP_AAC_ISO:
  case MPEG4_P2_TS_ASP_MPEG1_L3:
  case MPEG4_P2_TS_ASP_MPEG1_L3_ISO:
  case WMVMED_BASE:
  case MPEG4_P2_MATROSKA_MP3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_SD_NONSTD:
    format.setSampleRate(44100);
    return correctFormatStereo(format);

  case MPEG4_P2_MATROSKA_MP3_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_HD_NONSTD:
    format.setSampleRate(48000);
    return correctFormatStereo(format);

  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC_XAC3:
  case MPEG_TS_SD_EU:
  case MPEG_TS_SD_EU_ISO:
  case MPEG_TS_SD_NA:
  case MPEG_TS_SD_NA_ISO:
  case MPEG4_P2_TS_SP_AC3_L3:
  case MPEG4_P2_TS_SP_AC3_ISO:
  case MPEG4_P2_TS_ASP_AC3_L3:
  case MPEG4_P2_TS_ASP_AC3_ISO:
  case MPEG_PS_SD_EU_NONSTD:
  case MPEG_PS_SD_NA_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_SD_NONSTD:
    format.setSampleRate(44100);
    return correctFormatSurround51(format);

  case MPEG_TS_HD_EU:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_TS_HD_NA:
  case MPEG_TS_HD_NA_ISO:
  case MPEG_PS_HD_EU_NONSTD:
  case MPEG_PS_HD_NA_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_HD_NONSTD:
    format.setSampleRate(48000);
    return correctFormatSurround51(format);
  }

  return 0;
}

int MediaProfiles::correctFormat(VideoProfile profile, SVideoFormat &format)
{
  const double frameRate = format.frameRate().toFrequency();

  int offset = 0;
  switch (profile)
  {
  case MPEG1:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset -= Data::priorityBoost;
    else if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      offset += Data::priorityBoost;

    if (frameRate < 24.5)
    {
      format.setFrameRate(SInterval::ntscFrequency(24));
      format.setSize(format.size().scaled(352, 240));
    }
    else if (frameRate > 27.5)
    {
      format.setFrameRate(SInterval::ntscFrequency(30));
      format.setSize(format.size().scaled(352, 240));
    }
    else
    {
      format.setFrameRate(SInterval::fromFrequency(25));
      format.setSize(format.size().scaled(352, 288));
    }

    break;

  case MPEG_PS_PAL:
  case MPEG_PS_PAL_XAC3:
  case MPEG_TS_SD_EU:
  case MPEG_TS_SD_EU_ISO:
  case MPEG_PS_SD_EU_NONSTD:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = Data::hiddenPriority * 2;
    else if ((format.size().width() <= 768) && (format.size().height() <= 576))
      offset -= Data::priorityBoost;

    if (frameRate > 27.5)
      offset = Data::hiddenPriority * 2;

    format.setFrameRate(SInterval::fromFrequency(25));
    format.setSize(format.size().scaled(720, 576));
    break;

  case MPEG_PS_NTSC:
  case MPEG_PS_NTSC_XAC3:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = Data::hiddenPriority * 2;
    else if ((format.size().width() <= 768) && (format.size().height() <= 576))
      offset -= Data::priorityBoost;

    if (frameRate < 27.5)
      offset = Data::hiddenPriority * 2;

    format.setFrameRate(SInterval::ntscFrequency(30));
    format.setSize(format.size().scaled(704, 480));
    break;

  case MPEG_TS_SD_NA:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_PS_SD_NA_NONSTD:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = Data::hiddenPriority * 2;
    else if ((format.size().width() <= 768) && (format.size().height() <= 576))
      offset -= Data::priorityBoost;

    if (frameRate < 27.5)
      format.setFrameRate(SInterval::ntscFrequency(24));
    else
      format.setFrameRate(SInterval::ntscFrequency(30));

    format.setSize(format.size().scaled(704, 480));

    break;

  case MPEG_TS_HD_EU:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_PS_HD_EU_NONSTD:
    if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      offset -= Data::priorityBoost;
    else
      offset = Data::hiddenPriority * 2;

    if (frameRate < 24.5)
      format.setFrameRate(SInterval::fromFrequency(24));
    else if (frameRate > 27.5)
      format.setFrameRate(SInterval::fromFrequency(30));
    else
      format.setFrameRate(SInterval::fromFrequency(25));

    if ((format.size().width() >= 1920) || (format.size().height() >= 1080))
      format.setSize(SSize(1920, 1080));
    else if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      format.setSize(SSize(1280, 720));

    break;

  case MPEG_TS_HD_NA:
  case MPEG_TS_HD_NA_ISO:
  case MPEG_PS_HD_NA_NONSTD:
    if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      offset -= Data::priorityBoost;
    else
      offset = Data::hiddenPriority * 2;

    if (frameRate < 27.5)
      format.setFrameRate(SInterval::ntscFrequency(24));
    else
      format.setFrameRate(SInterval::ntscFrequency(30));

    if ((format.size().width() >= 1920) || (format.size().height() >= 1080))
      format.setSize(SSize(1920, 1080));
    else if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      format.setSize(SSize(1280, 720));

    break;

  case MPEG4_P2_TS_SP_AAC:
  case MPEG4_P2_TS_SP_AAC_ISO:
  case MPEG4_P2_TS_SP_MPEG2_L2:
  case MPEG4_P2_TS_SP_MPEG2_L2_ISO:
  case MPEG4_P2_TS_SP_AC3_ISO:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset -= Data::priorityBoost;
    else if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      offset += Data::priorityBoost;

    if ((float(format.size().absoluteWidth()) / float(format.size().absoluteHeight())) >= 1.6f)
      format.setSize(format.size().scaled(320, 180)); // QVGA 16:9
    else
      format.setSize(format.size().scaled(320, 240)); // QVGA 4:3

    format.setFrameRate(SInterval::fromFrequency(15));

    break;

  case MPEG4_P2_TS_SP_MPEG1_L3:
  case MPEG4_P2_TS_SP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_SP_AC3_L3:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = Data::hiddenPriority * 2;
    else if ((format.size().width() <= 720) && (format.size().height() <= 576))
      offset -= Data::priorityBoost;

    if ((float(format.size().absoluteWidth()) / float(format.size().absoluteHeight())) >= 1.6f)
      format.setSize(format.size().scaled(640, 360)); // VGA 16:9
    else
      format.setSize(format.size().scaled(640, 480)); // VGA

    break;

  case MPEG4_P2_TS_ASP_AAC:
  case MPEG4_P2_TS_ASP_AAC_ISO:
  case MPEG4_P2_TS_ASP_MPEG1_L3:
  case MPEG4_P2_TS_ASP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_ASP_AC3_L3:
  case MPEG4_P2_TS_ASP_AC3_ISO:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = Data::hiddenPriority * 2;
    else if ((format.size().width() <= 720) && (format.size().height() <= 576))
      offset -= Data::priorityBoost;

    format.setSize(format.size().scaled(720, 576));

    break;

  case WMVMED_BASE:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = Data::hiddenPriority * 2;
    else if ((format.size().width() <= 720) && (format.size().height() <= 576))
      offset -= Data::priorityBoost;

    if (frameRate < 27.5)
    {
      format.setFrameRate(SInterval::fromFrequency(25));
      format.setSize(format.size().scaled(720, 576));
    }
    else
    {
      format.setFrameRate(SInterval::ntscFrequency(30));
      format.setSize(format.size().scaled(720, 480));
    }

    break;

  case MPEG4_P2_MATROSKA_MP3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_SD_NONSTD:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = Data::hiddenPriority * 2;
    else if ((format.size().width() <= 768) && (format.size().height() <= 576))
      offset -= Data::priorityBoost;

    if ((float(format.size().absoluteWidth()) / float(format.size().absoluteHeight())) >= 1.6f)
      format.setSize(format.size().scaled(640, 360)); // VGA 16:9
    else
      format.setSize(format.size().scaled(640, 480)); // VGA

    break;

  case MPEG4_P2_MATROSKA_MP3_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_HD_NONSTD:
    if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      offset -= Data::priorityBoost;
    else
      offset = Data::hiddenPriority * 2;

    if ((format.size().width() >= 1920) || (format.size().height() >= 1080))
      format.setSize(SSize(1920, 1080));
    else if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      format.setSize(SSize(1280, 720));

    break;
  }

  return offset;
}

int MediaProfiles::correctFormat(ImageProfile profile, SSize &size)
{
  int offset = 0;
  switch (profile)
  {
  case JPEG_TN:
    if ((size.width() <= 160) && (size.height() <= 160))
      offset--; // Prefer

    size = SSize(160, 160);
    break;

  case JPEG_SM:
    if ((size.width() > 160) || (size.height() > 160))
    if ((size.width() <= 640) && (size.height() <= 480))
      offset--; // Prefer

    size = SSize(640, 480);
    break;    

  case JPEG_MED:
    if ((size.width() > 640) || (size.height() > 480))
    if ((size.width() <= 1024) && (size.height() <= 768))
      offset--; // Prefer

    size = SSize(1024, 768);
    break;

  case JPEG_LRG:
    if ((size.width() > 1024) || (size.height() > 768))
      offset--; // Prefer

    size = SSize(1920, 1080);
    break;

  case PNG_TN:
    if ((size.width() <= 160) && (size.height() <= 160))
      offset--; // Prefer

    size = SSize(160, 160);
    break;

  case PNG_LRG:
    if ((size.width() > 160) || (size.height() > 160))
      offset--; // Prefer

    size = SSize(1920, 1080);
    break;
  }

  return offset;
}

#define PROFILE_NAME(x) if (profileName == #x) return x

MediaProfiles::AudioProfile MediaProfiles::audioProfileFor(const QString &contentFeatures) const
{
  const int profilePos = contentFeatures.indexOf("DLNA.ORG_PN=");
  if (profilePos >= 0)
  {
    const int end = contentFeatures.indexOf(';', profilePos);
    const QString profileName = contentFeatures.mid(profilePos + 12, qMax(-1, end - (profilePos + 12)));

    QMap<QByteArray, AudioProfile>::ConstIterator i = d->audioProfileNames.find(profileName.toAscii());
    if (i != d->audioProfileNames.end())
      return i.value();

    Q_ASSERT_X(false, "Could not find", profileName.toAscii().data());
  }

  return AudioProfile(0);
}

MediaProfiles::VideoProfile MediaProfiles::videoProfileFor(const QString &contentFeatures) const
{
  const int profilePos = contentFeatures.indexOf("DLNA.ORG_PN=");
  if (profilePos >= 0)
  {
    const int end = contentFeatures.indexOf(';', profilePos);
    const QString profileName = contentFeatures.mid(profilePos + 12, qMax(-1, end - (profilePos + 12)));

    QMap<QByteArray, VideoProfile>::ConstIterator i = d->videoProfileNames.find(profileName.toAscii());
    if (i != d->videoProfileNames.end())
      return i.value();

    Q_ASSERT_X(false, "Could not find", profileName.toAscii().data());
  }

  return VideoProfile(0);
}

MediaProfiles::ImageProfile MediaProfiles::imageProfileFor(const QString &contentFeatures) const
{
  const int profilePos = contentFeatures.indexOf("DLNA.ORG_PN=");
  if (profilePos >= 0)
  {
    const int end = contentFeatures.indexOf(';', profilePos);
    const QString profileName = contentFeatures.mid(profilePos + 12, qMax(-1, end - (profilePos + 12)));

    QMap<QByteArray, ImageProfile>::ConstIterator i = d->imageProfileNames.find(profileName.toAscii());
    if (i != d->imageProfileNames.end())
      return i.value();

    Q_ASSERT_X(false, "Could not find", profileName.toAscii().data());
  }

  return ImageProfile(0);
}

QString MediaProfiles::audioCodecFor(AudioProfile profile, SAudioFormat::Channels)
{
  switch (profile)
  {
  case LPCM:
    return "PCM/S16BE";

  case MP2:
    return "MP2";

  case MP3:
    return "MP3";

  case AAC_ADTS:
    return "AAC";

  case AC3:
    return "AC3";

  case WMABASE:
    return "WMAV2";

  case VORBIS:
    return "VORBIS";
  }

  return QString::null;
}

QString MediaProfiles::audioCodecFor(VideoProfile profile, SAudioFormat::Channels channels)
{
  switch (profile)
  {
  case MPEG1:
  case MPEG_PS_PAL:
  case MPEG_PS_NTSC:
  case MPEG4_P2_TS_SP_MPEG2_L2:
  case MPEG4_P2_TS_SP_MPEG2_L2_ISO:
    return "MP2";

  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC_XAC3:
  case MPEG4_P2_TS_SP_AC3_L3:
  case MPEG4_P2_TS_SP_AC3_ISO:
  case MPEG4_P2_TS_ASP_AC3_L3:
  case MPEG4_P2_TS_ASP_AC3_ISO:
  case MPEG4_P2_MATROSKA_AC3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_HD_NONSTD:
    return "AC3";

  case MPEG_TS_SD_EU:
  case MPEG_TS_SD_EU_ISO:
  case MPEG_PS_SD_EU_NONSTD:
  case MPEG_TS_HD_EU:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_PS_HD_EU_NONSTD:
    if ((channels == SAudioFormat::Channels_Mono) || (channels == SAudioFormat::Channels_Stereo))
      return "MP2";
    else
      return "AC3";

  case MPEG_TS_SD_NA:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_PS_SD_NA_NONSTD:
  case MPEG_TS_HD_NA:
  case MPEG_TS_HD_NA_ISO:
  case MPEG_PS_HD_NA_NONSTD:
    return "AC3";

  case MPEG4_P2_TS_SP_AAC:
  case MPEG4_P2_TS_SP_AAC_ISO:
  case MPEG4_P2_TS_ASP_AAC:
  case MPEG4_P2_TS_ASP_AAC_ISO:
  case MPEG4_P2_MATROSKA_AAC_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_HD_NONSTD:
    return "AAC";

  case MPEG4_P2_TS_SP_MPEG1_L3:
  case MPEG4_P2_TS_SP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_ASP_MPEG1_L3:
  case MPEG4_P2_TS_ASP_MPEG1_L3_ISO:
    return "MP3";

  case WMVMED_BASE:
    return "WMAV2";

  case MPEG4_P2_MATROSKA_MP3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_MP3_HD_NONSTD:
    return "MP3";
  }

  return QString::null;
}

QString MediaProfiles::videoCodecFor(VideoProfile profile)
{
  switch (profile)
  {
  case MPEG1:
    return "MPEG1";

  case MPEG_PS_PAL:
  case MPEG_PS_NTSC:
  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC_XAC3:
  case MPEG_TS_SD_EU:
  case MPEG_TS_SD_EU_ISO:
  case MPEG_PS_SD_EU_NONSTD:
  case MPEG_TS_HD_EU:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_PS_HD_EU_NONSTD:
  case MPEG_TS_SD_NA:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_PS_SD_NA_NONSTD:
  case MPEG_TS_HD_NA:
  case MPEG_TS_HD_NA_ISO:
  case MPEG_PS_HD_NA_NONSTD:
    return "MPEG2";

  case MPEG4_P2_TS_SP_AAC:
  case MPEG4_P2_TS_SP_AAC_ISO:
  case MPEG4_P2_TS_SP_MPEG1_L3:
  case MPEG4_P2_TS_SP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_SP_MPEG2_L2:
  case MPEG4_P2_TS_SP_MPEG2_L2_ISO:
  case MPEG4_P2_TS_SP_AC3_L3:
  case MPEG4_P2_TS_SP_AC3_ISO:
  case MPEG4_P2_TS_ASP_AAC:
  case MPEG4_P2_TS_ASP_AAC_ISO:
  case MPEG4_P2_TS_ASP_MPEG1_L3:
  case MPEG4_P2_TS_ASP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_ASP_AC3_L3:
  case MPEG4_P2_TS_ASP_AC3_ISO:
  case MPEG4_P2_MATROSKA_MP3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_MP3_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_HD_NONSTD:
    return "MPEG4";

  case WMVMED_BASE:
    return "WMV3";
  }

  return QString::null;
}

QString MediaProfiles::imageCodecFor(ImageProfile profile)
{
  switch (profile)
  {
  case JPEG_TN:
  case JPEG_SM:
  case JPEG_MED:
  case JPEG_LRG:
    return "JPEG";

  case PNG_TN:
  case PNG_LRG:
    return "PNG";
  }

  return QString::null;
}

QString MediaProfiles::formatFor(AudioProfile profile)
{
  switch (profile)
  {
  case LPCM:
    return "s16be";

  case MP2:
    return "mp2";

  case MP3:
    return "mp3";

  case AAC_ADTS:
    return "adts";

  case AC3:
    return "ac3";

  case WMABASE:
    return "asf";

  case VORBIS:
    return "ogg";
  }

  return QString::null;
}

QString MediaProfiles::formatFor(VideoProfile profile)
{
  switch (profile)
  {
  case MPEG1:
    return "mpeg";

  case MPEG_PS_PAL:
  case MPEG_PS_NTSC:
  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC_XAC3:
  case MPEG_PS_SD_EU_NONSTD:
  case MPEG_PS_HD_EU_NONSTD:
  case MPEG_PS_SD_NA_NONSTD:
  case MPEG_PS_HD_NA_NONSTD:
    return "vob";

  case MPEG_TS_SD_EU:
  case MPEG_TS_HD_EU:
  case MPEG_TS_SD_NA:
  case MPEG_TS_HD_NA:
    return "m2ts";

  case MPEG_TS_SD_EU_ISO:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_TS_HD_NA_ISO:
    return "mpegts";

  case MPEG4_P2_TS_SP_AAC:
  case MPEG4_P2_TS_SP_MPEG1_L3:
  case MPEG4_P2_TS_SP_MPEG2_L2:
  case MPEG4_P2_TS_SP_AC3_L3:
  case MPEG4_P2_TS_ASP_AAC:
  case MPEG4_P2_TS_ASP_MPEG1_L3:
  case MPEG4_P2_TS_ASP_AC3_L3:
    return "m2ts";

  case MPEG4_P2_TS_SP_AAC_ISO:
  case MPEG4_P2_TS_SP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_SP_MPEG2_L2_ISO:
  case MPEG4_P2_TS_SP_AC3_ISO:
  case MPEG4_P2_TS_ASP_AAC_ISO:
  case MPEG4_P2_TS_ASP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_ASP_AC3_ISO:
    return "mpegts";

  case WMVMED_BASE:
    return "asf";

  case MPEG4_P2_MATROSKA_MP3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_MP3_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_HD_NONSTD:
    return "matroska";
  }

  return QString::null;
}

const char * MediaProfiles::mimeTypeFor(AudioProfile profile)
{
  switch (profile)
  {
  case LPCM:
    return "audio/L16;rate=48000;channels=2";

  case MP2:
  case MP3:
    return SHttpEngine::mimeAudioMpeg;

  case AAC_ADTS:
    return SHttpEngine::mimeAudioAac; //"audio/vnd.dlna.adts";

  case AC3:
    return SHttpEngine::mimeAudioAc3;

  case WMABASE:
    return SHttpEngine::mimeAudioWma;

  case VORBIS:
    return SHttpEngine::mimeAudioOgg;
  }

  return "";
}

const char * MediaProfiles::mimeTypeFor(VideoProfile profile)
{
  switch (profile)
  {
  case MPEG1:
  case MPEG_PS_PAL:
  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC:
  case MPEG_PS_NTSC_XAC3:
  case MPEG_PS_SD_EU_NONSTD:
  case MPEG_PS_HD_EU_NONSTD:
  case MPEG_PS_SD_NA_NONSTD:
  case MPEG_PS_HD_NA_NONSTD:
    return SHttpEngine::mimeVideoMpeg;

  case MPEG_TS_SD_EU_ISO:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_TS_HD_NA_ISO:
  case MPEG4_P2_TS_SP_AAC_ISO:
  case MPEG4_P2_TS_SP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_SP_MPEG2_L2_ISO:
  case MPEG4_P2_TS_SP_AC3_ISO:
  case MPEG4_P2_TS_ASP_AAC_ISO:
  case MPEG4_P2_TS_ASP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_ASP_AC3_ISO:
    return SHttpEngine::mimeVideoMpegTS;

  case MPEG_TS_SD_EU:
  case MPEG_TS_HD_EU:
  case MPEG_TS_SD_NA:
  case MPEG_TS_HD_NA:
    return SHttpEngine::mimeVideoMpegM2TS;

  case MPEG4_P2_TS_SP_AAC:
  case MPEG4_P2_TS_SP_MPEG1_L3:
  case MPEG4_P2_TS_SP_MPEG2_L2:
  case MPEG4_P2_TS_SP_AC3_L3:
  case MPEG4_P2_TS_ASP_AAC:
  case MPEG4_P2_TS_ASP_MPEG1_L3:
  case MPEG4_P2_TS_ASP_AC3_L3:
    return SHttpEngine::mimeVideoMpegM2TS;

  case WMVMED_BASE:
    return SHttpEngine::mimeVideoWmv;

  case MPEG4_P2_MATROSKA_MP3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_MP3_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_HD_NONSTD:
    return SHttpEngine::mimeVideoMatroska;
  }

  return "";
}

const char * MediaProfiles::mimeTypeFor(ImageProfile profile)
{
  switch (profile)
  {
  case JPEG_TN:
  case JPEG_SM:
  case JPEG_MED:
  case JPEG_LRG:
    return SHttpEngine::mimeImageJpeg;

  case PNG_TN:
  case PNG_LRG:
    return SHttpEngine::mimeImagePng;
  }

  return "";
}

const char * MediaProfiles::suffixFor(AudioProfile profile)
{
  switch (profile)
  {
  case LPCM:
    return ".lpcm";

  case MP2:
    return ".mp2";

  case MP3:
    return ".mp3";

  case AAC_ADTS:
    return ".aac";

  case AC3:
    return ".ac3";

  case WMABASE:
    return ".wma";

  case VORBIS:
    return ".oga";
  }

  return "";
}

const char * MediaProfiles::suffixFor(VideoProfile profile)
{
  switch (profile)
  {
  case MPEG1:
  case MPEG_PS_PAL:
  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC:
  case MPEG_PS_NTSC_XAC3:
  case MPEG_PS_SD_EU_NONSTD:
  case MPEG_PS_HD_EU_NONSTD:
  case MPEG_PS_SD_NA_NONSTD:
  case MPEG_PS_HD_NA_NONSTD:
    return ".mpeg";

  case MPEG_TS_SD_EU:
  case MPEG_TS_HD_EU:
  case MPEG_TS_SD_NA:
  case MPEG_TS_HD_NA:
    return ".m2ts";

  case MPEG_TS_SD_EU_ISO:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_TS_HD_NA_ISO:
  case MPEG4_P2_TS_SP_AAC_ISO:
  case MPEG4_P2_TS_SP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_SP_MPEG2_L2_ISO:
  case MPEG4_P2_TS_SP_AC3_ISO:
  case MPEG4_P2_TS_ASP_AAC_ISO:
  case MPEG4_P2_TS_ASP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_ASP_AC3_ISO:
    return ".ts";

  case MPEG4_P2_TS_SP_AAC:
  case MPEG4_P2_TS_SP_MPEG1_L3:
  case MPEG4_P2_TS_SP_MPEG2_L2:
  case MPEG4_P2_TS_SP_AC3_L3:
  case MPEG4_P2_TS_ASP_AAC:
  case MPEG4_P2_TS_ASP_MPEG1_L3:
  case MPEG4_P2_TS_ASP_AC3_L3:
    return ".m2ts";

  case WMVMED_BASE:
    return ".wmv";

  case MPEG4_P2_MATROSKA_MP3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_MP3_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_HD_NONSTD:
    return ".mkv";
  }

  return "";
}

const char * MediaProfiles::suffixFor(ImageProfile profile)
{
  switch (profile)
  {
  case JPEG_TN:
  case JPEG_SM:
  case JPEG_MED:
  case JPEG_LRG:
    return ".jpeg";

  case PNG_TN:
  case PNG_LRG:
    return ".png";
  }

  return "";
}

int MediaProfiles::profilePriority(AudioProfile profile)
{
  switch (profile)
  {
  case LPCM:
  case AC3:
    return -2;

  case MP2:
  case AAC_ADTS:
    return 0;

  case MP3:
    return -1;

  case WMABASE:
    return 1;

  case VORBIS:
    return 0;
  }

  return 0;
}

int MediaProfiles::profilePriority(VideoProfile profile)
{
  switch (profile)
  {
  case MPEG_TS_HD_EU:
  case MPEG_TS_HD_EU_ISO:
    return -9;

  case MPEG_TS_HD_NA:
  case MPEG_TS_HD_NA_ISO:
    return -6;

  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC_XAC3:
    return -1;

  case MPEG_PS_PAL:
  case MPEG_PS_NTSC:
    return 0;

  case MPEG_TS_SD_EU:
  case MPEG_TS_SD_EU_ISO:
    return 3;

  case MPEG_TS_SD_NA:
  case MPEG_TS_SD_NA_ISO:
    return 6;

  case MPEG4_P2_TS_SP_AAC:
  case MPEG4_P2_TS_SP_AAC_ISO:
  case MPEG4_P2_TS_SP_MPEG1_L3:
  case MPEG4_P2_TS_SP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_SP_MPEG2_L2:
  case MPEG4_P2_TS_SP_MPEG2_L2_ISO:
  case MPEG4_P2_TS_SP_AC3_L3:
  case MPEG4_P2_TS_SP_AC3_ISO:
    return 12;

  case MPEG4_P2_TS_ASP_AAC:
  case MPEG4_P2_TS_ASP_AAC_ISO:
  case MPEG4_P2_TS_ASP_MPEG1_L3:
  case MPEG4_P2_TS_ASP_MPEG1_L3_ISO:
  case MPEG4_P2_TS_ASP_AC3_L3:
  case MPEG4_P2_TS_ASP_AC3_ISO:
    return 15;

  case MPEG1:
    return 18;

  case MPEG_PS_HD_EU_NONSTD:
    return -8;

  case MPEG_PS_HD_NA_NONSTD:
    return -5;

  case MPEG4_P2_MATROSKA_MP3_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_HD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_HD_NONSTD:
    return -4;

  case MPEG_PS_SD_EU_NONSTD:
    return 4;

  case MPEG_PS_SD_NA_NONSTD:
    return 7;

  case MPEG4_P2_MATROSKA_MP3_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AAC_SD_NONSTD:
  case MPEG4_P2_MATROSKA_AC3_SD_NONSTD:
    return 8;

  case WMVMED_BASE:
    return 9;
  }

  return 0;
}

int MediaProfiles::profilePriority(ImageProfile profile)
{
  switch (profile)
  {
  case JPEG_LRG:
    return -3;

  case JPEG_MED:
  case PNG_LRG:
    return -2;

  case JPEG_SM:
    return -1;

  case JPEG_TN:
  case PNG_TN:
    return 0;
  }

  return 0;
}

int MediaProfiles::correctFormatStereo(SAudioFormat &format)
{
  if (format.numChannels() == 1)
  {
    format.setChannelSetup(SAudioFormat::Channels_Mono);
    return -1;
  }
  else if (format.numChannels() == 2)
  {
    format.setChannelSetup(SAudioFormat::Channels_Stereo);
    return -1;
  }
  else
  {
    format.setChannelSetup(SAudioFormat::Channels_Stereo);
    return 1;
  }
}

int MediaProfiles::correctFormatSurround51(SAudioFormat &format)
{
  if ((format.channelSetup() == SAudioFormat::Channels_Mono) ||
      (format.channelSetup() == SAudioFormat::Channels_Stereo) ||
      (format.channelSetup() == SAudioFormat::Channels_Quadraphonic) ||
      (format.channelSetup() == SAudioFormat::Channels_Surround_3_0) ||
      (format.channelSetup() == SAudioFormat::Channels_Surround_4_0) ||
      (format.channelSetup() == SAudioFormat::Channels_Surround_5_0) ||
      (format.channelSetup() == SAudioFormat::Channels_Surround_5_1))
  {
    return 0;
  }
  else if (format.numChannels() == 1)
  {
    format.setChannelSetup(SAudioFormat::Channels_Mono);
    return 1;
  }
  else if (format.numChannels() == 2)
  {
    format.setChannelSetup(SAudioFormat::Channels_Stereo);
    return 1;
  }
  else if (format.channelSetup() & SAudioFormat::Channel_LowFrequencyEffects)
  {
    format.setChannelSetup(SAudioFormat::Channels_Surround_5_0);
    return 1;
  }
  else
  {
    format.setChannelSetup(SAudioFormat::Channels_Surround_5_1);
    return 1;
  }
}

int MediaProfiles::correctFormatSurround71(SAudioFormat &format)
{
  if ((format.channelSetup() == SAudioFormat::Channels_Mono) ||
      (format.channelSetup() == SAudioFormat::Channels_Stereo) ||
      (format.channelSetup() == SAudioFormat::Channels_Quadraphonic) ||
      (format.channelSetup() == SAudioFormat::Channels_Surround_3_0) ||
      (format.channelSetup() == SAudioFormat::Channels_Surround_4_0) ||
      (format.channelSetup() == SAudioFormat::Channels_Surround_5_0) ||
      (format.channelSetup() == SAudioFormat::Channels_Surround_5_1))
  {
    return 1;
  }
  else if ((format.channelSetup() == SAudioFormat::Channels_Surround_6_0) ||
           (format.channelSetup() == SAudioFormat::Channels_Surround_6_1) ||
           (format.channelSetup() == SAudioFormat::Channels_Surround_7_0) ||
           (format.channelSetup() == SAudioFormat::Channels_Surround_7_1))
  {
    return 0;
  }
  else if (format.numChannels() == 1)
  {
    format.setChannelSetup(SAudioFormat::Channels_Mono);
    return 1;
  }
  else if (format.numChannels() == 2)
  {
    format.setChannelSetup(SAudioFormat::Channels_Stereo);
    return 1;
  }
  else if (format.channelSetup() & SAudioFormat::Channel_LowFrequencyEffects)
  {
    if (format.numChannels() <= 5)
    {
      format.setChannelSetup(SAudioFormat::Channels_Surround_5_0);
      return 1;
    }
    else
    {
      format.setChannelSetup(SAudioFormat::Channels_Surround_7_0);
      return 1;
    }
  }
  else
  {
    if (format.numChannels() <= 6)
    {
      format.setChannelSetup(SAudioFormat::Channels_Surround_5_1);
      return 1;
    }
    else
    {
      format.setChannelSetup(SAudioFormat::Channels_Surround_7_1);
      return 1;
    }
  }
}

QByteArray MediaProfiles::profileName(AudioProfile profile)
{
  for (QMap<QByteArray, AudioProfile>::ConstIterator i = Data::audioProfileNames.begin();
       i != Data::audioProfileNames.end();
       i++)
  if (i.value() == profile)
  {
    return i.key();
  }

  Q_ASSERT(false);
  return QByteArray();
}

QByteArray MediaProfiles::profileName(VideoProfile profile)
{
  for (QMap<QByteArray, VideoProfile>::ConstIterator i = Data::videoProfileNames.begin();
       i != Data::videoProfileNames.end();
       i++)
  if (i.value() == profile)
  {
    return i.key();
  }

  Q_ASSERT(false);
  return QByteArray();
}

QByteArray MediaProfiles::profileName(ImageProfile profile)
{
  for (QMap<QByteArray, ImageProfile>::ConstIterator i = Data::imageProfileNames.begin();
       i != Data::imageProfileNames.end();
       i++)
  if (i.value() == profile)
  {
    return i.key();
  }

  Q_ASSERT(false);
  return QByteArray();
}

} // End of namespace
