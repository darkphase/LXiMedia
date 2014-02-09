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

#ifndef LXIMEDIACENTER_MEDIAPROFILES_H
#define LXIMEDIACENTER_MEDIAPROFILES_H

#include <QtCore>
#include <LXiCore>
#include <LXiStream>
#include "export.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC MediaProfiles
{
public:
  enum AudioProfile
  {
    // PCM
    LPCM = 1,

    // MPEG
    MP2 = 0x0100, MP3,
    AAC_ADTS,
    AC3,

    // WMA
    WMABASE = 0x1000,

    // Non-standard
    VORBIS_NONSTD = 0x7000, FLAC_NONSTD,
    WAV_NONSTD
  };

  enum VideoProfile
  {
    // MPEG 1
    MPEG1 = 0x0100,

    // MPEG 2
    MPEG_PS_PAL = 0x0200, MPEG_PS_PAL_XAC3,
    MPEG_PS_NTSC, MPEG_PS_NTSC_XAC3,

    MPEG_TS_SD_EU, MPEG_TS_SD_EU_ISO,
    MPEG_TS_HD_EU, MPEG_TS_HD_EU_ISO,
    MPEG_TS_SD_NA, MPEG_TS_SD_NA_ISO,
    MPEG_TS_HD_NA, MPEG_TS_HD_NA_ISO,

    // MPEG 4 part 2
    MPEG4_P2_TS_SP_AAC = 0x0400, MPEG4_P2_TS_SP_AAC_ISO,
    MPEG4_P2_TS_SP_MPEG1_L3, MPEG4_P2_TS_SP_MPEG1_L3_ISO,
    MPEG4_P2_TS_SP_MPEG2_L2, MPEG4_P2_TS_SP_MPEG2_L2_ISO,
    MPEG4_P2_TS_SP_AC3_L3, MPEG4_P2_TS_SP_AC3_ISO,

    MPEG4_P2_TS_ASP_AAC, MPEG4_P2_TS_ASP_AAC_ISO,
    MPEG4_P2_TS_ASP_MPEG1_L3, MPEG4_P2_TS_ASP_MPEG1_L3_ISO,
    MPEG4_P2_TS_ASP_AC3_L3, MPEG4_P2_TS_ASP_AC3_ISO,

    // MPEG 4 part 10
    AVC_TS_MP_HD_AC3 = 0x0410, AVC_TS_MP_HD_AC3_ISO,

    // WMV
    WMVMED_BASE = 0x1000,

    // Non-standard
    MPEG_PS_SD_EU_NONSTD = 0x7000, MPEG_PS_HD_EU_NONSTD,
    MPEG_PS_SD_NA_NONSTD, MPEG_PS_HD_NA_NONSTD,

    MPEG4_P2_MATROSKA_MP3_SD_NONSTD, MPEG4_P2_MATROSKA_MP3_HD_NONSTD,
    MPEG4_P2_MATROSKA_AAC_SD_NONSTD, MPEG4_P2_MATROSKA_AAC_HD_NONSTD,
    MPEG4_P2_MATROSKA_AC3_SD_NONSTD, MPEG4_P2_MATROSKA_AC3_HD_NONSTD,

    OGG_THEORA_VORBIS_SD_NONSTD, OGG_THEORA_FLAC_SD_NONSTD
  };

  enum ImageProfile
  {
    // JPEG
    JPEG_TN = 0x0100, JPEG_SM, JPEG_MED, JPEG_LRG,

    // PNG
    PNG_TN = 0x0200, PNG_LRG
  };

public:
                                MediaProfiles(void);
                                ~MediaProfiles();

  void                          openDeviceConfig(const QString &);

  void                          setCodecs(const QSet<QString> &audioCodecs, const QSet<QString> &videoCodecs, const QSet<QString> &imageCodecs, const QSet<QString> &formats);

  QStringList                   enabledAudioProfiles(void);
  QStringList                   enabledVideoProfiles(void);
  QStringList                   enabledImageProfiles(void);
  bool                          isProfileEnabled(AudioProfile) const;
  bool                          isProfileEnabled(VideoProfile) const;
  bool                          isProfileEnabled(ImageProfile) const;

  ConnectionManager::ProtocolList listProtocols(const QString &client);
  ConnectionManager::ProtocolList listProtocols(const QString &client, const SAudioFormat &);
  ConnectionManager::ProtocolList listProtocols(const QString &client, const SAudioFormat &, const SVideoFormat &);
  ConnectionManager::ProtocolList listProtocols(const QString &client, const SSize &imageSize);

  QStringList                   supportedAudioProfiles(const QString &client);
  QStringList                   supportedVideoProfiles(const QString &client);
  QStringList                   supportedImageProfiles(const QString &client);

  static SSize                  maximumResolution(const QStringList &profileNames);
  static SSize                  maximumResolution(VideoProfile);
  static SAudioFormat::Channels maximumChannels(const QStringList &profileNames);
  static SAudioFormat::Channels maximumChannels(AudioProfile);
  static SAudioFormat::Channels maximumChannels(VideoProfile);

  static int                    correctFormat(AudioProfile, SAudioFormat &);
  static int                    correctFormat(VideoProfile, SAudioFormat &);
  static int                    correctFormat(VideoProfile, SVideoFormat &);
  static int                    correctFormat(ImageProfile, SSize &);

  AudioProfile                  audioProfileFor(const QString &contentFeatures) const;
  VideoProfile                  videoProfileFor(const QString &contentFeatures) const;
  ImageProfile                  imageProfileFor(const QString &contentFeatures) const;

  static QByteArray             audioCodecFor(AudioProfile, SAudioFormat::Channels);
  static QByteArray             audioCodecFor(VideoProfile, SAudioFormat::Channels);
  static QByteArray             videoCodecFor(VideoProfile);
  static QByteArray             imageCodecFor(ImageProfile);

  static QByteArray             formatFor(AudioProfile);
  static QByteArray             formatFor(VideoProfile);
  static QByteArray             formatFor(ImageProfile);

  static const char           * mimeTypeFor(AudioProfile);
  static const char           * mimeTypeFor(VideoProfile);
  static const char           * mimeTypeFor(ImageProfile);

  static const char           * suffixFor(AudioProfile);
  static const char           * suffixFor(VideoProfile);
  static const char           * suffixFor(ImageProfile);

private:
  static int                    profilePriority(AudioProfile);
  static int                    profilePriority(VideoProfile);
  static int                    profilePriority(ImageProfile);

  QString                       findBestClient(const QString &);

  static int                    correctFormatStereo(SAudioFormat &);
  static int                    correctFormatSurround51(SAudioFormat &);
  static int                    correctFormatSurround71(SAudioFormat &);

  static QByteArray             profileName(AudioProfile);
  static QByteArray             profileName(VideoProfile);
  static QByteArray             profileName(ImageProfile);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
