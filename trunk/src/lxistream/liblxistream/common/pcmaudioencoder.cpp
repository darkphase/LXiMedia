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

#include "pcmaudioencoder.h"

namespace LXiStream {
namespace Common {

PcmAudioEncoder::PcmAudioEncoder(const QString &, QObject *parent)
  : SInterfaces::AudioEncoder(parent),
    formatConvert(NULL)
{
}

PcmAudioEncoder::~PcmAudioEncoder()
{
}

bool PcmAudioEncoder::openCodec(const SAudioCodec &c, SInterfaces::BufferWriter *, Flags)
{
  if (c.name().startsWith("pcm_"))
  {
    outCodec = c;

    if (c.name() == "pcm_S8")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S8);
    else if (c.name() == "pcm_u8")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U8);
    else if (c.name() == "pcm_s16le")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S16LE);
    else if (c.name() == "pcm_s16be")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S16BE);
    else if (c.name() == "pcm_u16le")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U16LE);
    else if (c.name() == "pcm_u16be")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U16BE);

    else if (c.name() == "pcm_s32le")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S32LE);
    else if (c.name() == "pcm_s32be")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S32BE);
    else if (c.name() == "pcm_u32le")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U32LE);
    else if (c.name() == "pcm_u32be")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U32BE);

    else if (c.name() == "pcm_f32le")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_F32LE);
    else if (c.name() == "pcm_f32be")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_F32BE);

    else if (c.name() == "pcm_f64le")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_F64LE);
    else if (c.name() == "pcm_f64be")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_F64BE);
  }

  return formatConvert.destFormat() != SAudioFormat::Format_Invalid;
}

SAudioCodec PcmAudioEncoder::codec(void) const
{
  return outCodec;
}

SEncodedAudioBufferList PcmAudioEncoder::encodeBuffer(const SAudioBuffer &audioBuffer)
{
  if ((formatConvert.destFormat() != SAudioFormat::Format_Invalid) && !audioBuffer.isNull())
  {
    const SAudioBuffer destBuffer =
        (audioBuffer.format().format() != formatConvert.destFormat())
          ? formatConvert.convert(audioBuffer)
          : audioBuffer;

    if (!destBuffer.isNull())
    {
      SEncodedAudioBuffer encodedBuffer(
          SAudioCodec(
              outCodec.name(),
              destBuffer.format().channelSetup(),
              destBuffer.format().sampleRate()),
          destBuffer.memory());

      encodedBuffer.setPresentationTimeStamp(destBuffer.timeStamp());

      return SEncodedAudioBufferList() << encodedBuffer;
    }
  }

  return SEncodedAudioBufferList();
}

} } // End of namespaces
