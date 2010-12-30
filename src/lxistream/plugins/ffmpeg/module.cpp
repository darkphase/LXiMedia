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
#include "videoresizer.h"


namespace LXiStream {
namespace FFMpegBackend {


void Module::registerClasses(void)
{
  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

#ifdef QT_NO_DEBUG
  FFMpegCommon::init();
#else
  FFMpegCommon::init(true); // Verbose output
#endif

  // Format prober
  FormatProber::registerClass<FormatProber>(0);

  // Codecs
  for (::AVCodec *codec=::av_codec_next(NULL); codec; codec=::av_codec_next(codec))
  {
    const char * const name = FFMpegCommon::fromFFMpegCodecID(codec->id);
    if (name)
    {
      if (codec->type == CODEC_TYPE_AUDIO)
      {
        if (codec->decode)
          AudioDecoder::registerClass<AudioDecoder>(name);

        if (codec->encode)
          AudioEncoder::registerClass<AudioEncoder>(name);
      }
      else if (codec->type == CODEC_TYPE_VIDEO)
      {
        if (codec->decode)
          VideoDecoder::registerClass<VideoDecoder>(name);

        if (codec->encode)
          VideoEncoder::registerClass<VideoEncoder>(name);
      }
      else if (codec->type == CODEC_TYPE_SUBTITLE)
      {
        if (codec->decode)
          DataDecoder::registerClass<DataDecoder>(name);
      }
    }
  }

  // Formats
  for (::AVInputFormat *format=::av_iformat_next(NULL); format; format=::av_iformat_next(format))
    BufferReader::registerClass<BufferReader>(format->name);

  for (::AVOutputFormat *format=::av_oformat_next(NULL); format; format=::av_oformat_next(format))
    BufferWriter::registerClass<BufferWriter>(format->name);

  // Filters
  AudioResampler::registerClass<AudioResampler>("fir");
  VideoResizer::registerClass<VideoResizer>("lanczos");
  VideoResizer::registerClass<VideoResizer>("bicubic");
  VideoResizer::registerClass<VideoResizer>("bilinear");
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

#ifdef PLUGIN_NAME
#include <QtPlugin>
Q_EXPORT_PLUGIN2(PLUGIN_NAME, LXiStream::FFMpegBackend::Module);
#endif
