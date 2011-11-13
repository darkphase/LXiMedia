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
  if (c.codec().startsWith("PCM/"))
  {
    if (c.codec() == "PCM/S8")
      format = SAudioFormat::Format_PCM_S8;
    else if (c.codec() == "PCM/U8")
      format = SAudioFormat::Format_PCM_U8;
    else if (c.codec() == "PCM/S16LE")
      format = SAudioFormat::Format_PCM_S16LE;
    else if (c.codec() == "PCM/S16BE")
      format = SAudioFormat::Format_PCM_S16BE;
    else if (c.codec() == "PCM/U16LE")
      format = SAudioFormat::Format_PCM_U16LE;
    else if (c.codec() == "PCM/U16BE")
      format = SAudioFormat::Format_PCM_U16BE;

    else if (c.codec() == "PCM/S32LE")
      format = SAudioFormat::Format_PCM_S32LE;
    else if (c.codec() == "PCM/S32BE")
      format = SAudioFormat::Format_PCM_S32BE;
    else if (c.codec() == "PCM/U32LE")
      format = SAudioFormat::Format_PCM_U32LE;
    else if (c.codec() == "PCM/U32BE")
      format = SAudioFormat::Format_PCM_U32BE;

    else if (c.codec() == "PCM/F32LE")
      format = SAudioFormat::Format_PCM_F32LE;
    else if (c.codec() == "PCM/F32BE")
      format = SAudioFormat::Format_PCM_F32BE;

    else if (c.codec() == "PCM/F64LE")
      format = SAudioFormat::Format_PCM_F64LE;
    else if (c.codec() == "PCM/F64BE")
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
