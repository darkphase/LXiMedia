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
#include "audiodecoder.h"
#include "audioencoder.h"
#include "audioresampler.h"
#include "bufferreader.h"
#include "bufferwriter.h"
#include "datadecoder.h"
#include "formatprober.h"
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

  // Format prober
  FormatProber::registerClass<FormatProber>(-1); // A very expensive prober, try a cheaper one first.

  // Codecs
  const QSet<QString> buggyDecoders = QSet<QString>()
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
      << "FLAC" << "THEORA"
#endif
      ;

  const QSet<QString> buggyEncoders = QSet<QString>()
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
      << "DNXHD" << "DVVIDEO" << "FLAC" << "HUFFYUV" << "JPEGLS" << "MSMPEG4V1"
      << "PAM" << "PNG" << "QTRLE" << "RAWVIDEO" << "ROQ" << "RV10" << "RV20"
      << "SGI" << "SNOW" << "SVQ1" << "TARGA" << "THEORA" << "TIFF" << "ZLIB"
#endif
      ;

  for (::AVCodec *codec=::av_codec_next(NULL); codec; codec=::av_codec_next(codec))
  {
    const char * const name = FFMpegCommon::fromFFMpegCodecID(codec->id);
    if (name)
    {
      if (codec->type == CODEC_TYPE_AUDIO)
      {
        if (codec->decode && !buggyDecoders.contains(name))
          AudioDecoder::registerClass<AudioDecoder>(name);

        if (codec->encode && !buggyEncoders.contains(name))
          AudioEncoder::registerClass<AudioEncoder>(name);
      }
      else if (codec->type == CODEC_TYPE_VIDEO)
      {
        if (codec->decode && !buggyDecoders.contains(name))
          VideoDecoder::registerClass<VideoDecoder>(name);

        if (codec->encode && !buggyEncoders.contains(name))
          VideoEncoder::registerClass<VideoEncoder>(name);
      }
      else if (codec->type == CODEC_TYPE_SUBTITLE)
      {
        if (codec->decode && !buggyDecoders.contains(name))
          DataDecoder::registerClass<DataDecoder>(name);
      }
    }
  }

  // Formats
  const QSet<QString> buggyOutFormats = QSet<QString>()
      << "mpegts" << "asf"
      ;

  for (::AVInputFormat *format=::av_iformat_next(NULL); format; format=::av_iformat_next(format))
    BufferReader::registerClass<BufferReader>(format->name);

  for (::AVOutputFormat *format=::av_oformat_next(NULL); format; format=::av_oformat_next(format))
  if (!buggyOutFormats.contains(format->name))
    BufferWriter::registerClass<BufferWriter>(format->name);

  // Filters
  AudioResampler::registerClass<AudioResampler>("fir");
  VideoResizer::registerClass<VideoResizer>("lanczos");
  VideoResizer::registerClass<VideoResizer>("bicubic");
  VideoResizer::registerClass<VideoResizer>("bilinear");

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
  const QByteArray text =
      " <h2>FFMpeg (libavcodec, libavformat, libswscale)</h2>\n"
      " Versions: " LIBAVCODEC_IDENT ", " LIBAVFORMAT_IDENT ", " LIBSWSCALE_IDENT "<br />\n"
      " Website: <a href=\"http://www.ffmpeg.org/\">www.ffmpeg.org</a><br />\n"
      " <br />\n"
      " Used under the terms of the GNU Lesser General Public License version 2.1\n"
      " as published by the Free Software Foundation.<br />\n"
      " <br />\n";

  return text;
}

} } // End of namespaces

#include <QtPlugin>
Q_EXPORT_PLUGIN2(lxistream_ffmpeg, LXiStream::FFMpegBackend::Module);
