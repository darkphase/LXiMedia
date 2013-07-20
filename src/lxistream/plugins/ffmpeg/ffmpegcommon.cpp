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

#include "ffmpegcommon.h"
#include <cstring>
#include <QtConcurrent>

#ifdef __MMX__
#include <mmintrin.h>
#endif

namespace LXiStream {
namespace FFMpegBackend {

int   FFMpegCommon::logLevel = AV_LOG_QUIET;
bool  FFMpegCommon::logDisabled = false;

void FFMpegCommon::init(bool verbose)
{
  static bool initialized = false;

  if (SBuffer::numPaddingBytes < FF_INPUT_BUFFER_PADDING_SIZE)
    qFatal("The number of padding bytes (%d) is not sufficient for FFMpeg (>= %d)", SBuffer::numPaddingBytes, FF_INPUT_BUFFER_PADDING_SIZE);

  if (verbose)
    logLevel = AV_LOG_VERBOSE;
  else
    logLevel = AV_LOG_QUIET;

  if (!initialized)
  {
    initialized = true;

    ::av_log_set_callback(&FFMpegCommon::log);
    ::av_lockmgr_register(&FFMpegCommon::lock);
    ::av_register_all();
  }
}

void FFMpegCommon::disableLog(bool disabled)
{
  logDisabled = disabled;
}

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
::AVSampleFormat FFMpegCommon::toFFMpegSampleFormat(SAudioFormat::Format format)
{
  switch (format)
  {
  case SAudioFormat::Format_PCM_U8:       return ::AV_SAMPLE_FMT_U8;
  case SAudioFormat::Format_PCM_S16:      return ::AV_SAMPLE_FMT_S16;
  case SAudioFormat::Format_PCM_S32:      return ::AV_SAMPLE_FMT_S32;
  case SAudioFormat::Format_PCM_F32:      return ::AV_SAMPLE_FMT_FLT;
  case SAudioFormat::Format_PCM_F64:      return ::AV_SAMPLE_FMT_DBL;
  default:                                return ::AV_SAMPLE_FMT_NONE;
  }
}
#else
::SampleFormat FFMpegCommon::toFFMpegSampleFormat(SAudioFormat::Format format)
{
  switch (format)
  {
  case SAudioFormat::Format_PCM_U8:       return ::SAMPLE_FMT_U8;
  case SAudioFormat::Format_PCM_S16:      return ::SAMPLE_FMT_S16;
  case SAudioFormat::Format_PCM_S32:      return ::SAMPLE_FMT_S32;
  case SAudioFormat::Format_PCM_F32:      return ::SAMPLE_FMT_FLT;
  case SAudioFormat::Format_PCM_F64:      return ::SAMPLE_FMT_DBL;
  default:                                return ::SAMPLE_FMT_NONE;
  }
}
#endif

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
SAudioFormat::Format FFMpegCommon::fromFFMpegSampleFormat(::AVSampleFormat sf)
{
  switch (sf)
  {
  default:
  case ::AV_SAMPLE_FMT_NONE:  return SAudioFormat::Format_Invalid;
  case ::AV_SAMPLE_FMT_U8:    return SAudioFormat::Format_PCM_U8;
  case ::AV_SAMPLE_FMT_S16:   return SAudioFormat::Format_PCM_S16;
  case ::AV_SAMPLE_FMT_S32:   return SAudioFormat::Format_PCM_S32;
  case ::AV_SAMPLE_FMT_FLT:   return SAudioFormat::Format_PCM_F32;
  case ::AV_SAMPLE_FMT_DBL:   return SAudioFormat::Format_PCM_F64;
  }
}
#else
SAudioFormat::Format FFMpegCommon::fromFFMpegSampleFormat(::SampleFormat sf)
{
  switch (sf)
  {
  default:
  case ::SAMPLE_FMT_NONE:     return SAudioFormat::Format_Invalid;
  case ::SAMPLE_FMT_U8:       return SAudioFormat::Format_PCM_U8;
  case ::SAMPLE_FMT_S16:      return SAudioFormat::Format_PCM_S16;
  case ::SAMPLE_FMT_S32:      return SAudioFormat::Format_PCM_S32;
  case ::SAMPLE_FMT_FLT:      return SAudioFormat::Format_PCM_F32;
  case ::SAMPLE_FMT_DBL:      return SAudioFormat::Format_PCM_F64;
  }
}
#endif

::PixelFormat FFMpegCommon::toFFMpegPixelFormat(SVideoFormat::Format format)
{
  switch (format)
  {
  case SVideoFormat::Format_RGB555:       return PIX_FMT_RGB555;
  case SVideoFormat::Format_BGR555:       return PIX_FMT_BGR555;
  case SVideoFormat::Format_RGB565:       return PIX_FMT_RGB565;
  case SVideoFormat::Format_BGR565:       return PIX_FMT_BGR565;
  case SVideoFormat::Format_RGB24:        return PIX_FMT_RGB24;
  case SVideoFormat::Format_BGR24:        return PIX_FMT_BGR24;
  case SVideoFormat::Format_RGB32:        return PIX_FMT_RGB32;
  case SVideoFormat::Format_BGR32:        return PIX_FMT_BGR32;
  case SVideoFormat::Format_GRAY8:        return PIX_FMT_GRAY8;
  case SVideoFormat::Format_GRAY16BE:     return PIX_FMT_GRAY16BE;
  case SVideoFormat::Format_GRAY16LE:     return PIX_FMT_GRAY16LE;
  case SVideoFormat::Format_YUYV422:      return PIX_FMT_YUYV422;
  case SVideoFormat::Format_UYVY422:      return PIX_FMT_UYVY422;
  case SVideoFormat::Format_YUV410P:      return PIX_FMT_YUV410P;
  case SVideoFormat::Format_YUV411P:      return PIX_FMT_YUV411P;
  case SVideoFormat::Format_YUV420P:      return PIX_FMT_YUV420P;
  case SVideoFormat::Format_YUV422P:      return PIX_FMT_YUV422P;
  case SVideoFormat::Format_YUV444P:      return PIX_FMT_YUV444P;
  default:                                return PIX_FMT_NONE;
  }
}

SVideoFormat::Format FFMpegCommon::fromFFMpegPixelFormat(::PixelFormat pf)
{
  switch (pf)
  {
  default:
  case PIX_FMT_NONE:         return SVideoFormat::Format_Invalid;

  case PIX_FMT_RGB555:       return SVideoFormat::Format_RGB555;
  case PIX_FMT_BGR555:       return SVideoFormat::Format_BGR555;
  case PIX_FMT_RGB565:       return SVideoFormat::Format_RGB565;
  case PIX_FMT_BGR565:       return SVideoFormat::Format_BGR565;
  case PIX_FMT_RGB24:        return SVideoFormat::Format_RGB24;
  case PIX_FMT_BGR24:        return SVideoFormat::Format_BGR24;
  case PIX_FMT_RGB32:        return SVideoFormat::Format_RGB32;
  case PIX_FMT_BGR32:        return SVideoFormat::Format_BGR32;
  case PIX_FMT_GRAY8:        return SVideoFormat::Format_GRAY8;
  case PIX_FMT_GRAY16BE:     return SVideoFormat::Format_GRAY16BE;
  case PIX_FMT_GRAY16LE:     return SVideoFormat::Format_GRAY16LE;
  case PIX_FMT_YUYV422:      return SVideoFormat::Format_YUYV422;
  case PIX_FMT_UYVY422:      return SVideoFormat::Format_UYVY422;
  case PIX_FMT_YUV410P:      return SVideoFormat::Format_YUV410P;
  case PIX_FMT_YUV411P:      return SVideoFormat::Format_YUV411P;
  case PIX_FMT_YUV420P:
  case PIX_FMT_YUVJ420P:     return SVideoFormat::Format_YUV420P;
  case PIX_FMT_YUV422P:
  case PIX_FMT_YUVJ422P:     return SVideoFormat::Format_YUV422P;
  case PIX_FMT_YUV444P:
  case PIX_FMT_YUVJ444P:     return SVideoFormat::Format_YUV444P;
  }
}

int64_t FFMpegCommon::toFFMpegChannelLayout(SAudioFormat::Channels channels)
{
  switch (channels)
  {
  case SAudioFormat::Channels_Mono:             return CH_LAYOUT_MONO;
  case SAudioFormat::Channels_Stereo:           return CH_LAYOUT_STEREO;
  case SAudioFormat::Channels_Quadraphonic:     return CH_LAYOUT_QUAD;
  case SAudioFormat::Channels_Surround_3_0:     return CH_LAYOUT_SURROUND;
#ifdef CH_LAYOUT_4POINT0
  case SAudioFormat::Channels_Surround_4_0:     return CH_LAYOUT_4POINT0;
#endif
  case SAudioFormat::Channels_Surround_5_0:     return CH_LAYOUT_5POINT0;
  case SAudioFormat::Channels_Surround_5_1:     return CH_LAYOUT_5POINT1;
#ifdef CH_LAYOUT_5POINT0_BACK
  case SAudioFormat::Channels_Surround_6_0:     return CH_LAYOUT_5POINT0_BACK;
#endif
#ifdef CH_LAYOUT_5POINT1_BACK
  case SAudioFormat::Channels_Surround_6_1:     return CH_LAYOUT_5POINT1_BACK;
#endif
  case SAudioFormat::Channels_Surround_7_1:     return CH_LAYOUT_7POINT1;
  case SAudioFormat::Channels_Surround_7_1_Wide:return CH_LAYOUT_7POINT1_WIDE;
  default:
    {
      int64_t result = 0;

      if ((channels & SAudioFormat::Channel_LeftFront) != 0)                result |= CH_FRONT_LEFT;
      if ((channels & SAudioFormat::Channel_RightFront) != 0)               result |= CH_FRONT_RIGHT;
      if ((channels & SAudioFormat::Channel_CenterFront) != 0)              result |= CH_FRONT_CENTER;
      if ((channels & SAudioFormat::Channel_LowFrequencyEffects) != 0)      result |= CH_LOW_FREQUENCY;
      if ((channels & SAudioFormat::Channel_LeftBack) != 0)                 result |= CH_BACK_LEFT;
      if ((channels & SAudioFormat::Channel_RightBack) != 0)                result |= CH_BACK_RIGHT;
      if ((channels & SAudioFormat::Channel_CenterLeftFront) != 0)          result |= CH_FRONT_LEFT_OF_CENTER;
      if ((channels & SAudioFormat::Channel_CenterRightFront) != 0)         result |= CH_FRONT_RIGHT_OF_CENTER;
      if ((channels & SAudioFormat::Channel_CenterBack) != 0)               result |= CH_BACK_CENTER;
      if ((channels & SAudioFormat::Channel_LeftSide) != 0)                 result |= CH_SIDE_LEFT;
      if ((channels & SAudioFormat::Channel_RightSide) != 0)                result |= CH_SIDE_RIGHT;
      if ((channels & SAudioFormat::Channel_TopLeftFront) != 0)             result |= CH_TOP_FRONT_LEFT;
      if ((channels & SAudioFormat::Channel_TopCenterFront) != 0)           result |= CH_TOP_FRONT_CENTER;
      if ((channels & SAudioFormat::Channel_TopRightFront) != 0)            result |= CH_TOP_FRONT_RIGHT;
      if ((channels & SAudioFormat::Channel_TopLeftBack) != 0)              result |= CH_TOP_BACK_LEFT;
      if ((channels & SAudioFormat::Channel_TopCenterBack) != 0)            result |= CH_TOP_BACK_CENTER;
      if ((channels & SAudioFormat::Channel_TopRightBack) != 0)             result |= CH_TOP_BACK_RIGHT;

      return result;
    }
  }
}

SAudioFormat::Channels FFMpegCommon::fromFFMpegChannelLayout(int64_t layout, int channels)
{
#ifndef CH_LAYOUT_4POINT0
#define CH_LAYOUT_4POINT0 (CH_LAYOUT_SURROUND|CH_BACK_CENTER)
#endif
#ifndef CH_LAYOUT_5POINT0_BACK
#define CH_LAYOUT_5POINT0_BACK (CH_LAYOUT_SURROUND|CH_BACK_LEFT|CH_BACK_RIGHT)
#endif
#ifndef CH_LAYOUT_5POINT1_BACK
#define CH_LAYOUT_5POINT1_BACK (CH_LAYOUT_5POINT0_BACK|CH_LOW_FREQUENCY)
#endif
#ifndef CH_LAYOUT_6POINT0
#define CH_LAYOUT_6POINT0 (CH_LAYOUT_5POINT0|CH_BACK_CENTER)
#endif
#ifndef CH_LAYOUT_6POINT1
#define CH_LAYOUT_6POINT1 (CH_LAYOUT_5POINT1|CH_BACK_CENTER)
#endif

  switch (layout)
  {
  case CH_LAYOUT_MONO:              return SAudioFormat::Channels_Mono;
  case CH_LAYOUT_STEREO:            return SAudioFormat::Channels_Stereo;
  case CH_LAYOUT_QUAD:              return SAudioFormat::Channels_Quadraphonic;
  case CH_LAYOUT_SURROUND:          return SAudioFormat::Channels_Surround_3_0;
  case CH_LAYOUT_4POINT0:           return SAudioFormat::Channels_Surround_4_0;
  case CH_LAYOUT_5POINT0:           return SAudioFormat::Channels_Surround_5_0;
  case CH_LAYOUT_5POINT0_BACK:      return SAudioFormat::Channels_Surround_5_0;
  case CH_LAYOUT_5POINT1:           return SAudioFormat::Channels_Surround_5_1;
  case CH_LAYOUT_5POINT1_BACK:      return SAudioFormat::Channels_Surround_5_1;
  case CH_LAYOUT_6POINT0:           return SAudioFormat::Channels_Surround_6_0;
  case CH_LAYOUT_6POINT1:           return SAudioFormat::Channels_Surround_6_1;
  case CH_LAYOUT_7POINT1:           return SAudioFormat::Channels_Surround_7_1;
  case CH_LAYOUT_7POINT1_WIDE:      return SAudioFormat::Channels_Surround_7_1_Wide;
  case 0:                           return SAudioFormat::guessChannels(channels);
  default:
    {
      SAudioFormat::Channels result = 0;

      if ((layout & CH_FRONT_LEFT) != 0)                result |= SAudioFormat::Channel_LeftFront;
      if ((layout & CH_FRONT_RIGHT) != 0)               result |= SAudioFormat::Channel_RightFront;
      if ((layout & CH_FRONT_CENTER) != 0)              result |= SAudioFormat::Channel_CenterFront;
      if ((layout & CH_LOW_FREQUENCY) != 0)             result |= SAudioFormat::Channel_LowFrequencyEffects;
      if ((layout & CH_BACK_LEFT) != 0)                 result |= SAudioFormat::Channel_LeftBack;
      if ((layout & CH_BACK_RIGHT) != 0)                result |= SAudioFormat::Channel_RightBack;
      if ((layout & CH_FRONT_LEFT_OF_CENTER) != 0)      result |= SAudioFormat::Channel_CenterLeftFront;
      if ((layout & CH_FRONT_RIGHT_OF_CENTER) != 0)     result |= SAudioFormat::Channel_CenterRightFront;
      if ((layout & CH_BACK_CENTER) != 0)               result |= SAudioFormat::Channel_CenterBack;
      if ((layout & CH_SIDE_LEFT) != 0)                 result |= SAudioFormat::Channel_LeftSide;
      if ((layout & CH_SIDE_RIGHT) != 0)                result |= SAudioFormat::Channel_RightSide;
      if ((layout & CH_TOP_FRONT_LEFT) != 0)            result |= SAudioFormat::Channel_TopLeftFront;
      if ((layout & CH_TOP_FRONT_CENTER) != 0)          result |= SAudioFormat::Channel_TopCenterFront;
      if ((layout & CH_TOP_FRONT_RIGHT) != 0)           result |= SAudioFormat::Channel_TopRightFront;
      if ((layout & CH_TOP_BACK_LEFT) != 0)             result |= SAudioFormat::Channel_TopLeftBack;
      if ((layout & CH_TOP_BACK_CENTER) != 0)           result |= SAudioFormat::Channel_TopCenterBack;
      if ((layout & CH_TOP_BACK_RIGHT) != 0)            result |= SAudioFormat::Channel_TopRightBack;

      return result;
    }
  }
}

bool FFMpegCommon::isAudioFormat(const QByteArray &name)
{
  static const QSet<QByteArray> audioFormats = QSet<QByteArray>() <<
      "aac" <<
      "ac3" <<
      "adts" <<
      "aea" <<
      "aiff" <<
      "alaw" <<
      "alsa" <<
      "ape" <<
      "au" <<
      "caf" <<
      "daud" <<
      "dts" <<
      "eac3" <<
      "f32be" <<
      "f32le" <<
      "f64be" <<
      "f64le" <<
      "flac" <<
      "g722" <<
      "gsm" <<
      "jack" <<
      "mmf" <<
      "mp2" <<
      "mp3" <<
      "mpc" <<
      "mpc8" <<
      "mulaw" <<
      "nsv" <<
      "ogg" <<
      "oma" <<
      "oss" <<
      "s16be" <<
      "s16le" <<
      "s24be" <<
      "s24le" <<
      "s32be" <<
      "s32le" <<
      "s8" <<
      "spdif" <<
      "truehd" <<
      "tta" <<
      "u16be" <<
      "u16le" <<
      "u24be" <<
      "u24le" <<
      "u32be" <<
      "u32le" <<
      "u8" <<
      "voc" <<
      "vqf" <<
      "w64" <<
      "wav" <<
      "wv" <<
      "xwma";

  return audioFormats.contains(name.toLower());
}

bool FFMpegCommon::isVideoFormat(const QByteArray &name)
{
  static const QSet<QByteArray> videoFormats = QSet<QByteArray>() <<
      "3g2" <<
      "3gp" <<
      "asf" <<
      "asf_stream" <<
      "avi" <<
      "cavsvideo" <<
      "dirac" <<
      "dnxhd" <<
      "dv" <<
      "dv1394" <<
      "dvd" <<
      "fbdev" <<
      "filmstrip" <<
      "flv" <<
      "gxf" <<
      "h261" <<
      "h263" <<
      "h264" <<
      "ingenient" <<
      "ivf" <<
      "libdc1394" <<
      "m4v" <<
      "matroska" <<
      "matroska,webm" <<
      "mjpeg" <<
      "mov" <<
      "mov,mp4,m4a,3gp,3g2,mj2" <<
      "mp4" <<
      "mpeg" <<
      "mpeg1video" <<
      "mpeg2video" <<
      "mpegts" <<
      "mpegtsraw" <<
      "mpegvideo" <<
      "mpjpeg" <<
      "null" <<
      "nut" <<
      "nuv" <<
      "rawvideo" <<
      "smjpeg" <<
      "smk" <<
      "svcd" <<
      "swf" <<
      "thp" <<
      "vc1" <<
      "vc1test" <<
      "vcd" <<
      "video4linux2" <<
      "vob" <<
      "webm" <<
      "xmv";

  return videoFormats.contains(name.toLower());
}

::AVPacket FFMpegCommon::toAVPacket(const SEncodedAudioBuffer &buffer, ::AVStream *stream)
{
  ::AVPacket packet;
  ::av_init_packet(&packet);

  packet.data = (uint8_t *)buffer.data();
  packet.size = buffer.size();

  if (stream)
  {
    packet.stream_index = stream->index;

    if (buffer.presentationTimeStamp().isValid())
      packet.pts = buffer.presentationTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.pts = AV_NOPTS_VALUE;

    if (buffer.decodingTimeStamp().isValid())
      packet.dts = buffer.decodingTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.dts = packet.pts;

    if ((packet.pts != AV_NOPTS_VALUE) && (packet.dts != AV_NOPTS_VALUE) && (packet.dts > packet.pts))
      packet.pts = packet.dts;

    stream->pts.val = packet.pts;

    packet.duration = buffer.duration().toClock(stream->time_base.num, stream->time_base.den);
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  packet.flags |= AV_PKT_FLAG_KEY;
#else
  packet.flags |= PKT_FLAG_KEY;
#endif

  return packet;
}

::AVPacket FFMpegCommon::toAVPacket(const SEncodedVideoBuffer &buffer, ::AVStream *stream)
{
  ::AVPacket packet;
  ::av_init_packet(&packet);

  packet.data = (uint8_t *)buffer.data();
  packet.size = buffer.size();

  if (stream)
  {
    packet.stream_index = stream->index;

    if (buffer.presentationTimeStamp().isValid())
      packet.pts = buffer.presentationTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.pts = AV_NOPTS_VALUE;

    if (buffer.decodingTimeStamp().isValid())
      packet.dts = buffer.decodingTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.dts = packet.pts;

    if ((packet.pts != AV_NOPTS_VALUE) && (packet.dts != AV_NOPTS_VALUE) && (packet.dts > packet.pts))
      packet.pts = packet.dts;

    stream->pts.val = packet.pts;

    packet.duration = buffer.duration().toClock(stream->time_base.num, stream->time_base.den);
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  packet.flags |= buffer.isKeyFrame() ? AV_PKT_FLAG_KEY : 0;
#else
  packet.flags |= buffer.isKeyFrame() ? PKT_FLAG_KEY : 0;
#endif

  return packet;
}

::AVPacket FFMpegCommon::toAVPacket(const SEncodedDataBuffer &buffer, ::AVStream *stream)
{
  ::AVPacket packet;
  ::av_init_packet(&packet);

  packet.data = (uint8_t *)buffer.data();
  packet.size = buffer.size();

  if (stream)
  {
    packet.stream_index = stream->index;

    if (buffer.presentationTimeStamp().isValid())
      packet.pts = buffer.presentationTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.pts = AV_NOPTS_VALUE;

    if (buffer.decodingTimeStamp().isValid())
      packet.dts = buffer.decodingTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.dts = packet.pts;

    if (buffer.duration().isValid())
      packet.duration = buffer.duration().toClock(stream->time_base.num, stream->time_base.den);

    if ((packet.pts != AV_NOPTS_VALUE) && (packet.dts != AV_NOPTS_VALUE) && (packet.dts > packet.pts))
      packet.pts = packet.dts;

    stream->pts.val = packet.pts;

    packet.convergence_duration = buffer.duration().toClock(stream->time_base.num, stream->time_base.den);
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  packet.flags |= AV_PKT_FLAG_KEY;
#else
  packet.flags |= PKT_FLAG_KEY;
#endif

  return packet;
}

#ifdef OPT_ENABLE_THREADS
int FFMpegCommon::encodeThreadCount(::CodecID codec)
{
  int limit;
  switch (codec)
  {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  case CODEC_ID_MPEG2VIDEO:
  case CODEC_ID_H264:
  case CODEC_ID_THEORA:
    limit = threadLimit;
    break;
#endif

  default:
    limit = 1;
    break;
  }

  return qBound(1, QThreadPool::globalInstance()->maxThreadCount(), limit);
}

int FFMpegCommon::decodeThreadCount(::CodecID codec)
{
  int limit;
  switch (codec)
  {
  case CODEC_ID_H263:
  case CODEC_ID_H264:
  case CODEC_ID_THEORA:
  case CODEC_ID_FFH264:
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  case CODEC_ID_MPEG1VIDEO:
  case CODEC_ID_MPEG4:
  case CODEC_ID_MSMPEG4V1:
  case CODEC_ID_MSMPEG4V2:
  case CODEC_ID_MSMPEG4V3:
  case CODEC_ID_WMV1:
  case CODEC_ID_WMV2:
  case CODEC_ID_WMV3:
#endif
    limit = threadLimit;
    break;

  default:
    limit = 1;
    break;
  }

  return qBound(1, QThreadPool::globalInstance()->maxThreadCount(), limit);
}

int FFMpegCommon::execute(::AVCodecContext *c, int (*func)(::AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size)
{
  struct T
  {
    static int run(int (*func)(::AVCodecContext *c, void *arg), ::AVCodecContext *c, void *arg)
    {
      LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);

      const int result = func(c, arg);

#ifdef __MMX__
      // Prevent floating point errors.
      _mm_empty();
#endif

      return result;
    }
  };

  if ((count > 1) && (c->thread_count > 1))
  {
    QVector< QFuture<int> > future;
    future.reserve(count);

    for (int i=0; i<count; i++)
      future += QtConcurrent::run(&T::run, func, c, reinterpret_cast<char *>(arg2) + (size * i));

    for (int i=0; i<count; i++)
    {
      future[i].waitForFinished();
      if (ret)
        ret[i] = future[i].result();
    }
  }
  else // Single-threaded.
  {
    for (int i=0; i<count; i++)
    {
      if (ret)
        ret[i] = func(c, arg2);
      else
        func(c, arg2);
    }
  }

  return 0;
}

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
int FFMpegCommon::execute2(::AVCodecContext *c, int (*func)(::AVCodecContext *c2, void *arg, int jobnr, int threadnr), void *arg2, int *ret, int count)
{
  struct T
  {
    static int run(int (*func)(::AVCodecContext *c, void *arg, int jobnr, int threadnr), ::AVCodecContext *c, void *arg, int jobnr, int threadnr)
    {
      LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);

      const int result = func(c, arg, jobnr, threadnr);

#ifdef __MMX__
      // Prevent floating point errors.
      _mm_empty();
#endif

      return result;
    }
  };

  if ((count > 1) && (c->thread_count > 1))
  {
    QVector< QFuture<int> > future;
    future.reserve(c->thread_count);

    for (int i=0; i<count; i+=c->thread_count)
    {
      for (int j=0; (j<c->thread_count) && ((i+j)<count); j++)
        future += QtConcurrent::run(&T::run, func, c, arg2, i, j);

      for (int j=0; j<future.count(); j++)
      {
        future[j].waitForFinished();
        if (ret)
          ret[i+j] = future[j].result();
      }

      future.clear();
    }
  }
  else // Single-threaded.
  {
    for (int i=0; i<count; i++)
    {
      if (ret)
        ret[i] = func(c, arg2, i, 0);
      else
        func(c, arg2, i, 0);
    }
  }

  return 0;
}
#endif
#endif

void FFMpegCommon::log(void *, int level, const char *fmt, va_list vl)
{
  if ((level <= logLevel) && !logDisabled)
  {
    //const ::AVClass * const * const c = reinterpret_cast<const ::AVClass * const *>(ptr);

    char buffer[4096]; buffer[sizeof(buffer) - 1] = '\0';
    ::vsnprintf(buffer, sizeof(buffer) - 1, fmt, vl);

    // Trim trailing whitespace.
    for (int i=::strlen(buffer); i>=0; i--)
    if ((buffer[i] >= '\0') && (buffer[i] <= ' '))
      buffer[i] = '\0';
    else
      break;

    if (level >= AV_LOG_INFO)
      qDebug("FFMpeg info: %s", buffer);
    else if (level >= AV_LOG_FATAL)
      qDebug("FFMpeg fatal: %s", buffer);
#ifdef AV_LOG_PANIC
    else if (level >= AV_LOG_PANIC)
      qFatal("FFMpeg panic: %s", buffer);
#endif
    else
      qDebug("FFMpeg debug: %s", buffer);
  }
}

int FFMpegCommon::lock(void **mutex, AVLockOp op)
{
  switch (op)
  {
  case AV_LOCK_CREATE:
    *mutex = new QMutex();
    return 0;

  case AV_LOCK_OBTAIN:
    static_cast<QMutex *>(*mutex)->lock();
    return 0;

  case AV_LOCK_RELEASE:
    static_cast<QMutex *>(*mutex)->unlock();
    return 0;

  case AV_LOCK_DESTROY:
    delete static_cast<QMutex *>(*mutex);
    return 0;
  }

  return -1;
}


FFMpegCommon::SyncMemory::SyncMemory(int capacity, QSemaphore *semaphore)
  : SBuffer::Memory(capacity),
    semaphore(semaphore)
{
  if (semaphore)
    semaphore->acquire();
}

FFMpegCommon::SyncMemory::~SyncMemory()
{
  if (semaphore)
    semaphore->release();
}

} } // End of namespaces
