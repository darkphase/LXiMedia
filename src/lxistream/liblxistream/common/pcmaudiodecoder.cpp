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

#include "pcmaudiodecoder.h"

namespace LXiStream {
namespace Common {

PcmAudioDecoder::PcmAudioDecoder(const QString &, QObject *parent)
  : SInterfaces::AudioDecoder(parent),
    format(SAudioFormat::Format_Invalid),
    formatConvert(NULL),
    nextTimeStamp(STime::null)
{
}

PcmAudioDecoder::~PcmAudioDecoder()
{
}

bool PcmAudioDecoder::openCodec(const SAudioCodec &c, SInterfaces::AbstractBufferReader *, Flags)
{
  if (c.name().startsWith("pcm_"))
  {
    if (c.name() == "pcm_s8")
      format = SAudioFormat::Format_PCM_S8;
    else if (c.name() == "pcm_u8")
      format = SAudioFormat::Format_PCM_U8;
    else if (c.name() == "pcm_s16le")
      format = SAudioFormat::Format_PCM_S16LE;
    else if (c.name() == "pcm_s16be")
      format = SAudioFormat::Format_PCM_S16BE;
    else if (c.name() == "pcm_u16le")
      format = SAudioFormat::Format_PCM_U16LE;
    else if (c.name() == "pcm_u16be")
      format = SAudioFormat::Format_PCM_U16BE;

    else if (c.name() == "pcm_s32le")
      format = SAudioFormat::Format_PCM_S32LE;
    else if (c.name() == "pcm_s32be")
      format = SAudioFormat::Format_PCM_S32BE;
    else if (c.name() == "pcm_u32le")
      format = SAudioFormat::Format_PCM_U32LE;
    else if (c.name() == "pcm_u32be")
      format = SAudioFormat::Format_PCM_U32BE;

    else if (c.name() == "pcm_f32le")
      format = SAudioFormat::Format_PCM_F32LE;
    else if (c.name() == "pcm_f32be")
      format = SAudioFormat::Format_PCM_F32BE;

    else if (c.name() == "pcm_f64le")
      format = SAudioFormat::Format_PCM_F64LE;
    else if (c.name() == "pcm_f64be")
      format = SAudioFormat::Format_PCM_F64BE;

    formatConvert.setDestFormat(SAudioFormat::Format_PCM_S16);
  }

  return format != SAudioFormat::Format_Invalid;
}

SAudioBufferList PcmAudioDecoder::decodeBuffer(const SEncodedAudioBuffer &audioBuffer)
{
  if ((format != SAudioFormat::Format_Invalid) && !audioBuffer.isNull())
  {
    SAudioBuffer decodedBuffer(
        SAudioFormat(
            format,
            audioBuffer.codec().channelSetup(),
            audioBuffer.codec().sampleRate()),
        audioBuffer.memory());

    const STime pts = audioBuffer.presentationTimeStamp();
    if (pts.isValid())
    {
      decodedBuffer.setTimeStamp(pts);
    }
    else
    {
      const STime dts = audioBuffer.decodingTimeStamp();
      if (dts.isValid())
        decodedBuffer.setTimeStamp(dts);
      else
        decodedBuffer.setTimeStamp(nextTimeStamp);
    }

    nextTimeStamp = decodedBuffer.timeStamp() + decodedBuffer.duration();

    if (format != formatConvert.destFormat())
      return SAudioBufferList() << formatConvert.convert(decodedBuffer);
    else
      return SAudioBufferList() << decodedBuffer;
  }

  return SAudioBufferList();
}

} } // End of namespaces
