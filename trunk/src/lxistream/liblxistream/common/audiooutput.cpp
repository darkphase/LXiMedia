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

#include "audiooutput.h"
/*
namespace LXiStream {
namespace Common {


const unsigned AudioOutput::numBuffers = 15;


AudioOutput::AudioOutput(bool iec958, QObject *parent)
    : AudioMixer(parent),
      running(0),
      iec958(iec958),
      audioResamplerNode(NULL),
      audioEncoderNode()
{
  audioResamplerNode = SSystem::createObject<SNodes::Audio::Resampler>(this);
  if (audioResamplerNode)
  {
    audioResamplerNode->setChannels(SAudioCodec::Channel_Stereo);
    audioResamplerNode->setSampleRate(48000);

    return;
  }

  delete audioResamplerNode;
  audioResamplerNode = NULL;
}

SNodes::Audio::MixerInput * AudioOutput::createNode(void)
{
  return new AudioOutputNode(this);
}

SNode::Behavior AudioOutput::behavior(void) const
{
  return AudioMixer::behavior() | Behavior_Blocking;
}

SCodecList AudioOutput::acceptedCodecs(void) const
{
  SCodecList list;

  if (iec958)
  {
    list += SAudioCodec("AC3");
    list += SAudioCodec("DTS");
  }
  else
    list += SAudioCodec(SAudioCodec::Format_PCM_S16, SAudioCodec::Channel_Stereo);

  return list;
}

bool AudioOutput::prepare(const SCodecList &codecs)
{
  if (audioResamplerNode)
  if (!audioResamplerNode->prepare())
    return false;

  if (audioEncoderNode.prepare(acceptedCodecs()))
  {
    audioResamplerNode->unprepare();
    return false;
  }

  if (AudioMixer::prepare(codecs))
    return true;

  audioEncoderNode.unprepare();

  if (audioResamplerNode)
    audioResamplerNode->unprepare();

  return false;
}

bool AudioOutput::unprepare(void)
{
  bool result = true;

  result &= audioEncoderNode.unprepare();

  if (audioResamplerNode)
    result &= audioResamplerNode->unprepare();

  result &= AudioMixer::unprepare();
  return result;
}

SNode::Result AudioOutput::processBuffer(const SBuffer &input, SBufferList &)
{
  const SAudioBuffer audioBuffer = input;

  if (!audioBuffer.isNull())
  {
    if (audioBuffer.codec() == "DTS")
    {
      writeAudio(audioBuffer);
    }
    else if (audioBuffer.codec() == "AC3")
    {
      const quint16 * const rawAC3packet = reinterpret_cast<const quint16 *>(audioBuffer.bits());
      const size_t dataSize = audioBuffer.numBytes();

      SAudioBuffer destBuffer(ac3PacketSize);
      destBuffer.setNumBytes(ac3PacketSize);
      quint16 * const packet = reinterpret_cast<quint16 *>(destBuffer.bits());
      
      packet[0] = 0xF872;   // iec 61937 syncword 1
      packet[1] = 0x4E1F;   // iec 61937 syncword 2
      packet[2] = 0x0001;   // data-type ac3
      packet[2] |= qFromBigEndian(rawAC3packet[3]) & 0x0700; // bsmod
      packet[3] = dataSize << 3; // number of bits in payload

      memset(reinterpret_cast<quint8 *>(packet + 4) + dataSize - 2, 0, ac3PacketSize - 8 - (dataSize - 2));
      for (unsigned i=0; i<dataSize/2; i++)
        packet[i+4] = qFromBigEndian(rawAC3packet[i]);

      destBuffer.setCodec(SAudioCodec("AC3"));
      destBuffer.setTimeStamp(audioBuffer.timeStamp());

      writeAudio(destBuffer);
    }
    else if (audioBuffer.codec().numChannels() > 2)
    {
      SBufferList resampled;
      if (audioResamplerNode->processBuffer(audioBuffer, resampled) == Result_Active)
      foreach (const SBuffer &resampledBuffer, resampled)
      {
        SBufferList encoded, dummy;
        if (audioEncoderNode.processBuffer(resampledBuffer, encoded) == Result_Active)
        foreach (const SBuffer &encodedBuffer, encoded)
          AudioOutput::processBuffer(encodedBuffer, dummy);
      }
    }
    else if (audioBuffer.codec() == SAudioCodec::Format_PCM_S16)
    {
      if ((codec().sampleRate() == audioBuffer.codec().sampleRate()) &&
          (codec().channelSetup() == audioBuffer.codec().channelSetup()))
      {
        writeAudio(audioBuffer);
      }
      else if (audioResamplerNode)
      {
        SBufferList resampled;
        if (audioResamplerNode->processBuffer(audioBuffer, resampled) == Result_Active)
        foreach (const SBuffer &resampledBuffer, resampled)
          writeAudio(resampledBuffer);
      }
    }
  }

  return Result_Active;
}

void AudioOutput::process(void)
{
  SBufferList mixed, dummy;
  if (AudioMixer::processBuffer(SBuffer(), mixed) == Result_Active)
  foreach (const SBuffer &mixedBuffer, mixed)
    AudioOutput::processBuffer(mixedBuffer, dummy);
}

SAudioBuffer AudioOutput::createAC3SilentPacket(SAudioCodec::Channels channels)
{
  qint16 silentData[packetSize * maxNumChannels];
  memset(silentData, 0, sizeof(silentData));
  SAudioBuffer tmpBuffer(&silentData, sizeof(silentData));
  tmpBuffer.setCodec(SAudioCodec(SAudioCodec::Format_PCM_S16, channels, maxSampleRate));

  SBufferList encoded;
  if (audioEncoderNode.processBuffer(tmpBuffer, encoded) == Result_Active)
  {
    const SAudioBuffer tmpPacketBuffer = encoded;
    const quint16 * const tmpPacket = reinterpret_cast<const quint16 *>(tmpPacketBuffer.bits());
    const size_t dataSize = qMin(tmpPacketBuffer.numBytes(), ac3PacketSize - 8);

    SAudioBuffer destBuffer(ac3PacketSize + 0);
    quint16 * const packet = reinterpret_cast<quint16 *>(destBuffer.bits());

    packet[0] = 0xF872;   // iec 61937 syncword 1
    packet[1] = 0x4E1F;   // iec 61937 syncword 2
    packet[2] = 0x0001;   // data-type ac3
    packet[2] |= (silentData[5] & 0x0007) << 8; // bsmod
    packet[3] = dataSize << 3; // number of bits in payload

    memset(reinterpret_cast<quint8 *>(packet + 4) + dataSize - 2, 0, ac3PacketSize - 8 - (dataSize - 2));
    for (unsigned i=0; i<dataSize/2; i++)
      packet[i+4] = qFromBigEndian(tmpPacket[i]);

    destBuffer.setCodec(SAudioCodec("AC3"));
    destBuffer.setNumBytes(ac3PacketSize);
    destBuffer.setTimeStamp(STime()); // No timestamp

    return destBuffer;
  }

  return SBuffer();
}

SAudioBuffer AudioOutput::createSilentPacket(SAudioCodec::Channels channels)
{
  static const size_t bufferSize = packetSize * SAudioCodec::numChannels(channels) * sizeof(qint16);
  SAudioBuffer destBuffer(bufferSize);
  quint16 * const packet = reinterpret_cast<quint16 *>(destBuffer.bits());
  memset(packet, 0, bufferSize);

  destBuffer.setCodec(SAudioCodec(SAudioCodec::Format_PCM_S16, channels));
  destBuffer.setNumBytes(bufferSize);
  destBuffer.setTimeStamp(STime()); // No timestamp

  return destBuffer;
}


AudioOutputNode::AudioOutputNode(AudioOutput *parent)
                :AudioMixerNode(parent),
                 parent(parent)
{
}

SNode::Behavior AudioOutputNode::behavior(void) const
{
  return AudioMixerNode::behavior() | Behavior_Blocking;
}

STime AudioOutputNode::latency(void) const
{
  return AudioMixerNode::latency() + parent->latency();
}

SNode::Result AudioOutputNode::processBuffer(const SBuffer &input, SBufferList &output)
{
  const SAudioBuffer audioBuffer = input;

  if (!audioBuffer.isNull())
  {
    // Switch to surround if supported.
    if (parent->iec958 && (audioBuffer.codec().numChannels() > parent->codec().numChannels()))
    {
      SAudioCodec codec = parent->codec();
      codec.setChannelSetup(audioBuffer.codec().channelSetup());
      parent->setCodec(codec);
    }

    SNode::Result result = AudioMixerNode::processBuffer(audioBuffer, output);

    parent->process();

    return result;
  }

  return Result_Idle;
}

} } // End of namespaces
*/
