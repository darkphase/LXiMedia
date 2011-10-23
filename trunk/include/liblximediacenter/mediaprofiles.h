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

#ifndef LXIMEDIACENTER_MEDIAPROFILES_H
#define LXIMEDIACENTER_MEDIAPROFILES_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <LXiCore>
#include <LXiServer>
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

    // Vorbis
    VORBIS = 0x2000
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

    // WMV
    WMVMED_BASE = 0x1000,

    // Non-standard
    MPEG_PS_SD_EU_NONSTD = 0x7000, MPEG_PS_HD_EU_NONSTD,
    MPEG_PS_SD_NA_NONSTD, MPEG_PS_HD_NA_NONSTD,

    MPEG4_P2_MATROSKA_MP3_SD_NONSTD, MPEG4_P2_MATROSKA_MP3_HD_NONSTD,
    MPEG4_P2_MATROSKA_AAC_SD_NONSTD, MPEG4_P2_MATROSKA_AAC_HD_NONSTD,
    MPEG4_P2_MATROSKA_AC3_SD_NONSTD, MPEG4_P2_MATROSKA_AC3_HD_NONSTD,
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

  SUPnPBase::ProtocolList       listProtocols(const QString &client);
  SUPnPBase::ProtocolList       listProtocols(const QString &client, const SAudioFormat &, bool seekable);
  SUPnPBase::ProtocolList       listProtocols(const QString &client, const SAudioFormat &, const SVideoFormat &, bool seekable);
  SUPnPBase::ProtocolList       listProtocols(const QString &client, const SSize &imageSize);

  QStringList                   supportedAudioProfiles(const QString &client);
  QStringList                   supportedVideoProfiles(const QString &client);
  QStringList                   supportedImageProfiles(const QString &client);

  static int                    correctFormat(AudioProfile, SAudioFormat &);
  static int                    correctFormat(VideoProfile, SAudioFormat &);
  static int                    correctFormat(VideoProfile, SVideoFormat &);
  static int                    correctFormat(ImageProfile, SSize &);

  AudioProfile                  audioProfileFor(const QString &contentFeatures) const;
  VideoProfile                  videoProfileFor(const QString &contentFeatures) const;
  ImageProfile                  imageProfileFor(const QString &contentFeatures) const;

  static QString                audioCodecFor(AudioProfile, SAudioFormat::Channels);
  static QString                audioCodecFor(VideoProfile, SAudioFormat::Channels);
  static QString                videoCodecFor(VideoProfile);
  static QString                imageCodecFor(ImageProfile);

  static QString                formatFor(AudioProfile);
  static QString                formatFor(VideoProfile);

  static const char           * mimeTypeFor(AudioProfile);
  static const char           * mimeTypeFor(VideoProfile);
  static const char           * mimeTypeFor(ImageProfile);

  static const char           * suffixFor(AudioProfile);
  static const char           * suffixFor(VideoProfile);
  static const char           * suffixFor(ImageProfile);

private:
  _lxi_internal static int      profilePriority(AudioProfile);
  _lxi_internal static int      profilePriority(VideoProfile);
  _lxi_internal static int      profilePriority(ImageProfile);

  _lxi_internal QString         findBestClient(const QString &);

  _lxi_internal static int      correctFormatStereo(SAudioFormat &);
  _lxi_internal static int      correctFormatSurround51(SAudioFormat &);
  _lxi_internal static int      correctFormatSurround71(SAudioFormat &);

  _lxi_internal static QByteArray profileName(AudioProfile);
  _lxi_internal static QByteArray profileName(VideoProfile);
  _lxi_internal static QByteArray profileName(ImageProfile);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
