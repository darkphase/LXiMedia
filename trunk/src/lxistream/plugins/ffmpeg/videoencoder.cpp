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

#include "videoencoder.h"
#include <cmath>

namespace LXiStream {
namespace FFMpegBackend {

const int VideoEncoder::bufferSize = qMax(FF_MIN_BUFFER_SIZE, 1048576);

VideoEncoder::VideoEncoder(const QString &, QObject *parent)
  : SInterfaces::VideoEncoder(parent),
    outCodec(),
    codecHandle(NULL),
    contextHandle(NULL),
    pictureHandle(NULL),
    pictureBuffer(),
    formatConvert(NULL),
    inputTimeStamps(),
    lastSubStreamId(0),
    fastEncode(false),
#ifdef OPT_RESEND_LAST_FRAME
    enableResend(false),
    lastInBufferId(0),
#endif
    outBuffer()
{
}

VideoEncoder::~VideoEncoder()
{
  if (codecHandle && contextHandle)
    avcodec_close(contextHandle);

  if (contextHandle)
    av_free(contextHandle);

  if (pictureHandle)
    av_free(pictureHandle);
}

bool VideoEncoder::openCodec(const SVideoCodec &c, Flags flags)
{
  if (contextHandle)
    qFatal("VideoEncoder already opened a codec.");

  outCodec = c;

  if ((codecHandle = avcodec_find_encoder(FFMpegCommon::toFFMpegCodecID(outCodec))) == NULL)
  {
    qCritical() << "VideoEncoder: Video codec not found " << outCodec.codec();
    return false;
  }

  contextHandle = ::avcodec_alloc_context();
  ::avcodec_get_context_defaults2(contextHandle, CODEC_TYPE_VIDEO);

  contextHandle->pix_fmt = PIX_FMT_YUV420P;
  contextHandle->width = outCodec.size().width();
  contextHandle->height = outCodec.size().height();
  contextHandle->sample_aspect_ratio = ::av_d2q(outCodec.size().aspectRatio(), 256);
  contextHandle->time_base.num = outCodec.frameRate().num();
  contextHandle->time_base.den = outCodec.frameRate().den();

  const int baseRate = ((int(sqrt(float(contextHandle->width * contextHandle->height))) + 249) / 250) * 250;
  if (flags & Flag_LowQuality)
    contextHandle->bit_rate = baseRate *  8000;
  else if (flags & Flag_HighQuality)
    contextHandle->bit_rate = baseRate * 32000;
  else // Normal quality
    contextHandle->bit_rate = baseRate * 16000;

  if (outCodec.bitRate() > 0)
    contextHandle->bit_rate = outCodec.bitRate();

  contextHandle->bit_rate_tolerance = contextHandle->bit_rate / 2;

  if (flags & Flag_HardBitrateLimit)
  {
    contextHandle->rc_max_rate = contextHandle->bit_rate;
    contextHandle->bit_rate -= contextHandle->bit_rate_tolerance;
    contextHandle->rc_buffer_size = ((20 * contextHandle->rc_max_rate) / (1151929 / 2)) * 8 * 1024;
    contextHandle->rc_initial_buffer_occupancy = contextHandle->rc_buffer_size * 3 / 4;
  }

  if (flags & Flag_Fast)
  {
    contextHandle->rc_max_rate += contextHandle->bit_rate_tolerance / 2;
    contextHandle->bit_rate += contextHandle->bit_rate_tolerance / 2;
    contextHandle->gop_size = (flags & Flag_Slideshow) ? 4 : 0;
    contextHandle->max_b_frames = 0;
    contextHandle->strict_std_compliance = FF_COMPLIANCE_INOFFICIAL;

    contextHandle->flags2 |= CODEC_FLAG2_FAST;

    fastEncode = true;

#ifdef OPT_RESEND_LAST_FRAME
    // Resending the last frame can only be done if the bitrate limit is not hard.
    if (((flags & Flag_HardBitrateLimit) == 0) && ((flags & Flag_Slideshow) == 0))
      enableResend = true;
#endif
  }

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
  // This circumvents a bug generating corrupt blocks in large areas with the same color.
  if (((outCodec == "MPEG1") || (outCodec == "MPEG2")) && !fastEncode)
    contextHandle->max_b_frames = 3;
#endif

  if (outCodec == "MJPEG")
    contextHandle->pix_fmt = PIX_FMT_YUVJ420P;

#ifdef OPT_ENABLE_THREADS
  contextHandle->thread_count = FFMpegCommon::encodeThreadCount(codecHandle->id);
  contextHandle->execute = &FFMpegCommon::execute;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  contextHandle->execute2 = &FFMpegCommon::execute2;
#endif
#endif

  if (avcodec_open(contextHandle, codecHandle) < 0)
  {
    qCritical() << "VideoEncoder: Could not open video codec " << codecHandle->name;
    codecHandle = NULL;
    return false;
  }

  formatConvert.setDestFormat(FFMpegCommon::fromFFMpegPixelFormat(contextHandle->pix_fmt));

  // Determine intermediate buffer size.
  pictureHandle = avcodec_alloc_frame();
  avcodec_get_frame_defaults(pictureHandle);

  outCodec.setBitRate(contextHandle->bit_rate + contextHandle->bit_rate_tolerance);

  if (contextHandle->extradata_size > 0)
    outCodec.setExtraData(QByteArray((const char *)contextHandle->extradata, contextHandle->extradata_size));

  outBuffer.resize(qMax(contextHandle->width * contextHandle->height * 4, 262144));

  return true;
}

SVideoCodec VideoEncoder::codec(void) const
{
  return outCodec;
}

SEncodedVideoBufferList VideoEncoder::encodeBuffer(const SVideoBuffer &videoBuffer)
{
  SEncodedVideoBufferList output;
  if (!videoBuffer.isNull() && codecHandle)
  {
#ifdef OPT_RESEND_LAST_FRAME
    // Simply return the previously encoded buffer if we're encoding fast (I
    // frames only) and the input buffer is the same as the output buffer.
    if (enableResend && (videoBuffer.memory()->uid == lastInBufferId) &&
        lastEncodedBuffer.isKeyFrame())
    {
      const STime frameTime = STime(1, SInterval(contextHandle->time_base.num, contextHandle->time_base.den));

      lastEncodedBuffer.setPresentationTimeStamp(lastEncodedBuffer.presentationTimeStamp() + frameTime);
      lastEncodedBuffer.setDecodingTimeStamp(lastEncodedBuffer.decodingTimeStamp() + frameTime);

      output << lastEncodedBuffer;
      return output;
    }
#endif

    const SVideoBuffer preprocBuffer = formatConvert.convert(videoBuffer);
    if (!preprocBuffer.isNull())
    {
      for (unsigned i=0; i<4; i++)
      {
        pictureHandle->data[i] = (uint8_t *)preprocBuffer.scanLine(0, i);
        pictureHandle->linesize[i] = preprocBuffer.lineSize(i);
      }

      if (preprocBuffer.format().fieldMode() == SVideoFormat::FieldMode_InterlacedTopFirst)
      {
        pictureHandle->interlaced_frame = 1;
        pictureHandle->top_field_first = 1;
      }
      else if (preprocBuffer.format().fieldMode() == SVideoFormat::FieldMode_InterlacedBottomFirst)
      {
        pictureHandle->interlaced_frame = 1;
        pictureHandle->top_field_first = 0;
      }

      inputTimeStamps.append(preprocBuffer.timeStamp());
      pictureHandle->pts = inputTimeStamps.last().toClock(contextHandle->time_base.num, contextHandle->time_base.den);

      int out_size = avcodec_encode_video(contextHandle,
                                          (uint8_t *)outBuffer.data(),
                                          outBuffer.size(),
                                          pictureHandle);
      if (out_size > 0)
      {
        SEncodedVideoBuffer destBuffer(outCodec, outBuffer.data(), out_size);
        destBuffer.setKeyFrame((!fastEncode && contextHandle->coded_frame)
                               ? bool(contextHandle->coded_frame->key_frame)
                               : true);

        STime dts = inputTimeStamps.takeFirst();
        if (!fastEncode && (contextHandle->coded_frame->pts != AV_NOPTS_VALUE))
        {
          const STime pts = STime::fromClock(contextHandle->coded_frame->pts,
                                             contextHandle->time_base.num,
                                             contextHandle->time_base.den);

          if ((contextHandle->max_b_frames > 0) && (contextHandle->gop_size > 0))
          {
            dts = qMax(STime::null,
                       dts - STime::fromClock(contextHandle->max_b_frames,
                                              contextHandle->time_base.num,
                                              contextHandle->time_base.den));
          }

          destBuffer.setPresentationTimeStamp(pts);
          if (pts < dts)
            destBuffer.setDecodingTimeStamp(pts);
          else
            destBuffer.setDecodingTimeStamp(dts);
        }
        else
        {
          destBuffer.setPresentationTimeStamp(dts);
          destBuffer.setDecodingTimeStamp(dts);
        }

  //      qDebug() << "Output timestamp" << fastEncode
  //          << "dts =" << destBuffer.decodingTimeStamp().toMSec() << inputTimeStamps.count()
  //          << ", pts =" << destBuffer.presentationTimeStamp().toMSec()
  //          << ", size =" << out_size;

  #ifdef OPT_RESEND_LAST_FRAME
        if (enableResend)
        {
          lastInBufferId = videoBuffer.memory()->uid;
          lastEncodedBuffer = destBuffer;
        }
  #endif

        output << destBuffer;
      }
    }
#ifdef OPT_RESEND_LAST_FRAME
    else if (enableResend)
    {
      lastInBufferId = 0;
      lastEncodedBuffer.clear();
    }
#endif
  }
  else if (codecHandle)
  {
    for (int out_size=1; out_size > 0;)
    {
      // Get any remaining frames
      out_size = avcodec_encode_video(contextHandle,
                                      (uint8_t *)outBuffer.data(),
                                      outBuffer.size(),
                                      NULL);
      if ((out_size > 0) && !inputTimeStamps.isEmpty())
      {
        const STime dts = inputTimeStamps.takeFirst();

        SEncodedVideoBuffer destBuffer(outCodec, outBuffer.data(), out_size);
        destBuffer.setKeyFrame((!fastEncode && contextHandle->coded_frame)
                               ? bool(contextHandle->coded_frame->key_frame)
                               : true);

        if (!fastEncode && (contextHandle->coded_frame->pts != AV_NOPTS_VALUE))
        {
          const STime pts = STime::fromClock(contextHandle->coded_frame->pts,
                                             contextHandle->time_base.num,
                                             contextHandle->time_base.den);

          destBuffer.setPresentationTimeStamp(pts);
          if (pts < dts)
            destBuffer.setDecodingTimeStamp(pts);
          else
            destBuffer.setDecodingTimeStamp(dts);
        }
        else
        {
          destBuffer.setPresentationTimeStamp(dts);
          destBuffer.setDecodingTimeStamp(dts);
        }

        output << destBuffer;
      }
    }

#ifdef OPT_RESEND_LAST_FRAME
    if (enableResend)
    {
      lastInBufferId = 0;
      lastEncodedBuffer.clear();
    }
#endif
  }

  return output;
}


} } // End of namespaces
