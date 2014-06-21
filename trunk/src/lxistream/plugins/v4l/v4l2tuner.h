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

#ifndef V4LBACKEND_V4L2TUNER_H
#define V4LBACKEND_V4L2TUNER_H

#include <QtCore>
#include <LXiStream>
#include "v4l2device.h"

namespace LXiStream {
namespace V4lBackend {

class V4l2Device;


class V4l2Tuner : public SAnalogTuner
{
Q_OBJECT
public:
                                V4l2Tuner(V4l2Device *);

  virtual quint64               frequency(void) const;
  virtual bool                  setFrequency(quint64);
  virtual bool                  frequencyInfo(quint64 &low, quint64 &high, quint64 &step) const;
  virtual Status                signalStatus(void) const;

  virtual AudioStandard         audioStandard(void) const;
  virtual bool                  setAudioStandard(AudioStandard);
  virtual VideoStandard         videoStandard(void) const;
  virtual bool                  setVideoStandard(VideoStandard);

private:
  V4l2Device            * const parent;

public:
  bool                          hasTuner;
  int                           tunerID;
  bool                          tunerLow;
};


} } // End of namespaces

#endif
