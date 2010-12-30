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

#include "videodecoder.h"

namespace LXiStream {
namespace FFMpegBackend {


VideoDecoder::VideoDecoder(const QString &, QObject *parent)
  : SInterfaces::VideoDecoder(parent),
    inCodec(),
    codecHandle(NULL),
    contextHandle(NULL),
    pictureHandle(NULL),
    outFormat(),
    timeStamp(STime::null)
{
}

VideoDecoder::~VideoDecoder()
{
  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

  if (codecHandle && contextHandle)
    avcodec_close(contextHandle);

  if (contextHandle)
  {
    if (contextHandle->extradata)
      delete [] contextHandle->extradata;

    av_free(contextHandle);
  }

  if (pictureHandle)
    av_free(pictureHandle);
}

bool VideoDecoder::openCodec(const SVideoCodec &c, Flags flags)
{
  if (contextHandle)
    qFatal("VideoDecoder already opened a codec.");

  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

  inCodec = c;
  timeStamp = STime::null;

  if ((codecHandle = avcodec_find_decoder(FFMpegCommon::toFFMpegCodecID(inCodec))) == NULL)
  {
    qWarning() << "VideoDecoder: Video codec not found " << inCodec.codec();
    return false;
  }

  pictureHandle = avcodec_alloc_frame();
  avcodec_get_frame_defaults(pictureHandle);

  contextHandle = avcodec_alloc_context();
#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
  contextHandle->error_recognition = FF_ER_COMPLIANT;
#endif
  contextHandle->error_concealment = FF_EC_DEBLOCK;
  contextHandle->flags2 |= CODEC_FLAG2_CHUNKS;

#ifdef FF_BUG_NO_PADDING
  if (codecHandle->id == CODEC_ID_MPEG4)
    contextHandle->workaround_bugs = FF_BUG_NO_PADDING;
#endif

  contextHandle->codec_type = CODEC_TYPE_VIDEO;
  contextHandle->width = 0;
  contextHandle->height = 0;
  contextHandle->coded_width = inCodec.size().width();
  contextHandle->coded_height = inCodec.size().height();
  contextHandle->time_base.num = inCodec.frameRate().num();
  contextHandle->time_base.den = inCodec.frameRate().den();

  if (flags & Flag_KeyframesOnly)
    contextHandle->skip_frame = AVDISCARD_NONKEY;
  else
    contextHandle->skip_frame = AVDISCARD_DEFAULT;

  contextHandle->bit_rate = inCodec.bitRate();

  contextHandle->extradata_size = inCodec.extraData().size();
  if (contextHandle->extradata_size > 0)
  {
    contextHandle->extradata = new uint8_t[contextHandle->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE];
    memcpy(contextHandle->extradata, inCodec.extraData().data(), contextHandle->extradata_size);
    memset(contextHandle->extradata + contextHandle->extradata_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
  }
  else
    contextHandle->extradata = NULL;

#if ((LIBAVCODEC_VERSION_INT >> 16) < 52)
  struct T
  {
    static int get_buffer(struct AVCodecContext *c, AVFrame *pic)
    {
      int ret = ::avcodec_default_get_buffer(c, pic);

      if (c->opaque)
        pic->opaque = new qint64(*reinterpret_cast<const qint64 *>(c->opaque));

      return ret;
    }

    static void release_buffer(struct AVCodecContext *c, AVFrame *pic)
    {
      if (pic)
      if (pic->opaque)
        delete reinterpret_cast<qint64 *>(pic->opaque);

      ::avcodec_default_release_buffer(c, pic);
    }
  };

  contextHandle->get_buffer = &T::get_buffer;
  contextHandle->release_buffer = &T::release_buffer;
#endif

  if (avcodec_open(contextHandle, codecHandle) < 0)
  {
    qCritical() << "VideoDecoder: Could not open video codec " << inCodec.codec();
    codecHandle = NULL;
    return false;
  }

//    packet.pts = AV_NOPTS_VALUE;
//    packet.dts = AV_NOPTS_VALUE;
//    packet.data = NULL;
//    packet.size = 0;
//    packet.stream_index = 0;
//    packet.flags = 0;
//    packet.duration = 0;
//    packet.destruct = NULL;
//    packet.priv = NULL;
//    packet.pos = -1;
//    packet.convergence_duration = AV_NOPTS_VALUE;

  return true;
}

SVideoBufferList VideoDecoder::decodeBuffer(const SEncodedVideoBuffer &videoBuffer)
{
  SVideoBufferList output;
  if (!videoBuffer.isNull() && codecHandle)
  {
    const qint64 inputPts =
        videoBuffer.presentationTimeStamp().isValid()
        ? videoBuffer.presentationTimeStamp().toClock(contextHandle->time_base.num, contextHandle->time_base.den)
        : AV_NOPTS_VALUE;

#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
    contextHandle->reordered_opaque = inputPts;
#else
    contextHandle->opaque = new qint64(inputPts);
#endif

    size_t sourceSize = videoBuffer.size();
    uint8_t *sourcePtr = (uint8_t *)videoBuffer.data();
    while (sourceSize > 0)
    {
      avcodec_get_frame_defaults(pictureHandle);

      //packet.data = sourcePtr;
      //packet.size = sourceSize;

      int gotPicture = 0;
      //const int len = avcodec_decode_video2(contextHandle, pictureHandle, &gotPicture, &packet);
      const int len = avcodec_decode_video(contextHandle, pictureHandle, &gotPicture, sourcePtr, sourceSize);
      if ((len >= 0) && gotPicture)
      {
        // Determine metadata.
        SVideoFormat::Format format = FFMpegCommon::fromFFMpegPixelFormat(contextHandle->pix_fmt);

        SSize size;
        if ((contextHandle->sample_aspect_ratio.den == 0) || (contextHandle->sample_aspect_ratio.num == 0))
          size = SSize(contextHandle->width, contextHandle->height);
        else
          size = SSize(contextHandle->width, contextHandle->height, ::av_q2d(contextHandle->sample_aspect_ratio));

        const SInterval frameRate = videoBuffer.codec().frameRate();

        SVideoFormat::FieldMode fieldMode = SVideoFormat::FieldMode_Progressive;
        if (pictureHandle->interlaced_frame != 0)
        {
          fieldMode = (pictureHandle->top_field_first != 0)
                        ? SVideoFormat::FieldMode_InterlacedTopFirst
                        : SVideoFormat::FieldMode_InterlacedBottomFirst;
        }

        if ((outFormat.format() != format) || (outFormat.size() != size) ||
            !qFuzzyCompare(outFormat.frameRate().toFrequency(), frameRate.toFrequency()) ||
            (outFormat.fieldMode() != fieldMode))
        {
          outFormat = SVideoFormat(format, size, frameRate, fieldMode);

          if (outFormat.numPlanes() > 1)
          {
            int wr, hr;
            outFormat.planarYUVRatio(wr, hr);
            outNumLines[0] = contextHandle->height;
            outNumLines[1] = contextHandle->height / hr;
            outNumLines[2] = contextHandle->height / hr;
          }
          else
          {
            outNumLines[0] = contextHandle->height;
            outNumLines[1] = 0;
            outNumLines[2] = 0;
          }
        }

        // Fill destination buffer.
        SVideoBuffer destBuffer(outFormat);
        for (int i=0, n=outFormat.numPlanes(); i<n; i++)
        for (int y=0; y<outNumLines[i]; y++)
        {
          memcpy(destBuffer.scanLine(y, i),
                 pictureHandle->data[i] + (pictureHandle->linesize[i] * y),
                 qMin(destBuffer.lineSize(i), pictureHandle->linesize[i]));
        }

        // Determine correct timestamp
#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
        const STime rpts =
            (pictureHandle->reordered_opaque != AV_NOPTS_VALUE)
            ? STime::fromClock(pictureHandle->reordered_opaque, contextHandle->time_base.num, contextHandle->time_base.den)
            : STime();
#else
        const STime rpts =
            (*reinterpret_cast<const qint64 *>(pictureHandle->opaque) != AV_NOPTS_VALUE)
            ? STime::fromClock(*reinterpret_cast<const qint64 *>(pictureHandle->opaque), contextHandle->time_base.num, contextHandle->time_base.den)
            : STime();
#endif

        STime ts;
        if (rpts.isValid())
          ts = rpts;
        else if (videoBuffer.isKeyFrame() && videoBuffer.decodingTimeStamp().isValid())
          ts = videoBuffer.decodingTimeStamp();
        else 
          ts = timeStamp;

        // Correct invalid timestamps
        const STime delta = qAbs(timeStamp - ts);
        if (!videoBuffer.isKeyFrame() && (delta <= STime(contextHandle->gop_size, frameRate)))
          ts = timeStamp;
        else if (videoBuffer.isKeyFrame() && (delta <= STime(2, frameRate)))
          ts = timeStamp;

/*
        // This might give a better PTS.
        if (pictureHandle->pts > 0)
        {
          const STime framePts = STime::fromClock(pictureHandle->pts, contextHandle->time_base.num, contextHandle->time_base.den);
          if (qAbs(ts - framePts) < STime::fromSec(1))
            ts = framePts;
        }
*/

//        static STime lastTs = STime::fromSec(0);
//        qDebug() << "Video timestamp" << ts.toMSec() << (ts - lastTs).toMSec()
//            << ", ref =" << timeStamp.toMSec()
//            << ", key =" << videoBuffer.isKeyFrame()
//            << ", rpts =" << rpts.toMSec()
//            << ", pts =" << videoBuffer.presentationTimeStamp().toMSec()
//            << ", dts =" << videoBuffer.decodingTimeStamp().toMSec();
//        lastTs = ts;

        destBuffer.setTimeStamp(ts);
        output << destBuffer;

        timeStamp = ts + STime(1, frameRate);
      }

      if (len > 0)
      {
        sourceSize -= len;
        sourcePtr += len;
      }
      else
        break;
    }

#if ((LIBAVCODEC_VERSION_INT >> 16) < 52)
    delete reinterpret_cast<qint64 *>(contextHandle->opaque);
    contextHandle->opaque = NULL;
#endif
  }
  else if (codecHandle)
  {
    for (int gotPicture=1; gotPicture;)
    {
      // Get any remaining frames
      avcodec_get_frame_defaults(pictureHandle);
      gotPicture = 0;
      //const int len = avcodec_decode_video2(contextHandle, pictureHandle, &gotPicture, &packet);
      avcodec_decode_video(contextHandle, pictureHandle, &gotPicture, NULL, 0);
      if (gotPicture)
      {
        SVideoBuffer destBuffer(outFormat);
        for (int i=0, n=outFormat.numPlanes(); i<n; i++)
        for (int y=0; y<outNumLines[i]; y++)
        {
          memcpy(destBuffer.scanLine(y, i),
                 pictureHandle->data[i] + (pictureHandle->linesize[i] * y),
                 qMin(destBuffer.lineSize(i), pictureHandle->linesize[i]));
        }

        destBuffer.setTimeStamp(timeStamp);
        output << destBuffer;

        timeStamp += STime(1, outFormat.frameRate());
      }
    }
  }

  return output;
}


} } // End of namespaces
