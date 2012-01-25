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

#ifndef LXSTREAMCOMMON_AUDIOOUTPUT_H
#define LXSTREAMCOMMON_AUDIOOUTPUT_H
/*
#include <QtCore>
#include "audiomixer.h"

namespace LXiStream {
namespace Common {

class AudioOutputNode;

class AudioOutput : public AudioMixer
{
Q_OBJECT
friend class AudioOutputNode;
protected:
  enum Output { ANALOG_OUTPUT = 0, IEC958_OUTPUT = 1 };

public:
                                AudioOutput(bool, QObject *);

  virtual SNodes::Audio::MixerInput * createNode(void);

public: // From SNode
  virtual Behavior              behavior(void) const;
  virtual SCodecList            acceptedCodecs(void) const;
  virtual bool                  prepare(const SCodecList &);
  virtual bool                  unprepare(void);
  virtual Result                processBuffer(const SBuffer &, SBufferList &);

protected:
  virtual void                  writeAudio(const SAudioBuffer &) = 0;

  virtual SAudioBuffer          createAC3SilentPacket(SAudioCodec::Channels);
  virtual SAudioBuffer          createSilentPacket(SAudioCodec::Channels);

private:
  void                          process(void);

protected:
  static const unsigned         maxDelay = 20; // msec
  static const unsigned         numBuffers;
  static const unsigned         packetSize = 1024;
  static const unsigned         maxNumChannels = 6;
  static const unsigned         maxSamples = 16 * packetSize;
  static const size_t           ac3PacketSize = maxNumChannels * packetSize; // 6144
  static const unsigned         maxSampleRate = 48000;
  static const unsigned         bitRate = 96000;

  QAtomicInt                    running;
  bool                          iec958;
  SNodes::Audio::Resampler    * audioResamplerNode;
  SAudioEncoderNode             audioEncoderNode;
};


class AudioOutputNode : public AudioMixerNode
{
Q_OBJECT
friend class AudioOutput;
public:
  explicit                      AudioOutputNode(AudioOutput *);

  virtual Behavior              behavior(void) const;
  virtual Result                processBuffer(const SBuffer &, SBufferList &);

  virtual STime                 latency(void) const;

private:
  AudioOutput           * const parent;
};


} } // End of namespaces
*/
#endif
