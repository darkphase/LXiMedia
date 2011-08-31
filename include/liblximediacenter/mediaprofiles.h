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
    AC3 = 1,
    LPCM,
    MP2, MP3,
    WMABASE,
    VORBIS
  };

  enum VideoProfile
  {
    MPEG1 = 1,
    MPEG_PS_PAL, MPEG_PS_PAL_XAC3,
    MPEG_PS_NTSC, MPEG_PS_NTSC_XAC3,
    MPEG_PS_SD_EU, MPEG_TS_SD_EU, MPEG_TS_SD_EU_ISO,
    MPEG_PS_HD_EU, MPEG_TS_HD_EU, MPEG_TS_HD_EU_ISO,
    MPEG_PS_SD_NA, MPEG_TS_SD_NA, MPEG_TS_SD_NA_ISO,
    MPEG_PS_HD_NA, MPEG_TS_HD_NA, MPEG_TS_HD_NA_ISO
  };

  enum ImageProfile
  {
    JPEG_TN = 1, JPEG_SM, JPEG_MED, JPEG_LRG,
    PNG_TN, PNG_LRG
  };

public:
                                MediaProfiles(void);
                                ~MediaProfiles();

  void                          addProfile(AudioProfile, int priority = 0);
  void                          addProfile(VideoProfile, int priority = 0);
  void                          addProfile(ImageProfile, int priority = 0);

  SUPnPBase::ProtocolList       listProtocols(void);
  SUPnPBase::ProtocolList       listProtocols(const SAudioFormat &);
  SUPnPBase::ProtocolList       listProtocols(const SAudioFormat &, const SVideoFormat &);
  SUPnPBase::ProtocolList       listProtocols(const SSize &imageSize);

  static int                    correctFormat(AudioProfile, SAudioFormat &);
  static int                    correctFormat(VideoProfile, SAudioFormat &);
  static int                    correctFormat(VideoProfile, SVideoFormat &);
  static int                    correctFormat(ImageProfile, SSize &);

  static AudioProfile           audioProfileFor(const QString &contentFeatures);
  static VideoProfile           videoProfileFor(const QString &contentFeatures);
  static ImageProfile           imageProfileFor(const QString &contentFeatures);

  static SAudioCodec            audioCodecFor(AudioProfile, const SAudioFormat &);
  static SAudioCodec            audioCodecFor(VideoProfile, const SAudioFormat &);
  static SVideoCodec            videoCodecFor(VideoProfile, const SVideoFormat &);

  static QString                formatFor(AudioProfile);
  static QString                formatFor(VideoProfile);

  static const char           * mimeTypeFor(AudioProfile);
  static const char           * mimeTypeFor(VideoProfile);
  static const char           * mimeTypeFor(ImageProfile);

  static const char           * suffixFor(AudioProfile);
  static const char           * suffixFor(VideoProfile);
  static const char           * suffixFor(ImageProfile);

private:
  _lxi_internal static int      correctFormatStereo(SAudioFormat &);
  _lxi_internal static int      correctFormatSurround51(SAudioFormat &);
  _lxi_internal static int      correctFormatSurround71(SAudioFormat &);

  _lxi_internal static const char * profileName(AudioProfile);
  _lxi_internal static const char * profileName(VideoProfile);
  _lxi_internal static const char * profileName(ImageProfile);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
