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
  QMultiMap<int, AudioProfile>  audioProfiles;
  QMultiMap<int, VideoProfile>  videoProfiles;
  QMultiMap<int, ImageProfile>  imageProfiles;
};

MediaProfiles::MediaProfiles(void)
  : d(new Data())
{
}

MediaProfiles::~MediaProfiles()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void MediaProfiles::addProfile(AudioProfile profile, int priority)
{
  d->audioProfiles.insert(priority, profile);
}

void MediaProfiles::addProfile(VideoProfile profile, int priority)
{
  d->videoProfiles.insert(priority, profile);
}

void MediaProfiles::addProfile(ImageProfile profile, int priority)
{
  d->imageProfiles.insert(priority, profile);
}

SUPnPBase::ProtocolList MediaProfiles::listProtocols(void)
{
  QMultiMap<int, SUPnPBase::Protocol> result;

  for (QMultiMap<int, AudioProfile>::Iterator i = d->audioProfiles.begin();
       i != d->audioProfiles.end();
       i++)
  {
    result.insert(i.key(), SUPnPBase::Protocol(
        "http-get",
        mimeTypeFor(i.value()),
        true,
        profileName(i.value()),
        suffixFor(i.value())));
  }

  for (QMultiMap<int, VideoProfile>::Iterator i = d->videoProfiles.begin();
       i != d->videoProfiles.end();
       i++)
  {
    result.insert(i.key(), SUPnPBase::Protocol(
        "http-get",
        mimeTypeFor(i.value()),
        true,
        profileName(i.value()),
        suffixFor(i.value())));
  }

  for (QMultiMap<int, ImageProfile>::Iterator i = d->imageProfiles.begin();
       i != d->imageProfiles.end();
       i++)
  {
    result.insert(i.key(), SUPnPBase::Protocol(
        "http-get",
        mimeTypeFor(i.value()),
        true,
        profileName(i.value()),
        suffixFor(i.value())));
  }

  return result.values();
}

SUPnPBase::ProtocolList MediaProfiles::listProtocols(const SAudioFormat &audioFormat)
{
  QMultiMap<int, SUPnPBase::Protocol> result;

  for (QMultiMap<int, AudioProfile>::Iterator i = d->audioProfiles.begin();
       i != d->audioProfiles.end();
       i++)
  {
    SAudioFormat correctedAudioFormat = audioFormat;
    const int priority = i.key() + correctFormat(*i,correctedAudioFormat);

    result.insert(priority, SUPnPBase::Protocol(
        "http-get",
        mimeTypeFor(i.value()),
        true,
        profileName(i.value()),
        suffixFor(i.value())));
  }

  return result.values();
}

SUPnPBase::ProtocolList MediaProfiles::listProtocols(const SAudioFormat &audioFormat, const SVideoFormat &videoFormat)
{
  QMultiMap<int, SUPnPBase::Protocol> result;

  for (QMultiMap<int, VideoProfile>::Iterator i = d->videoProfiles.begin();
       i != d->videoProfiles.end();
       i++)
  {
    SAudioFormat correctedAudioFormat = audioFormat;
    SVideoFormat correctedVideoFormat = videoFormat;
    const int priority =
        i.key() +
        correctFormat(*i, correctedAudioFormat) +
        correctFormat(*i, correctedVideoFormat);

    if (priority < 128)
    {
      result.insert(priority, SUPnPBase::Protocol(
          "http-get",
          mimeTypeFor(i.value()),
          true,
          profileName(i.value()),
          suffixFor(i.value())));
    }
  }

  return result.values();
}

SUPnPBase::ProtocolList MediaProfiles::listProtocols(const SSize &imageSize)
{
  QMultiMap<int, SUPnPBase::Protocol> result;

  for (QMultiMap<int, ImageProfile>::Iterator i = d->imageProfiles.begin();
       i != d->imageProfiles.end();
       i++)
  {
    SSize correctedImageSize = imageSize;
    const int priority = i.key() + correctFormat(*i, correctedImageSize);

    if (priority < 128)
    {
      result.insert(priority, SUPnPBase::Protocol(
          "http-get",
          mimeTypeFor(i.value()),
          true,
          profileName(i.value()),
          suffixFor(i.value())));
    }
  }

  return result.values();
}

int MediaProfiles::correctFormat(AudioProfile profile, SAudioFormat &format)
{
  switch (profile)
  {
  case AC3:
    format.setSampleRate(48000);
    return correctFormatSurround51(format);

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
  case WMABASE:
  case VORBIS:
    format.setSampleRate(48000);
    return correctFormatStereo(format);
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
    format.setSampleRate(48000);
    return correctFormatStereo(format);

  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC_XAC3:
  case MPEG_PS_SD_EU:
  case MPEG_TS_SD_EU:
  case MPEG_TS_SD_EU_ISO:
  case MPEG_PS_HD_EU:
  case MPEG_TS_HD_EU:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_PS_SD_NA:
  case MPEG_TS_SD_NA:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_PS_HD_NA:
  case MPEG_TS_HD_NA:
  case MPEG_TS_HD_NA_ISO:
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
      offset -= 2; // Prefer

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
  case MPEG_PS_SD_EU:
  case MPEG_TS_SD_EU:
  case MPEG_TS_SD_EU_ISO:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = 256; // Hide
    else if ((format.size().width() <= 720) && (format.size().height() <= 576))
      offset -= 2; // Prefer

    if (frameRate < 27.5)
      offset -= 2; // Prefer

    format.setFrameRate(SInterval::fromFrequency(25));
    format.setSize(format.size().scaled(720, 576));
    break;

  case MPEG_PS_NTSC:
  case MPEG_PS_NTSC_XAC3:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = 256; // Hide
    else if ((format.size().width() <= 720) && (format.size().height() <= 576))
      offset -= 2; // Prefer

    if (frameRate >= 27.5)
      offset -= 2; // Prefer

    format.setFrameRate(SInterval::ntscFrequency(30));
    format.setSize(format.size().scaled(704, 480));
    break;

  case MPEG_PS_SD_NA:
  case MPEG_TS_SD_NA:
  case MPEG_TS_SD_NA_ISO:
    if ((format.size().width() <= 352) && (format.size().height() <= 288))
      offset = 256; // Hide
    else if ((format.size().width() <= 720) && (format.size().height() <= 576))
      offset -= 2; // Prefer

    if ((frameRate >= 27.5) || (frameRate < 24.5))
      offset -= 2; // Prefer

    if (frameRate < 27.5)
      format.setFrameRate(SInterval::ntscFrequency(24));
    else
      format.setFrameRate(SInterval::ntscFrequency(30));

    format.setSize(format.size().scaled(704, 480));

    break;

  case MPEG_PS_HD_EU:
  case MPEG_TS_HD_EU:
  case MPEG_TS_HD_EU_ISO:
    if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      offset -= 4; // Prefer
    else
      offset = 256; // Hide

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

  case MPEG_PS_HD_NA:
  case MPEG_TS_HD_NA:
  case MPEG_TS_HD_NA_ISO:
    if ((format.size().width() >= 1280) || (format.size().height() >= 720))
      offset -= 4; // Prefer
    else
      offset = 256; // Hide

    if (frameRate < 27.5)
      format.setFrameRate(SInterval::ntscFrequency(24));
    else
      format.setFrameRate(SInterval::ntscFrequency(30));

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

MediaProfiles::AudioProfile MediaProfiles::audioProfileFor(const QString &contentFeatures)
{
  const int profilePos = contentFeatures.indexOf("DLNA.ORG_PN=");
  if (profilePos >= 0)
  {
    const int end = contentFeatures.indexOf(';', profilePos);
    const QString profileName = contentFeatures.mid(profilePos + 12, qMax(-1, end - (profilePos + 12)));

         PROFILE_NAME(AC3);
    else PROFILE_NAME(LPCM);
    else PROFILE_NAME(MP2);
    else PROFILE_NAME(MP3);
    else PROFILE_NAME(WMABASE);
    else PROFILE_NAME(VORBIS);
  }

  return AudioProfile(0);
}

MediaProfiles::VideoProfile MediaProfiles::videoProfileFor(const QString &contentFeatures)
{
  const int profilePos = contentFeatures.indexOf("DLNA.ORG_PN=");
  if (profilePos >= 0)
  {
    const int end = contentFeatures.indexOf(';', profilePos);
    const QString profileName = contentFeatures.mid(profilePos + 12, qMax(-1, end - (profilePos + 12)));

         PROFILE_NAME(MPEG1);
    else PROFILE_NAME(MPEG_PS_PAL);
    else PROFILE_NAME(MPEG_PS_PAL_XAC3);
    else PROFILE_NAME(MPEG_PS_NTSC);
    else PROFILE_NAME(MPEG_PS_NTSC_XAC3);
    else PROFILE_NAME(MPEG_PS_SD_EU);
    else PROFILE_NAME(MPEG_TS_SD_EU);
    else PROFILE_NAME(MPEG_TS_SD_EU_ISO);
    else PROFILE_NAME(MPEG_PS_HD_EU);
    else PROFILE_NAME(MPEG_TS_HD_EU);
    else PROFILE_NAME(MPEG_TS_HD_EU_ISO);
    else PROFILE_NAME(MPEG_PS_SD_NA);
    else PROFILE_NAME(MPEG_TS_SD_NA);
    else PROFILE_NAME(MPEG_TS_SD_NA_ISO);
    else PROFILE_NAME(MPEG_PS_HD_NA);
    else PROFILE_NAME(MPEG_TS_HD_NA);
    else PROFILE_NAME(MPEG_TS_HD_NA_ISO);
  }

  return VideoProfile(0);
}

MediaProfiles::ImageProfile MediaProfiles::imageProfileFor(const QString &contentFeatures)
{
  const int profilePos = contentFeatures.indexOf("DLNA.ORG_PN=");
  if (profilePos >= 0)
  {
    const int end = contentFeatures.indexOf(';', profilePos);
    const QString profileName = contentFeatures.mid(profilePos + 12, qMax(-1, end - (profilePos + 12)));

         PROFILE_NAME(JPEG_TN);
    else PROFILE_NAME(JPEG_SM);
    else PROFILE_NAME(JPEG_MED);
    else PROFILE_NAME(JPEG_LRG);
    else PROFILE_NAME(PNG_TN);
    else PROFILE_NAME(PNG_LRG);
  }

  return ImageProfile(0);
}

#undef PROFILE_NAME

SAudioCodec MediaProfiles::audioCodecFor(AudioProfile profile, const SAudioFormat &format)
{
  switch (profile)
  {
  case AC3:
    return SAudioCodec("AC3", format.channelSetup(), format.sampleRate());

  case LPCM:
    return SAudioCodec("PCM/S16BE", format.channelSetup(), format.sampleRate());

  case MP2:
    return SAudioCodec("MP2", format.channelSetup(), format.sampleRate());

  case MP3:
    return SAudioCodec("MP3", format.channelSetup(), format.sampleRate());

  case WMABASE:
    return SAudioCodec("WMA", format.channelSetup(), format.sampleRate());

  case VORBIS:
    return SAudioCodec("VORBIS", format.channelSetup(), format.sampleRate());
  }

  return SAudioCodec();
}

SAudioCodec MediaProfiles::audioCodecFor(VideoProfile profile, const SAudioFormat &format)
{
  switch (profile)
  {
  case MPEG1:
  case MPEG_PS_PAL:
  case MPEG_PS_NTSC:
    return SAudioCodec("MP2", format.channelSetup(), format.sampleRate());

  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC_XAC3:
    return SAudioCodec("AC3", format.channelSetup(), format.sampleRate());

  case MPEG_PS_SD_EU:
  case MPEG_TS_SD_EU:
  case MPEG_TS_SD_EU_ISO:
  case MPEG_PS_HD_EU:
  case MPEG_TS_HD_EU:
  case MPEG_TS_HD_EU_ISO:
    if ((format.channelSetup() == SAudioFormat::Channels_Mono) ||
        (format.channelSetup() == SAudioFormat::Channels_Stereo))
    {
      return SAudioCodec("MP2", format.channelSetup(), format.sampleRate());
    }
    else
      return SAudioCodec("AC3", format.channelSetup(), format.sampleRate());

  case MPEG_PS_SD_NA:
  case MPEG_TS_SD_NA:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_PS_HD_NA:
  case MPEG_TS_HD_NA:
  case MPEG_TS_HD_NA_ISO:
    return SAudioCodec("AC3", format.channelSetup(), format.sampleRate());
  }

  return SAudioCodec();
}

SVideoCodec MediaProfiles::videoCodecFor(VideoProfile profile, const SVideoFormat &format)
{
  switch (profile)
  {
  case MPEG1:
    return SVideoCodec("MPEG1", format.size(), format.frameRate());

  case MPEG_PS_PAL:
  case MPEG_PS_NTSC:
  case MPEG_PS_PAL_XAC3:
  case MPEG_PS_NTSC_XAC3:
  case MPEG_PS_SD_EU:
  case MPEG_TS_SD_EU:
  case MPEG_TS_SD_EU_ISO:
  case MPEG_PS_HD_EU:
  case MPEG_TS_HD_EU:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_PS_SD_NA:
  case MPEG_TS_SD_NA:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_PS_HD_NA:
  case MPEG_TS_HD_NA:
  case MPEG_TS_HD_NA_ISO:
    return SVideoCodec("MPEG2", format.size(), format.frameRate());
  }

  return SVideoCodec();
}

QString MediaProfiles::formatFor(AudioProfile profile)
{
  switch (profile)
  {
  case AC3:
    return "ac3";

  case LPCM:
    return "s16be";

  case MP2:
    return "mp2";

  case MP3:
    return "mp3";

  case WMABASE:
    return "asf";

  case VORBIS:
    return "ogg";
  }

  return "";
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
  case MPEG_PS_SD_EU:
  case MPEG_PS_HD_EU:
  case MPEG_PS_SD_NA:
  case MPEG_PS_HD_NA:
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
  }

  return "";
}

const char * MediaProfiles::mimeTypeFor(AudioProfile profile)
{
  switch (profile)
  {
  case AC3:
    return "audio/ac3";

  case LPCM:
    return "audio/L16;rate=48000;channels=2";

  case MP2:
  case MP3:
    return "audio/mpeg";

  case WMABASE:
    return "audio/x-ms-wma";

  case VORBIS:
    return "application/ogg";
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
  case MPEG_PS_SD_EU:
  case MPEG_PS_HD_EU:
  case MPEG_PS_SD_NA:
  case MPEG_PS_HD_NA:
    return "video/mpeg";

  case MPEG_TS_SD_EU:
  case MPEG_TS_HD_EU:
  case MPEG_TS_SD_NA:
  case MPEG_TS_HD_NA:
    return "video/vnd.dlna.mpeg-tts";

  case MPEG_TS_SD_EU_ISO:
  case MPEG_TS_HD_EU_ISO:
  case MPEG_TS_SD_NA_ISO:
  case MPEG_TS_HD_NA_ISO:
    return "video/x-mpegts";
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
    return "image/jpeg";

  case PNG_TN:
  case PNG_LRG:
    return "image/png";
  }

  return "";
}

const char * MediaProfiles::suffixFor(AudioProfile profile)
{
  switch (profile)
  {
  case AC3:
    return ".ac3";

  case LPCM:
    return ".lpcm";

  case MP2:
    return ".mp2";

  case MP3:
    return ".mp3";

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
  case MPEG_PS_SD_EU:
  case MPEG_PS_HD_EU:
  case MPEG_PS_SD_NA:
  case MPEG_PS_HD_NA:
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
    return ".ts";
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

#define PROFILE_NAME(x) case x: return #x

const char * MediaProfiles::profileName(AudioProfile profile)
{
  switch (profile)
  {
    PROFILE_NAME(AC3);
    PROFILE_NAME(LPCM);
    PROFILE_NAME(MP2);
    PROFILE_NAME(MP3);
    PROFILE_NAME(WMABASE);
    PROFILE_NAME(VORBIS);
  }

  return "";
}

const char * MediaProfiles::profileName(VideoProfile profile)
{
  switch (profile)
  {
    PROFILE_NAME(MPEG1);
    PROFILE_NAME(MPEG_PS_PAL);
    PROFILE_NAME(MPEG_PS_PAL_XAC3);
    PROFILE_NAME(MPEG_PS_NTSC);
    PROFILE_NAME(MPEG_PS_NTSC_XAC3);
    PROFILE_NAME(MPEG_PS_SD_EU);
    PROFILE_NAME(MPEG_TS_SD_EU);
    PROFILE_NAME(MPEG_TS_SD_EU_ISO);
    PROFILE_NAME(MPEG_PS_HD_EU);
    PROFILE_NAME(MPEG_TS_HD_EU);
    PROFILE_NAME(MPEG_TS_HD_EU_ISO);
    PROFILE_NAME(MPEG_PS_SD_NA);
    PROFILE_NAME(MPEG_TS_SD_NA);
    PROFILE_NAME(MPEG_TS_SD_NA_ISO);
    PROFILE_NAME(MPEG_PS_HD_NA);
    PROFILE_NAME(MPEG_TS_HD_NA);
    PROFILE_NAME(MPEG_TS_HD_NA_ISO);
  }

  return "";
}

const char * MediaProfiles::profileName(ImageProfile profile)
{
  switch (profile)
  {
    PROFILE_NAME(JPEG_TN);
    PROFILE_NAME(JPEG_SM);
    PROFILE_NAME(JPEG_MED);
    PROFILE_NAME(JPEG_LRG);
    PROFILE_NAME(PNG_TN);
    PROFILE_NAME(PNG_LRG);
  }

  return "";
}

#undef PROFILE_NAME

} // End of namespace
