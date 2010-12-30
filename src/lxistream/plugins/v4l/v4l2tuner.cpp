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

#include "v4l2tuner.h"
#include "v4l2device.h"
#include "v4l2input.h"
#include "vbiinput.h"

namespace LXiStream {
namespace V4lBackend {


V4l2Tuner::V4l2Tuner(V4l2Device *parent)
          :SAnalogTuner(parent),
           parent(parent),
           hasTuner(false),
           tunerID(0),
           tunerLow(false)
{
}

quint64 V4l2Tuner::frequency(void) const
{
  Q_ASSERT(parent->videoDev);
  SDebug::MutexLocker l(&parent->videoDev->mutex, __FILE__, __LINE__);

  if (hasTuner)
  {
    v4l2_frequency freqinfo;
    freqinfo.tuner = tunerID;
    freqinfo.type = V4L2_TUNER_ANALOG_TV;

    if (ioctl(parent->videoDev->devDesc, VIDIOC_G_FREQUENCY, &freqinfo) >= 0)
    {
      quint64 freq = freqinfo.frequency;

      if (!tunerLow)
        freq *= Q_UINT64_C(1000);

      return (freq * Q_UINT64_C(1000)) / Q_UINT64_C(16);
    }
  }

  return 0;
}

bool V4l2Tuner::setFrequency(quint64 freq)
{
  Q_ASSERT(parent->videoDev);
  SDebug::MutexLocker l(&parent->videoDev->mutex, __FILE__, __LINE__);

  if (hasTuner)
  {
    bool result = true;

    quint64 frequency = (freq * Q_UINT64_C(16)) / Q_UINT64_C(1000);

    if (!tunerLow)
      frequency /= Q_UINT64_C(1000);

    if (parent->videoDev->streamOn)
      ioctl(parent->videoDev->devDesc, VIDIOC_STREAMOFF, &parent->videoDev->bufferRequest.type);

    v4l2_frequency freqinfo;
    memset(&freqinfo, 0, sizeof(freqinfo));
    freqinfo.tuner = tunerID;
    freqinfo.type = V4L2_TUNER_ANALOG_TV;
    freqinfo.frequency = __u32(frequency);

    if (ioctl(parent->videoDev->devDesc, VIDIOC_S_FREQUENCY, &freqinfo) < 0)
      result = false;

    if (parent->videoDev->streamOn)
    if (ioctl(parent->videoDev->devDesc, VIDIOC_STREAMON, &parent->videoDev->bufferRequest.type ) >= 0)
    {
      for (int i=0; i<parent->videoDev->mappedBuffers; i++)
        parent->videoDev->queueBuffer(i);
    }

    return result;
  }

  return false;
}

bool V4l2Tuner::frequencyInfo(quint64 &low, quint64 &high, quint64 &step) const
{
  Q_ASSERT(parent->videoDev);

  if (hasTuner)
  {
    low  =  40000000;
    high = 900000000;
    step =    250000;

    return true;
  }

  return false;
}

SAnalogTuner::AudioStandard V4l2Tuner::audioStandard(void) const
{
  Q_ASSERT(parent->videoDev);
  SDebug::MutexLocker l(&parent->videoDev->mutex, __FILE__, __LINE__);

  if (hasTuner)
  {
    v4l2_tuner tuner;
    memset(&tuner, 0, sizeof(tuner));
    tuner.index = tunerID;

    if (ioctl(parent->videoDev->devDesc, VIDIOC_G_TUNER, &tuner) >= 0)
      return V4l2Device::fromV4LAudioMode(tuner.audmode);
  }

  return AudioStandard_None;
}

bool V4l2Tuner::setAudioStandard(AudioStandard audioStandard)
{
  Q_ASSERT(parent->videoDev);
  SDebug::MutexLocker l(&parent->videoDev->mutex, __FILE__, __LINE__);

  if (hasTuner)
  {
    bool result = true;

    v4l2_tuner tuner;
    memset(&tuner, 0, sizeof(tuner));
    tuner.index = tunerID;
    tuner.audmode = V4l2Device::toV4LAudioMode(audioStandard);
    if (ioctl(parent->videoDev->devDesc, VIDIOC_S_TUNER, &tuner) < 0)
      result = false;

    return result;
  }

  return false;
}

SAnalogTuner::VideoStandard V4l2Tuner::videoStandard(void) const
{
  Q_ASSERT(parent->videoDev);
  SDebug::MutexLocker l(&parent->videoDev->mutex, __FILE__, __LINE__);

  if (hasTuner)
  {
    v4l2_std_id std;
    memset(&std, 0, sizeof(std));

    if (ioctl(parent->videoDev->devDesc, VIDIOC_G_STD, &std) >= 0)
      return V4l2Device::fromV4LVideoStandard(std);
  }

  return VideoStandard_None;
}

bool V4l2Tuner::setVideoStandard(VideoStandard videoStandard)
{
  Q_ASSERT(parent->videoDev);
  SDebug::MutexLocker l(&parent->videoDev->mutex, __FILE__, __LINE__);

  if (hasTuner)
  {
    bool result = true;

    v4l2_std_id std = V4l2Device::toV4LVideoStandard(videoStandard);

    if (ioctl(parent->videoDev->devDesc, VIDIOC_S_STD, &std) < 0)
      result = false;

    return result;
  }

  return false;
}

STuner::Status V4l2Tuner::signalStatus(void) const
{
  Q_ASSERT(parent->videoDev);
  SDebug::MutexLocker l(&parent->videoDev->mutex, __FILE__, __LINE__);

  Status status;
  status.hasSignal = false;
  status.hasCarrier = false;
  status.hasSync = false;
  status.hasLock = false;
  status.bitErrorRate = 0;
  status.signalStrength = 0.0;
  status.signalNoiseRatio = 0.0;

  if (hasTuner)
  {
    v4l2_tuner tuner;
    tuner.index = tunerID;

    if (ioctl(parent->videoDev->devDesc, VIDIOC_G_TUNER, &tuner) >= 0)
    {
      status.hasSignal =
      status.hasCarrier =
      status.hasSync =
      status.hasLock = (tuner.signal != 0);

      if (status.hasSignal)
      {
        if (parent->vbiDev)
        {
          status.bitErrorRate = parent->vbiDev->bitErrorRate();
          status.signalStrength = parent->vbiDev->signalQuality();

          // In case no VBI signal; 1.0 is a default signal strength.
          if (status.signalStrength == 0.0)
            status.signalStrength = 1.0;
        }
      }

      return status;
    }
  }

  return status;
}


} } // End of namespaces
