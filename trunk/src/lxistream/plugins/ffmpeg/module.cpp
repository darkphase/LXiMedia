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

#include "module.h"
#include "audiodecoder.h"
#include "audioencoder.h"
#include "audioresampler.h"
#include "bufferreader.h"
#include "bufferwriter.h"
#include "datadecoder.h"
#include "formatprober.h"
#include "networkbufferreader.h"
#include "videodecoder.h"
#include "videoencoder.h"
#include "videoformatconverter.h"
#include "videoresizer.h"


namespace LXiStream {
namespace FFMpegBackend {

bool Module::registerClasses(void)
{
#ifdef QT_NO_DEBUG
  FFMpegCommon::init();
#else
  FFMpegCommon::init(true); // Verbose output
#endif

  // Static initializers
  FFMpegCommon::isAudioFormat(QByteArray());
  FFMpegCommon::isVideoFormat(QByteArray());

  // Format prober
  FormatProber::registerClass<FormatProber>(-1); // A expensive prober, try a cheaper one first.

  // Codecs
  const QSet<QByteArray> unsupportedDecoders = QSet<QByteArray>()
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
      // Audio
      << "FLAC"
      // Video
      << "THEORA"
#endif
      ;

  const QSet<QByteArray> unsupportedEncoders = QSet<QByteArray>()
      // Audio
      << "GSM" << "GSM_MS" << "NELLYMOSER" << "PCM/S16LE" << "PCM/S16BE"
      << "PCM/U16LE" << "PCM/U16BE" << "PCM/MULAW" << "PCM/ALAW"
//      << "WMAV1" << "WMAV2"
      // Video
      << "BMP" << "DVVIDEO" << "DNXHD" << "HUFFYUV" << "JPEGLS" << "MSMPEG4V1"
      << "PAM" << "PCX" << "PNG" << "QTRLE" << "RAWVIDEO" << "ROQ" << "RV10"
      << "RV20" << "SGI" << "SNOW" << "SVQ1" << "TARGA" << "TIFF" << "V210"
      << "VP8" << "ZLIB"
#if (LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)) || defined(Q_OS_WIN)
      // Audio
      << "FLAC" << "VORBIS"
      // Video
      << "THEORA"
#endif
      ;

  for (::AVCodec *codec=::av_codec_next(NULL); codec; codec=::av_codec_next(codec))
  {
    const char * const name = FFMpegCommon::fromFFMpegCodecID(codec->id);
    if (name)
    {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
      if (codec->type == AVMEDIA_TYPE_AUDIO)
#else
      if (codec->type == CODEC_TYPE_AUDIO)
#endif
      {
        if (codec->decode && !unsupportedDecoders.contains(name))
          AudioDecoder::registerClass<AudioDecoder>(name);

        if (codec->encode && !unsupportedEncoders.contains(name))
          AudioEncoder::registerClass<AudioEncoder>(name);
      }
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
      else if (codec->type == AVMEDIA_TYPE_VIDEO)
#else
      else if (codec->type == CODEC_TYPE_VIDEO)
#endif
      {
        if (codec->decode && !unsupportedDecoders.contains(name))
          VideoDecoder::registerClass<VideoDecoder>(name);

        if (codec->encode && !unsupportedEncoders.contains(name))
          VideoEncoder::registerClass<VideoEncoder>(name);
      }
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
      else if (codec->type == AVMEDIA_TYPE_SUBTITLE)
#else
      else if (codec->type == CODEC_TYPE_SUBTITLE)
#endif
      {
        if (codec->decode && !unsupportedDecoders.contains(name))
          DataDecoder::registerClass<DataDecoder>(name);
      }
    }
  }

  // Formats
  const QSet<QByteArray> unsupportedReaders = QSet<QByteArray>()
      ;

  const QSet<QByteArray> unsupportedWriters = QSet<QByteArray>()
      ;

  for (::AVInputFormat *format=::av_iformat_next(NULL); format; format=::av_iformat_next(format))
    if (!unsupportedReaders.contains(format->name))
    BufferReader::registerClass<BufferReader>(format->name);

  for (::AVOutputFormat *format=::av_oformat_next(NULL); format; format=::av_oformat_next(format))
  if (!unsupportedWriters.contains(format->name))
    BufferWriter::registerClass<BufferWriter>(format->name);

  // Protocols
  const QSet<QByteArray> unsupportedProtocols = QSet<QByteArray>()
      ;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  void *opaque = NULL;
  for (const char *protocol = avio_enum_protocols(&opaque, 0); protocol; protocol = avio_enum_protocols(&opaque, 0))
    NetworkBufferReader::registerClass<NetworkBufferReader>(protocol);
#else
  for (::URLProtocol *protocol=::av_protocol_next(NULL); protocol; protocol=::av_protocol_next(protocol))
  if (!unsupportedProtocols.contains(protocol->name))
    NetworkBufferReader::registerClass<NetworkBufferReader>(protocol->name);
#endif

  // Filters
  AudioResampler::registerClass<AudioResampler>("fir");
  VideoResizer::registerClass<VideoResizer>("lanczos");
  VideoResizer::registerClass<VideoResizer>("bicubic");
  VideoResizer::registerClass<VideoResizer>("bilinear");
  VideoResizer::registerClass<VideoResizer>("deinterlace");

  // Format converters
  FFMpegCommon::disableLog(true);

  for (int srcFormat = PIX_FMT_NONE + 1; srcFormat < PIX_FMT_NB; srcFormat++)
  for (int dstFormat = PIX_FMT_NONE + 1; dstFormat < PIX_FMT_NB; dstFormat++)
    if ((srcFormat != dstFormat) && VideoFormatConverter::testformat(::PixelFormat(srcFormat), ::PixelFormat(dstFormat)))
  {
    VideoFormatConverter::registerClass<VideoFormatConverter>(
        FFMpegCommon::fromFFMpegPixelFormat(::PixelFormat(srcFormat)),
        FFMpegCommon::fromFFMpegPixelFormat(::PixelFormat(dstFormat)),
        -1);
  }

  FFMpegCommon::disableLog(false);

  return true;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return "FFMpeg plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  const QByteArray text =
      " <h3>FFMpeg (libavcodec, libavformat, libswscale)</h3>\n"
      " <p>Versions: " LIBAVCODEC_IDENT ", " LIBAVFORMAT_IDENT ", " LIBSWSCALE_IDENT "</p>\n"
      " <p>Website: <a href=\"http://www.ffmpeg.org/\">www.ffmpeg.org</a></p>\n"
      " <p>Used under the terms of the GNU Lesser General Public License version 2.1\n"
      " as published by the Free Software Foundation.</p>\n";

  return text;
}

} } // End of namespaces
