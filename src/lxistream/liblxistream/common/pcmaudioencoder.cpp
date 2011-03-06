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

#include "pcmaudioencoder.h"

// Implemented in pcmaudio.swap.c
extern "C" void LXiStream_Common_PcmAudio_swap16(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_swap32(void *, const void *, size_t);
extern "C" void LXiStream_Common_PcmAudio_swap64(void *, const void *, size_t);

namespace LXiStream {
namespace Common {

PcmAudioEncoder::PcmAudioEncoder(const QString &, QObject *parent)
  : SInterfaces::AudioEncoder(parent),
    encode(NULL)
{
}

PcmAudioEncoder::~PcmAudioEncoder()
{
}

bool PcmAudioEncoder::openCodec(const SAudioCodec &c, Flags)
{
  if (c.codec().startsWith("PCM/"))
  {
    outCodec = c;

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    if (c.codec() == "PCM/S16LE")
      encode = &PcmAudioEncoder::copyBuffer;
    else if (c.codec() == "PCM/S16BE")
      encode = &PcmAudioEncoder::swapBuffer16;
    else if (c.codec() == "PCM/U16LE")
      encode = &PcmAudioEncoder::copyBuffer;
    else if (c.codec() == "PCM/U16BE")
      encode = &PcmAudioEncoder::swapBuffer16;

    else if (c.codec() == "PCM/S32LE")
      encode = &PcmAudioEncoder::copyBuffer;
    else if (c.codec() == "PCM/S32BE")
      encode = &PcmAudioEncoder::swapBuffer32;
    else if (c.codec() == "PCM/U32LE")
      encode = &PcmAudioEncoder::copyBuffer;
    else if (c.codec() == "PCM/U32BE")
      encode = &PcmAudioEncoder::swapBuffer32;

    else if (c.codec() == "PCM/F32LE")
      encode = &PcmAudioEncoder::copyBuffer;
    else if (c.codec() == "PCM/F32BE")
      encode = &PcmAudioEncoder::swapBuffer32;

    else if (c.codec() == "PCM/F64LE")
      encode = &PcmAudioEncoder::copyBuffer;
    else if (c.codec() == "PCM/F64BE")
      encode = &PcmAudioEncoder::swapBuffer64;
#else
    if (c.codec() == "PCM/S16LE")
      encode = &PcmAudioEncoder::swapBuffer16;
    else if (c.codec() == "PCM/S16BE")
      encode = &PcmAudioEncoder::copyBuffer;
    else if (c.codec() == "PCM/U16LE")
      encode = &PcmAudioEncoder::swapBuffer16;
    else if (c.codec() == "PCM/U16BE")
      encode = &PcmAudioEncoder::copyBuffer;

    else if (c.codec() == "PCM/S32LE")
      encode = &PcmAudioEncoder::swapBuffer32;
    else if (c.codec() == "PCM/S32BE")
      encode = &PcmAudioEncoder::copyBuffer;
    else if (c.codec() == "PCM/U32LE")
      encode = &PcmAudioEncoder::swapBuffer32;
    else if (c.codec() == "PCM/U32BE")
      encode = &PcmAudioEncoder::copyBuffer;

    else if (c.codec() == "PCM/F32LE")
      encode = &PcmAudioEncoder::swapBuffer32;
    else if (c.codec() == "PCM/F32BE")
      encode = &PcmAudioEncoder::copyBuffer;

    else if (c.codec() == "PCM/F64LE")
      encode = &PcmAudioEncoder::swapBuffer64;
    else if (c.codec() == "PCM/F64BE")
      encode = &PcmAudioEncoder::copyBuffer;
#endif
  }

  return encode != NULL;
}

SAudioCodec PcmAudioEncoder::codec(void) const
{
  return outCodec;
}

SEncodedAudioBufferList PcmAudioEncoder::encodeBuffer(const SAudioBuffer &audioBuffer)
{
  SEncodedAudioBufferList output;

  if (encode && !audioBuffer.isNull())
  {
    SEncodedAudioBuffer destBuffer = encode(audioBuffer, outCodec);

    destBuffer.setPresentationTimeStamp(audioBuffer.timeStamp());
    destBuffer.setDecodingTimeStamp(audioBuffer.timeStamp());

    output << destBuffer;
  }
  else if (audioBuffer.isNull())
    output << SEncodedAudioBuffer();

  return output;
}

SEncodedAudioBuffer PcmAudioEncoder::copyBuffer(const SAudioBuffer &buffer, const QString &codec)
{
  const SAudioCodec dstCodec(codec,
                             buffer.format().channelSetup(),
                             buffer.format().sampleRate());

  return SEncodedAudioBuffer(dstCodec, buffer.memory());
}

SEncodedAudioBuffer PcmAudioEncoder::swapBuffer16(const SAudioBuffer &buffer, const QString &codec)
{
  const SAudioCodec dstCodec(codec,
                             buffer.format().channelSetup(),
                             buffer.format().sampleRate());

  SEncodedAudioBuffer dstBuffer(dstCodec, buffer.size());
  dstBuffer.resize(buffer.size());

  LXiStream_Common_PcmAudio_swap16(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

SEncodedAudioBuffer PcmAudioEncoder::swapBuffer32(const SAudioBuffer &buffer, const QString &codec)
{
  const SAudioCodec dstCodec(codec,
                             buffer.format().channelSetup(),
                             buffer.format().sampleRate());

  SEncodedAudioBuffer dstBuffer(dstCodec, buffer.size());
  dstBuffer.resize(buffer.size());

  LXiStream_Common_PcmAudio_swap32(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

SEncodedAudioBuffer PcmAudioEncoder::swapBuffer64(const SAudioBuffer &buffer, const QString &codec)
{
  const SAudioCodec dstCodec(codec,
                             buffer.format().channelSetup(),
                             buffer.format().sampleRate());

  SEncodedAudioBuffer dstBuffer(dstCodec, buffer.size());
  dstBuffer.resize(buffer.size());

  LXiStream_Common_PcmAudio_swap64(dstBuffer.data(), buffer.data(), dstBuffer.size());

  return dstBuffer;
}

} } // End of namespaces
