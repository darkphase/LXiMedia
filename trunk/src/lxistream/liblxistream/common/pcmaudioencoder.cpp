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
  if (c.codec().startsWith("PCM/"))
  {
    outCodec = c;

    if (c.codec() == "PCM/S8")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S8);
    else if (c.codec() == "PCM/U8")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U8);
    else if (c.codec() == "PCM/S16LE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S16LE);
    else if (c.codec() == "PCM/S16BE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S16BE);
    else if (c.codec() == "PCM/U16LE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U16LE);
    else if (c.codec() == "PCM/U16BE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U16BE);

    else if (c.codec() == "PCM/S32LE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S32LE);
    else if (c.codec() == "PCM/S32BE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_S32BE);
    else if (c.codec() == "PCM/U32LE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U32LE);
    else if (c.codec() == "PCM/U32BE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_U32BE);

    else if (c.codec() == "PCM/F32LE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_F32LE);
    else if (c.codec() == "PCM/F32BE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_F32BE);

    else if (c.codec() == "PCM/F64LE")
      formatConvert.setDestFormat(SAudioFormat::Format_PCM_F64LE);
    else if (c.codec() == "PCM/F64BE")
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
              outCodec.codec(),
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
