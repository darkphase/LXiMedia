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

#ifndef LXSTREAMFRONTEND_SANALOGTUNER_H
#define LXSTREAMFRONTEND_SANALOGTUNER_H

#include <QtCore>
#include "stuner.h"

namespace LXiStream {

class SAnalogTuner : public STuner
{
Q_OBJECT
Q_PROPERTY(AudioStandard audioStandard READ audioStandard WRITE setAudioStandard)
Q_PROPERTY(VideoStandard videoStandard READ videoStandard WRITE setVideoStandard)
public:
  enum AudioStandard
  {
    AudioStandard_None = 0,
    AudioStandard_Mono,
    AudioStandard_Stereo,
    AudioStandard_Lang1,
    AudioStandard_Lang2
  };

  enum VideoStandard
  {
    VideoStandard_None = 0,

    VideoStandard_PAL = 0x1000,
    VideoStandard_PAL_B,
    VideoStandard_PAL_B1,
    VideoStandard_PAL_G,
    VideoStandard_PAL_H,
    VideoStandard_PAL_I,
    VideoStandard_PAL_D,
    VideoStandard_PAL_D1,
    VideoStandard_PAL_K,
    VideoStandard_PAL_BG,
    VideoStandard_PAL_DK,
    VideoStandard_PAL_M,   //!< 60 Hz
    VideoStandard_PAL_N,   //!< 60 Hz
    VideoStandard_PAL_Nc,  //!< 60 Hz
    VideoStandard_PAL_60,  //!< 60 Hz

    VideoStandard_NTSC = 0x2000,
    VideoStandard_NTSC_M,
    VideoStandard_NTSC_M_JP,

    VideoStandard_SECAM = 0x3000,
    VideoStandard_SECAM_B,
    VideoStandard_SECAM_D,
    VideoStandard_SECAM_G,
    VideoStandard_SECAM_H,
    VideoStandard_SECAM_K,
    VideoStandard_SECAM_K1,
    VideoStandard_SECAM_L,
    VideoStandard_SECAM_DK
  };

public:
  explicit                      SAnalogTuner(QObject *parent = NULL);

  virtual AudioStandard         audioStandard(void) const = 0;
  virtual bool                  setAudioStandard(AudioStandard) = 0;
  virtual VideoStandard         videoStandard(void) const = 0;
  virtual bool                  setVideoStandard(VideoStandard) = 0;
};


} // End of namespace

#endif
