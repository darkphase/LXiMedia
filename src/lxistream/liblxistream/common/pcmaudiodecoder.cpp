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

#include "pcmaudiodecoder.h"

// Implemented in pcmaudio.swap.c
extern "C" void LXiStream_Common_PcmAudio_swap16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_swap32(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_swap64(void *, const void *, size_t);

namespace LXiStream {
namespace Common {

PcmAudioDecoder::PcmAudioDecoder(const QString &, QObject *parent)
  : SInterfaces::AudioDecoder(parent),
    decode(NULL),
    nextTimeStamp(STime::null)
{
}

PcmAudioDecoder::~PcmAudioDecoder()
{
}

bool PcmAudioDecoder::openCodec(const SAudioCodec &c, Flags)
{
  if (c.codec().startsWith("PCM/"))
  {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    if (c.codec() == "PCM/S16LE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/S16BE")
      decode = &PcmAudioDecoder::swapBuffer16<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/U16LE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_U16LE>;
    else if (c.codec() == "PCM/U16BE")
      decode = &PcmAudioDecoder::swapBuffer16<SAudioFormat::Format_PCM_U16LE>;

    else if (c.codec() == "PCM/S32LE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_S32LE>;
    else if (c.codec() == "PCM/S32BE")
      decode = &PcmAudioDecoder::swapBuffer32<SAudioFormat::Format_PCM_S32LE>;
    else if (c.codec() == "PCM/U32LE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_U32LE>;
    else if (c.codec() == "PCM/U32BE")
      decode = &PcmAudioDecoder::swapBuffer32<SAudioFormat::Format_PCM_U32LE>;

    else if (c.codec() == "PCM/F32LE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_F32LE>;
    else if (c.codec() == "PCM/F32BE")
      decode = &PcmAudioDecoder::swapBuffer32<SAudioFormat::Format_PCM_F32LE>;

    else if (c.codec() == "PCM/F64LE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_F64LE>;
    else if (c.codec() == "PCM/F64BE")
      decode = &PcmAudioDecoder::swapBuffer64<SAudioFormat::Format_PCM_F64LE>;
#else
    if (c.codec() == "PCM/S16LE")
      decode = &PcmAudioDecoder::swapBuffer16<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/S16BE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/U16LE")
      decode = &PcmAudioDecoder::swapBuffer16<SAudioFormat::Format_PCM_U16BE>;
    else if (c.codec() == "PCM/U16BE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_U16BE>;

    else if (c.codec() == "PCM/S32LE")
      decode = &PcmAudioDecoder::swapBuffer32<SAudioFormat::Format_PCM_S32BE>;
    else if (c.codec() == "PCM/S32BE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_S32BE>;
    else if (c.codec() == "PCM/U32LE")
      decode = &PcmAudioDecoder::swapBuffer32<SAudioFormat::Format_PCM_U32BE>;
    else if (c.codec() == "PCM/U32BE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_U32BE>;

    else if (c.codec() == "PCM/F32LE")
      decode = &PcmAudioDecoder::swapBuffer32<SAudioFormat::Format_PCM_F32BE>;
    else if (c.codec() == "PCM/F32BE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_F32BE>;

    else if (c.codec() == "PCM/F64LE")
      decode = &PcmAudioDecoder::swapBuffer64<SAudioFormat::Format_PCM_F64BE>;
    else if (c.codec() == "PCM/F64BE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_F64BE>;
#endif
  }

  return decode != NULL;
}

SAudioBufferList PcmAudioDecoder::decodeBuffer(const SEncodedAudioBuffer &audioBuffer)
{
  SAudioBufferList output;

  if (decode && !audioBuffer.isNull())
  {
    SAudioBuffer destBuffer = decode(audioBuffer);

    STime timeStamp = nextTimeStamp;
    if (audioBuffer.presentationTimeStamp().isValid())
      timeStamp = audioBuffer.presentationTimeStamp();
    else if (audioBuffer.decodingTimeStamp().isValid())
      timeStamp = audioBuffer.decodingTimeStamp();

    destBuffer.setTimeStamp(timeStamp);
    nextTimeStamp = timeStamp + destBuffer.duration();

    output << destBuffer;
  }
  else if (audioBuffer.isNull())
    output << SAudioBuffer();

  return output;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::copyBuffer(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  return SAudioBuffer(dstFormat, buffer.memory());
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::swapBuffer16(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / dstFormat.sampleSize()) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_swap16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::swapBuffer32(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / dstFormat.sampleSize()) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_swap32(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::swapBuffer64(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / dstFormat.sampleSize()) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_swap64(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

} } // End of namespaces
