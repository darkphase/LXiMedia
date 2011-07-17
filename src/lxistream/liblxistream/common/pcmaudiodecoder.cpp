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
extern "C" void LXiStream_Common_PcmAudio_decodeS8S16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_decodeU8S16(void *, const void *, size_t);

extern "C" void LXiStream_Common_PcmAudio_swap16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_decodeU16S16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_swapDecodeU16S16(void *, const void *, size_t);

extern "C" void LXiStream_Common_PcmAudio_decodeS32S16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_swapDecodeS32S16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_decodeU32S16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_swapDecodeU32S16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_decodeF32S16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_swapDecodeF32S16(void *, const void *, size_t);

extern "C" void LXiStream_Common_PcmAudio_decodeF64S16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_swapDecodeF64S16(void *, const void *, size_t);

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
    if (c.codec() == "PCM/S8")
      decode = &PcmAudioDecoder::decodeBufferS8<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/U8")
      decode = &PcmAudioDecoder::decodeBufferU8<SAudioFormat::Format_PCM_S16LE>;

    else if (c.codec() == "PCM/S16LE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/S16BE")
      decode = &PcmAudioDecoder::swapBufferS16<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/U16LE")
      decode = &PcmAudioDecoder::decodeBufferU16<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/U16BE")
      decode = &PcmAudioDecoder::swapDecodeBufferU16<SAudioFormat::Format_PCM_S16LE>;

    else if (c.codec() == "PCM/S32LE")
      decode = &PcmAudioDecoder::decodeBufferS32<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/S32BE")
      decode = &PcmAudioDecoder::swapDecodeBufferS32<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/U32LE")
      decode = &PcmAudioDecoder::decodeBufferU32<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/U32BE")
      decode = &PcmAudioDecoder::swapDecodeBufferU32<SAudioFormat::Format_PCM_S16LE>;

    else if (c.codec() == "PCM/F32LE")
      decode = &PcmAudioDecoder::decodeBufferF32<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/F32BE")
      decode = &PcmAudioDecoder::swapDecodeBufferF32<SAudioFormat::Format_PCM_S16LE>;

    else if (c.codec() == "PCM/F64LE")
      decode = &PcmAudioDecoder::decodeBufferF64<SAudioFormat::Format_PCM_S16LE>;
    else if (c.codec() == "PCM/F64BE")
      decode = &PcmAudioDecoder::swapDecodeBufferF64<SAudioFormat::Format_PCM_S16LE>;
#else
    if (c.codec() == "PCM/S8")
      decode = &PcmAudioDecoder::decodeBufferS8<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/U8")
      decode = &PcmAudioDecoder::decodeBufferU8<SAudioFormat::Format_PCM_S16BE>;

    else if (c.codec() == "PCM/S16LE")
      decode = &PcmAudioDecoder::swapBuffer16<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/S16BE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/U16LE")
      decode = &PcmAudioDecoder::swapDecodeBuffer16<SAudioFormat::Format_PCM_U16BE>;
    else if (c.codec() == "PCM/U16BE")
      decode = &PcmAudioDecoder::copyBuffer<SAudioFormat::Format_PCM_U16BE>;

    else if (c.codec() == "PCM/S32LE")
      decode = &PcmAudioDecoder::swapDecodeBufferS32<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/S32BE")
      decode = &PcmAudioDecoder::decodeBufferS32<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/U32LE")
      decode = &PcmAudioDecoder::swapDecodeBufferU32<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/U32BE")
      decode = &PcmAudioDecoder::decodeBufferU32<SAudioFormat::Format_PCM_S16BE>;

    else if (c.codec() == "PCM/F32LE")
      decode = &PcmAudioDecoder::swapDecodeBufferF32<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/F32BE")
      decode = &PcmAudioDecoder::decodeBufferF32<SAudioFormat::Format_PCM_S16BE>;

    else if (c.codec() == "PCM/F64LE")
      decode = &PcmAudioDecoder::swapDecodeBufferF64<SAudioFormat::Format_PCM_S16BE>;
    else if (c.codec() == "PCM/F64BE")
      decode = &PcmAudioDecoder::decodeBufferF64<SAudioFormat::Format_PCM_S16BE>;
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
SAudioBuffer PcmAudioDecoder::decodeBufferS8(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(qint8)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_decodeS8S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::decodeBufferU8(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(quint8)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_decodeU8S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
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
SAudioBuffer PcmAudioDecoder::swapBufferS16(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / dstFormat.sampleSize()) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_swap16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::decodeBufferU16(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / dstFormat.sampleSize()) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_decodeU16S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::swapDecodeBufferU16(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / dstFormat.sampleSize()) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_swapDecodeU16S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::decodeBufferS32(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(qint32)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_decodeS32S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::swapDecodeBufferS32(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(qint32)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_swapDecodeS32S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::decodeBufferU32(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(quint32)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_decodeU32S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::swapDecodeBufferU32(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(quint32)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_swapDecodeU32S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::decodeBufferF32(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(float)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_decodeF32S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::swapDecodeBufferF32(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(float)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_swapDecodeF32S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::decodeBufferF64(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(double)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_decodeF64S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

template <SAudioFormat::Format format>
SAudioBuffer PcmAudioDecoder::swapDecodeBufferF64(const SEncodedAudioBuffer &buffer)
{
  const SAudioFormat dstFormat(format,
                               buffer.codec().channelSetup(),
                               buffer.codec().sampleRate());

  SAudioBuffer dstBuffer(dstFormat, (buffer.size() / sizeof(double)) / dstFormat.numChannels());

  LXiStream_Common_PcmAudio_swapDecodeF64S16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

} } // End of namespaces
