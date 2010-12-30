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

#include "module.h"
#include "audioresampler.h"
#include "deinterlace.h"
#include "formatprober.h"
#include "pcmaudiodecoder.h"
#include "pcmaudioencoder.h"
#include "psbufferreader.h"
#include "psbufferwriter.h"
#include "rawsubtitledecoder.h"

namespace LXiStream {
namespace Common {


void Module::registerClasses(void)
{
  AudioResampler::registerClass<AudioResampler>(SFactory::Scheme(-1, "linear"));

  // Ensure static initializers have been initialized.
  FormatProber::audioSuffixes();
  FormatProber::videoSuffixes();
  FormatProber::imageSuffixes();
  FormatProber::rawImageSuffixes();
  FormatProber::registerClass<FormatProber>(1);

  // Codecs
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/S16LE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/S16BE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/U16LE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/U16BE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/S32LE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/S32BE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/U32LE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/U32BE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/F32LE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/F32BE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/F64LE"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "PCM/F64BE"));

  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/S16LE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/S16BE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/U16LE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/U16BE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/S32LE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/S32BE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/U32LE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/U32BE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/F32LE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/F32BE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/F64LE"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "PCM/F64BE"));

  RawSubtitleDecoder::registerClass<RawSubtitleDecoder>(SFactory::Scheme(1, "SUB/RAWUTF8"));
  RawSubtitleDecoder::registerClass<RawSubtitleDecoder>(SFactory::Scheme(1, "SUB/RAWLATIN1"));

  // Buffer readers and writers
  //PsBufferReader::registerClass<PsBufferReader>(PsBufferWriter::formatName);
  //PsBufferWriter::registerClass<PsBufferWriter>(PsBufferWriter::formatName);

  DeinterlaceBlend::registerClass<DeinterlaceBlend>(SFactory::Scheme(0, "blend"));
  DeinterlaceBob::registerClass<DeinterlaceBob>(SFactory::Scheme(-1, "bob"));
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return QByteArray();
}


} } // End of namespaces
