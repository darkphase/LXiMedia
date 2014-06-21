/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include "module.h"
#include "audioformatconverter.h"
#include "audioresampler.h"
#include "deinterlace.h"
#include "formatprober.h"
#include "localfilesystem.h"
#include "pcmaudiodecoder.h"
#include "pcmaudioencoder.h"
#include "psbufferreader.h"
#include "psbufferwriter.h"
#include "rawsubtitledecoder.h"
#include "subtitlereader.h"
#include "tsbufferwriter.h"
#include "videoformatconverter.h"

namespace LXiStream {
namespace Common {

bool Module::registerClasses(void)
{
  // Ensure static initializers have been initialized.
  FormatProber::audioSuffixes();
  FormatProber::videoSuffixes();
  FormatProber::imageSuffixes();
  FormatProber::subtitleSuffixes();
  FormatProber::registerClass<FormatProber>(INT_MAX); // This one always first.

  LocalFilesystem::registerClass<LocalFilesystem>(LocalFilesystem::scheme);
  AudioResampler::registerClass<AudioResampler>(SFactory::Scheme(-1, "linear"));

  // Codecs
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_s8"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_u8"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_s16le"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_s16be"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_u16le"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_u16be"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_s32le"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_s32be"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_u32le"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_u32be"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_f32le"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_f32be"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_f64le"));
  PcmAudioDecoder::registerClass<PcmAudioDecoder>(SFactory::Scheme(1, "pcm_f64be"));

  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_s8"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_s8"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_s16le"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_s16be"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_u16le"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_u16be"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_s32le"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_s32be"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_u32le"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_u32be"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_f32le"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_f32be"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_f64le"));
  PcmAudioEncoder::registerClass<PcmAudioEncoder>(SFactory::Scheme(1, "pcm_f64be"));

  RawSubtitleDecoder::registerClass<RawSubtitleDecoder>(SFactory::Scheme(1, "sub_rawutf8"));
  RawSubtitleDecoder::registerClass<RawSubtitleDecoder>(SFactory::Scheme(1, "sub_raw8bit"));

  // Buffer readers and writers
  //PsBufferReader::registerClass<PsBufferReader>(PsBufferWriter::formatName);
  //PsBufferWriter::registerClass<PsBufferWriter>(PsBufferWriter::formatName);
  SubtitleReader::registerClass<SubtitleReader>(SFactory::Scheme(1, "srt"));
  TsBufferWriter::registerClass<TsBufferWriter>(SFactory::Scheme(-1, "m2ts"));

  // Filters
  DeinterlaceBlend::registerClass<DeinterlaceBlend>(SFactory::Scheme(0, "blend"));
  DeinterlaceBob::registerClass<DeinterlaceBob>(SFactory::Scheme(-1, "bob"));

  // Audio format converters
  AudioFormatConverter_Format_PCM_S16LE_Format_PCM_S16BE::registerClass<AudioFormatConverter_Format_PCM_S16LE_Format_PCM_S16BE>();
  AudioFormatConverter_Format_PCM_S16BE_Format_PCM_S16LE::registerClass<AudioFormatConverter_Format_PCM_S16BE_Format_PCM_S16LE>();
  AudioFormatConverter_Format_PCM_U16LE_Format_PCM_U16BE::registerClass<AudioFormatConverter_Format_PCM_U16LE_Format_PCM_U16BE>();
  AudioFormatConverter_Format_PCM_U16BE_Format_PCM_U16LE::registerClass<AudioFormatConverter_Format_PCM_U16BE_Format_PCM_U16LE>();
  AudioFormatConverter_Format_PCM_S32LE_Format_PCM_S32BE::registerClass<AudioFormatConverter_Format_PCM_S32LE_Format_PCM_S32BE>();
  AudioFormatConverter_Format_PCM_S32BE_Format_PCM_S32LE::registerClass<AudioFormatConverter_Format_PCM_S32BE_Format_PCM_S32LE>();
  AudioFormatConverter_Format_PCM_U32LE_Format_PCM_U32BE::registerClass<AudioFormatConverter_Format_PCM_U32LE_Format_PCM_U32BE>();
  AudioFormatConverter_Format_PCM_U32BE_Format_PCM_U32LE::registerClass<AudioFormatConverter_Format_PCM_U32BE_Format_PCM_U32LE>();
  AudioFormatConverter_Format_PCM_F32LE_Format_PCM_F32BE::registerClass<AudioFormatConverter_Format_PCM_F32LE_Format_PCM_F32BE>();
  AudioFormatConverter_Format_PCM_F32BE_Format_PCM_F32LE::registerClass<AudioFormatConverter_Format_PCM_F32BE_Format_PCM_F32LE>();
  AudioFormatConverter_Format_PCM_F64LE_Format_PCM_F64BE::registerClass<AudioFormatConverter_Format_PCM_F64LE_Format_PCM_F64BE>();
  AudioFormatConverter_Format_PCM_F64BE_Format_PCM_F64LE::registerClass<AudioFormatConverter_Format_PCM_F64BE_Format_PCM_F64LE>();

  AudioFormatConverter_Format_PCM_S8_Format_PCM_S16::registerClass<AudioFormatConverter_Format_PCM_S8_Format_PCM_S16>();
  AudioFormatConverter_Format_PCM_U8_Format_PCM_U16::registerClass<AudioFormatConverter_Format_PCM_U8_Format_PCM_U16>();
  AudioFormatConverter_Format_PCM_S16_Format_PCM_S8::registerClass<AudioFormatConverter_Format_PCM_S16_Format_PCM_S8>();
  AudioFormatConverter_Format_PCM_U16_Format_PCM_U8::registerClass<AudioFormatConverter_Format_PCM_U16_Format_PCM_U8>();
  AudioFormatConverter_Format_PCM_S16_Format_PCM_S32::registerClass<AudioFormatConverter_Format_PCM_S16_Format_PCM_S32>();
  AudioFormatConverter_Format_PCM_U16_Format_PCM_U32::registerClass<AudioFormatConverter_Format_PCM_U16_Format_PCM_U32>();
  AudioFormatConverter_Format_PCM_S32_Format_PCM_S16::registerClass<AudioFormatConverter_Format_PCM_S32_Format_PCM_S16>();
  AudioFormatConverter_Format_PCM_U32_Format_PCM_U16::registerClass<AudioFormatConverter_Format_PCM_U32_Format_PCM_U16>();
  AudioFormatConverter_Format_PCM_S16_Format_PCM_F32::registerClass<AudioFormatConverter_Format_PCM_S16_Format_PCM_F32>();
  AudioFormatConverter_Format_PCM_F32_Format_PCM_S16::registerClass<AudioFormatConverter_Format_PCM_F32_Format_PCM_S16>();
  AudioFormatConverter_Format_PCM_S32_Format_PCM_F32::registerClass<AudioFormatConverter_Format_PCM_S32_Format_PCM_F32>();
  AudioFormatConverter_Format_PCM_F32_Format_PCM_S32::registerClass<AudioFormatConverter_Format_PCM_F32_Format_PCM_S32>();
  AudioFormatConverter_Format_PCM_S16_Format_PCM_F64::registerClass<AudioFormatConverter_Format_PCM_S16_Format_PCM_F64>();
  AudioFormatConverter_Format_PCM_F64_Format_PCM_S16::registerClass<AudioFormatConverter_Format_PCM_F64_Format_PCM_S16>();
  AudioFormatConverter_Format_PCM_S32_Format_PCM_F64::registerClass<AudioFormatConverter_Format_PCM_S32_Format_PCM_F64>();
  AudioFormatConverter_Format_PCM_F64_Format_PCM_S32::registerClass<AudioFormatConverter_Format_PCM_F64_Format_PCM_S32>();

  AudioFormatConverter_Format_PCM_U16_Format_PCM_S16::registerClass<AudioFormatConverter_Format_PCM_U16_Format_PCM_S16>();
  AudioFormatConverter_Format_PCM_S16_Format_PCM_U16::registerClass<AudioFormatConverter_Format_PCM_S16_Format_PCM_U16>();

  // Video format converters
  VideoFormatConverter_Format_YUYV422_Format_RGB32::registerClass<VideoFormatConverter_Format_YUYV422_Format_RGB32>();
  VideoFormatConverter_Format_RGB32_Format_YUYV422::registerClass<VideoFormatConverter_Format_RGB32_Format_YUYV422>();
  VideoFormatConverter_Format_UYVY422_Format_RGB32::registerClass<VideoFormatConverter_Format_UYVY422_Format_RGB32>();
  VideoFormatConverter_Format_RGB32_Format_UYVY422::registerClass<VideoFormatConverter_Format_RGB32_Format_UYVY422>();
  VideoFormatConverter_Format_BGR32_Format_RGB32::registerClass<VideoFormatConverter_Format_BGR32_Format_RGB32>();
  VideoFormatConverter_Format_RGB32_Format_BGR32::registerClass<VideoFormatConverter_Format_RGB32_Format_BGR32>();
  VideoFormatConverter_Format_YUV420P_Format_RGB32::registerClass<VideoFormatConverter_Format_YUV420P_Format_RGB32>();
  VideoFormatConverter_Format_RGB32_Format_YUV420P::registerClass<VideoFormatConverter_Format_RGB32_Format_YUV420P>();
  VideoFormatConverter_Format_YUV422P_Format_RGB32::registerClass<VideoFormatConverter_Format_YUV422P_Format_RGB32>();
  VideoFormatConverter_Format_RGB32_Format_YUV422P::registerClass<VideoFormatConverter_Format_RGB32_Format_YUV422P>();
  VideoFormatConverter_Format_YUV444P_Format_RGB32::registerClass<VideoFormatConverter_Format_YUV444P_Format_RGB32>();
  VideoFormatConverter_Format_RGB32_Format_YUV444P::registerClass<VideoFormatConverter_Format_RGB32_Format_YUV444P>();

  VideoFormatConverter_Format_GRBG8_Format_RGB32::registerClass<VideoFormatConverter_Format_GRBG8_Format_RGB32>();
  VideoFormatConverter_Format_GBRG8_Format_RGB32::registerClass<VideoFormatConverter_Format_GBRG8_Format_RGB32>();
  VideoFormatConverter_Format_RGGB8_Format_RGB32::registerClass<VideoFormatConverter_Format_RGGB8_Format_RGB32>();
  VideoFormatConverter_Format_BGGR8_Format_RGB32::registerClass<VideoFormatConverter_Format_BGGR8_Format_RGB32>();

  VideoFormatConverter_Format_YUYV422_Format_YUV420P::registerClass<VideoFormatConverter_Format_YUYV422_Format_YUV420P>();
  VideoFormatConverter_Format_UYVY422_Format_YUV420P::registerClass<VideoFormatConverter_Format_UYVY422_Format_YUV420P>();
  VideoFormatConverter_Format_YUYV422_Format_YUV422P::registerClass<VideoFormatConverter_Format_YUYV422_Format_YUV422P>();
  VideoFormatConverter_Format_UYVY422_Format_YUV422P::registerClass<VideoFormatConverter_Format_UYVY422_Format_YUV422P>();

  return true;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return QByteArray();
}

QByteArray Module::licenses(void)
{
  return QByteArray();
}

} } // End of namespaces
