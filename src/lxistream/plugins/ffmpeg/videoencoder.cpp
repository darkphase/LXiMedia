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

#include "videoencoder.h"
#include "bufferwriter.h"
#include <cmath>

namespace LXiStream {
namespace FFMpegBackend {

VideoEncoder::VideoEncoder(const QString &, QObject *parent)
  : SInterfaces::VideoEncoder(parent),
    outCodec(),
    codecDict(NULL),
    codecHandle(NULL),
    contextHandle(NULL),
    contextHandleOwner(false),
    pictureHandle(NULL),
    pictureBuffer(),
    formatConvert(NULL),
    inputTimeStamps(),
    lastSubStreamId(0),
    memorySem(memorySemCount),
    fastEncode(false),
    noDelay(false),
    enableWait(false),
    bufferSize(FF_MIN_BUFFER_SIZE)
{
}

VideoEncoder::~VideoEncoder()
{
  encodeFuture.waitForFinished();
  delayedResult.clear();
  memorySem.acquire(memorySemCount);

  if (contextHandle && contextHandleOwner)
  {
    if (codecHandle)
      ::avcodec_close(contextHandle);

    ::av_free(contextHandle);
  }

  if (codecDict)
    ::av_dict_free(&codecDict);

  if (pictureHandle)
    ::av_free(pictureHandle);
}

const ::AVCodecContext * VideoEncoder::avCodecContext(void) const
{
  enableWait = true;

  return contextHandle;
}

bool VideoEncoder::openCodec(const SVideoCodec &c, SInterfaces::BufferWriter *bufferWriter, Flags flags)
{
  if (contextHandle)
    qFatal("VideoEncoder already opened a codec.");

  outCodec = c;

  if ((codecHandle = ::avcodec_find_encoder_by_name(outCodec.name())) == NULL)
  {
    qCritical() << "VideoEncoder: Video codec not found " << outCodec.name();
    return false;
  }

  BufferWriter * const ffBufferWriter = qobject_cast<BufferWriter *>(bufferWriter);
  if (ffBufferWriter)
  {
    contextHandle = ffBufferWriter->createStream(codecHandle)->codec;
    contextHandleOwner = false;
  }
  else
  {
    contextHandle = ::avcodec_alloc_context3(codecHandle);
    contextHandleOwner = true;
  }

  contextHandle->pix_fmt = ::PIX_FMT_NONE;

  if (codecHandle->pix_fmts)
  for (const ::PixelFormat *f=codecHandle->pix_fmts; *f != ::PIX_FMT_NONE; f++)
  if (FFMpegCommon::fromFFMpegPixelFormat(*f) != SVideoFormat::Format_Invalid)
  {
    contextHandle->pix_fmt = *f;
    break;
  }

  if (contextHandle->pix_fmt == ::PIX_FMT_NONE)
    contextHandle->pix_fmt = ::PIX_FMT_YUV420P;

  contextHandle->width = outCodec.size().width();
  contextHandle->height = outCodec.size().height();
  contextHandle->sample_aspect_ratio = ::av_d2q(outCodec.size().aspectRatio(), 256);
  contextHandle->ticks_per_frame = 1;
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
  }

  if (flags & Flag_Fast)
  {
    contextHandle->rc_max_rate += contextHandle->bit_rate_tolerance / 2;
    contextHandle->bit_rate += contextHandle->bit_rate_tolerance / 2;
    contextHandle->gop_size = 0;
    contextHandle->max_b_frames = 0;
    contextHandle->strict_std_compliance = FF_COMPLIANCE_UNOFFICIAL;

    contextHandle->flags2 |= CODEC_FLAG2_FAST;

    fastEncode = true;
  }
  else if (outCodec.gopSize() >= 0)
    contextHandle->gop_size = outCodec.gopSize();

  if (flags & Flag_Slideshow)
  {
    contextHandle->me_method = 1;
    contextHandle->max_b_frames = 0;
    contextHandle->gop_size = (outCodec.gopSize() >= 0) ? outCodec.gopSize() : 100;
  }

  if (flags & Flag_NoDelay)
  {
    contextHandle->max_b_frames = 0;
    noDelay = true;
  }
  else
    noDelay = QThreadPool::globalInstance()->maxThreadCount() <= 1;

#ifdef OPT_ENABLE_THREADS
  contextHandle->thread_count = FFMpegCommon::encodeThreadCount(codecHandle->id);
  contextHandle->execute = &FFMpegCommon::execute;
  contextHandle->execute2 = &FFMpegCommon::execute2;
#endif

  if (ffBufferWriter)
  if (ffBufferWriter->avFormat()->flags & AVFMT_GLOBALHEADER)
    contextHandle->flags |= CODEC_FLAG_GLOBAL_HEADER;

  // Determine rate-control buffer size.
  if (contextHandle->rc_max_rate > 0)
  {
    contextHandle->rc_buffer_size = qBound(262144, (contextHandle->rc_max_rate / 500000) * 65536, 4194304);
    contextHandle->rc_initial_buffer_occupancy = contextHandle->rc_buffer_size * 3 / 4;
  }

  if (avcodec_open2(contextHandle, codecHandle, &codecDict) < 0)
  {
    qCritical() << "VideoEncoder: Could not open video codec " << codecHandle->name;
    codecHandle = NULL;
    return false;
  }

  frameTime = STime(contextHandle->ticks_per_frame, SInterval(contextHandle->time_base.num, contextHandle->time_base.den));

  formatConvert.setDestFormat(FFMpegCommon::fromFFMpegPixelFormat(contextHandle->pix_fmt));

  // Determine intermediate buffer size.
  pictureHandle = avcodec_alloc_frame();
  avcodec_get_frame_defaults(pictureHandle);

  outCodec.setBitRate(contextHandle->bit_rate + contextHandle->bit_rate_tolerance);
  outCodec.setGopSize(contextHandle->gop_size);

  bufferSize = qMax((contextHandle->width * contextHandle->height * 4) + 8192, FF_MIN_BUFFER_SIZE);

  return true;
}

SVideoCodec VideoEncoder::codec(void) const
{
  return outCodec;
}

SEncodedVideoBufferList VideoEncoder::encodeBuffer(const SVideoBuffer &videoBuffer)
{
  SEncodedVideoBufferList output;

  if (!noDelay)
  {
    LXI_PROFILE_WAIT(encodeFuture.waitForFinished());

    output = delayedResult;
    delayedResult.clear();

    if (!videoBuffer.isNull())
      encodeFuture = QtConcurrent::run(this, &VideoEncoder::encodeBufferTask, videoBuffer, &delayedResult, enableWait);
    else // Flush
      encodeBufferTask(videoBuffer, &output, false);
  }
  else
    encodeBufferTask(videoBuffer, &output, false);

  return output;
}

void VideoEncoder::encodeBufferTask(const SVideoBuffer &videoBuffer, SEncodedVideoBufferList *output, bool wait)
{
  // We need to wait until all BufferWriters have written the previous buffer as
  // they may share the AVCodecContext.
  if (wait)
  {
    LXI_PROFILE_WAIT(memorySem.acquire(memorySemCount));
    memorySem.release(memorySemCount);
  }

  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  if (!videoBuffer.isNull() && codecHandle)
  {
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

      SBuffer outBuffer(SBuffer::MemoryPtr(
          new FFMpegCommon::SyncMemory(bufferSize, wait ? &memorySem : NULL)));

      int out_size = avcodec_encode_video(contextHandle,
                                          (uint8_t *)outBuffer.data(),
                                          outBuffer.capacity(),
                                          pictureHandle);
      if (out_size > 0)
      {
        outBuffer.resize(out_size);

        SEncodedVideoBuffer destBuffer(outCodec, outBuffer.memory());
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

        destBuffer.setDuration(frameTime);

//        qDebug() << "Output timestamp" << fastEncode
//            << "dts =" << destBuffer.decodingTimeStamp().toMSec() << inputTimeStamps.count()
//            << ", pts =" << destBuffer.presentationTimeStamp().toMSec()
//            << ", size =" << out_size
//            << ", duration =" << destBuffer.duration().toMSec()
//            << ", key =" << destBuffer.isKeyFrame();

        output->append(destBuffer);
      }
    }
  }
  else if (codecHandle)
  {
    for (int out_size=1; out_size > 0;)
    {
      // Get any remaining frames
      SBuffer outBuffer(SBuffer::MemoryPtr(
          new FFMpegCommon::SyncMemory(bufferSize, wait ? &memorySem : NULL)));

      out_size = avcodec_encode_video(contextHandle,
                                      (uint8_t *)outBuffer.data(),
                                      outBuffer.capacity(),
                                      NULL);
      if ((out_size > 0) && !inputTimeStamps.isEmpty())
      {
        const STime dts = inputTimeStamps.takeFirst();

        outBuffer.resize(out_size);

        SEncodedVideoBuffer destBuffer(outCodec, outBuffer.memory());
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

        destBuffer.setDuration(frameTime);

        output->append(destBuffer);
      }
    }

    output->append(SEncodedVideoBuffer()); // Flush
  }
}

} } // End of namespaces
