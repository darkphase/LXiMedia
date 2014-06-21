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
      ;

  const QSet<QByteArray> unsupportedEncoders = QSet<QByteArray>()
      // Audio
      << "libgsm" << "libgsm_ms" << "nellymoser" << "real_144"
//      << "wmav1" << "wmav2"
      // Video
      << "bmp" << "pam" << "pcx" << "png" << "qtrle" << "snow" << "rawvideo"
      << "roq" << "targa" << "tiff"
      ;

  for (::AVCodec *codec=::av_codec_next(NULL); codec; codec=::av_codec_next(codec))
  {
    if (codec->type == AVMEDIA_TYPE_AUDIO)
    {
      if (codec->decode && !unsupportedDecoders.contains(codec->name))
        AudioDecoder::registerClass<AudioDecoder>(codec->name);

      if (codec->encode && !unsupportedEncoders.contains(codec->name))
        AudioEncoder::registerClass<AudioEncoder>(codec->name);
    }
    else if (codec->type == AVMEDIA_TYPE_VIDEO)
    {
      if (codec->decode && !unsupportedDecoders.contains(codec->name))
        VideoDecoder::registerClass<VideoDecoder>(codec->name);

      if (codec->encode && !unsupportedEncoders.contains(codec->name))
        VideoEncoder::registerClass<VideoEncoder>(codec->name);
    }
    else if (codec->type == AVMEDIA_TYPE_SUBTITLE)
    {
      if (codec->decode && !unsupportedDecoders.contains(codec->name))
        DataDecoder::registerClass<DataDecoder>(codec->name);
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

  void *opaque = NULL;
  for (const char *protocol = avio_enum_protocols(&opaque, 0); protocol; protocol = avio_enum_protocols(&opaque, 0))
    NetworkBufferReader::registerClass<NetworkBufferReader>(protocol);

  // Filters
  AudioResampler::registerClass<AudioResampler>("fir");
  VideoResizer::registerClass<VideoResizer>("lanczos");
  VideoResizer::registerClass<VideoResizer>("bicubic");
  VideoResizer::registerClass<VideoResizer>("bilinear");
  VideoResizer::registerClass<VideoResizer>("deinterlace");

  // Format converters
  FFMpegCommon::disableLog(true);

  static const int formats[] =
  {
    PIX_FMT_YUV420P, PIX_FMT_YUV440P, PIX_FMT_YUV422P, PIX_FMT_YUV444P, PIX_FMT_YUV410P, PIX_FMT_YUV411P,
    PIX_FMT_YUVJ420P, PIX_FMT_YUVJ440P, PIX_FMT_YUVJ422P, PIX_FMT_YUVJ444P,
    PIX_FMT_YUYV422, PIX_FMT_UYVY422, PIX_FMT_UYYVYY411,
    PIX_FMT_ARGB, PIX_FMT_RGBA, PIX_FMT_ABGR, PIX_FMT_BGRA
  };

  for (unsigned srcFormat = 0; srcFormat < (sizeof(formats) / sizeof(*formats)); srcFormat++)
  for (unsigned dstFormat = 0; dstFormat < (sizeof(formats) / sizeof(*formats)); dstFormat++)
  if ((srcFormat != dstFormat) && VideoFormatConverter::testformat(::PixelFormat(formats[srcFormat]), ::PixelFormat(formats[dstFormat])))
  {
    VideoFormatConverter::registerClass<VideoFormatConverter>(
        FFMpegCommon::fromFFMpegPixelFormat(::PixelFormat(formats[srcFormat])),
        FFMpegCommon::fromFFMpegPixelFormat(::PixelFormat(formats[dstFormat])),
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
      " <p>Used under the terms of the GNU General Public License version 3\n"
      " as published by the Free Software Foundation.</p>\n";

  return text;
}

} } // End of namespaces
