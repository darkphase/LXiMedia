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

#include "videodecoder.h"
#include "bufferreader.h"
#include "networkbufferreader.h"

namespace LXiStream {
namespace FFMpegBackend {


VideoDecoder::VideoDecoder(const QString &, QObject *parent)
  : SInterfaces::VideoDecoder(parent),
    inCodec(),
    codecHandle(NULL),
    contextHandle(NULL),
    contextHandleOwner(false),
    pictureHandle(NULL),
    outFormat(),
    timeStamp(STime::null)
{
}

VideoDecoder::~VideoDecoder()
{
  if (contextHandle && contextHandleOwner)
  {
    if (codecHandle)
      ::avcodec_close(contextHandle);

    ::av_free(contextHandle);
  }

  if (pictureHandle)
    av_free(pictureHandle);
}

bool VideoDecoder::openCodec(const SVideoCodec &c, SInterfaces::AbstractBufferReader *bufferReader, Flags flags)
{
  if (contextHandle)
    qFatal("VideoDecoder already opened a codec.");

  inCodec = c;
  timeStamp = STime::null;
  reorderBuffer.clear();

  if ((codecHandle = avcodec_find_decoder(FFMpegCommon::toFFMpegCodecID(inCodec))) == NULL)
  {
    qWarning() << "VideoDecoder: Video codec not found " << inCodec.codec();
    return false;
  }

  BufferReaderBase * ffBufferReader = qobject_cast<BufferReader *>(bufferReader);
  if (ffBufferReader == NULL)
    ffBufferReader = qobject_cast<NetworkBufferReader *>(bufferReader);

  if (ffBufferReader && (inCodec.streamId() >= 0))
  {
    contextHandle = ffBufferReader->getStream(inCodec.streamId())->codec;
    contextHandleOwner = false;
  }
  else
  {
    contextHandle = avcodec_alloc_context();
    contextHandleOwner = true;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
    contextHandle->codec_type = AVMEDIA_TYPE_VIDEO;
#else
    contextHandle->codec_type = CODEC_TYPE_VIDEO;
#endif

    contextHandle->coded_width = inCodec.size().width();
    contextHandle->coded_height = inCodec.size().height();
    contextHandle->time_base.num = inCodec.frameRate().num();
    contextHandle->time_base.den = inCodec.frameRate().den();

    contextHandle->bit_rate = inCodec.bitRate();
  }

  contextHandle->flags2 |= CODEC_FLAG2_CHUNKS;
  contextHandle->error_recognition = FF_ER_COMPLIANT;
  contextHandle->error_concealment = FF_EC_DEBLOCK;

#ifdef FF_BUG_NO_PADDING
  if (codecHandle->id == CODEC_ID_MPEG4)
    contextHandle->workaround_bugs = FF_BUG_NO_PADDING;
#endif

  contextHandle->width = 0;
  contextHandle->height = 0;

  if (flags & Flag_KeyframesOnly)
    contextHandle->skip_frame = AVDISCARD_NONKEY;
  else
    contextHandle->skip_frame = AVDISCARD_DEFAULT;

  if (flags & Flag_Fast)
    contextHandle->flags2 |= CODEC_FLAG2_FAST;

#ifdef OPT_ENABLE_THREADS
  contextHandle->thread_count = FFMpegCommon::decodeThreadCount(codecHandle->id);
  contextHandle->execute = &FFMpegCommon::execute;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  contextHandle->execute2 = &FFMpegCommon::execute2;
#endif
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 6, 0)
  if (avcodec_open(contextHandle, codecHandle) < 0)
#else
  if (avcodec_open2(contextHandle, codecHandle, NULL) < 0)
#endif
  {
    qCritical() << "VideoDecoder: Could not open video codec " << inCodec.codec();
    codecHandle = NULL;
    return false;
  }

  pictureHandle = avcodec_alloc_frame();
  avcodec_get_frame_defaults(pictureHandle);

  return true;
}

SVideoBufferList VideoDecoder::decodeBuffer(const SEncodedVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  SVideoBufferList output;
  if (!videoBuffer.isNull() && codecHandle)
  {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
    ::AVPacket packet = FFMpegCommon::toAVPacket(videoBuffer);
#endif

    const uint8_t *inPtr = reinterpret_cast<const uint8_t *>(videoBuffer.data());
    int inSize = videoBuffer.size();

    while (inSize > 0)
    {
      ::avcodec_get_frame_defaults(pictureHandle);

      contextHandle->reordered_opaque =
          videoBuffer.presentationTimeStamp().isValid()
          ? videoBuffer.presentationTimeStamp().toClock(contextHandle->time_base.num, contextHandle->time_base.den)
          : AV_NOPTS_VALUE;

      int gotPicture = 0;

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
      const int len = ::avcodec_decode_video(contextHandle, pictureHandle, &gotPicture, (uint8_t *)inPtr, inSize);
#else
      packet.data = (uint8_t *)inPtr;
      packet.size = inSize;
      const int len = ::avcodec_decode_video2(contextHandle, pictureHandle, &gotPicture, &packet);
#endif

      for (int i=0, n=outFormat.numPlanes(); (i<n) && gotPicture; i++)
      if (pictureHandle->data[i] == NULL)
        gotPicture = 0;

      if ((len >= 0) && gotPicture)
      {
        // Determine metadata.
        SVideoFormat::Format format = FFMpegCommon::fromFFMpegPixelFormat(contextHandle->pix_fmt);

        SSize size;
        if ((contextHandle->sample_aspect_ratio.den == 0) || (contextHandle->sample_aspect_ratio.num == 0))
          size = SSize(contextHandle->width, contextHandle->height);
        else
          size = SSize(contextHandle->width, contextHandle->height, ::av_q2d(contextHandle->sample_aspect_ratio));

        if (qFuzzyCompare(size.aspectRatio(), 1.0f))
          size.setAspectRatio(videoBuffer.codec().size().aspectRatio());

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
        const STime rpts =
            (pictureHandle->reordered_opaque != AV_NOPTS_VALUE)
            ? STime::fromClock(pictureHandle->reordered_opaque, contextHandle->time_base.num, contextHandle->time_base.den)
            : STime();

        STime ts;
        if (rpts.isValid())
          ts = rpts;
        else if (videoBuffer.isKeyFrame() && videoBuffer.decodingTimeStamp().isValid())
          ts = videoBuffer.decodingTimeStamp();
        else 
          ts = timeStamp;

        // This might give a better PTS.
//        if (pictureHandle->pts > 0)
//        {
//          const STime framePts = STime::fromClock(pictureHandle->pts, contextHandle->time_base.num, contextHandle->time_base.den);
//          if (qAbs(ts - framePts) < STime::fromSec(1))
//            ts = framePts;
//        }

        if ((contextHandle->has_b_frames > 1) && (contextHandle->has_b_frames <= 30))
        {
          destBuffer.setTimeStamp(ts);
          reorderBuffer.insert(ts, destBuffer);

          if (reorderBuffer.count() > contextHandle->has_b_frames)
          {
            QMap<STime, SVideoBuffer>::Iterator i = reorderBuffer.begin();
            ts = i.key();
            destBuffer = i.value();
            reorderBuffer.erase(i);
          }
          else
            destBuffer = SVideoBuffer();
        }

        if (!destBuffer.isNull())
        {
          // Correct invalid timestamps
          const STime delta = qAbs(timeStamp - ts);
          if (!videoBuffer.isKeyFrame() && (delta <= STime(contextHandle->gop_size, frameRate)))
            ts = timeStamp;
          else if (videoBuffer.isKeyFrame() && (delta <= STime(2, frameRate)))
            ts = timeStamp;

          destBuffer.setTimeStamp(ts);
          output << destBuffer;

//          static STime lastTs = STime::fromSec(0);
//          qDebug() << "Video timestamp" << ts.toMSec() << (ts - lastTs).toMSec()
//              << ", ref =" << timeStamp.toMSec()
//              << ", key =" << videoBuffer.isKeyFrame()
//              << ", rpts =" << rpts.toMSec()
//              << ", pts =" << videoBuffer.presentationTimeStamp().toMSec()
//              << ", dts =" << videoBuffer.decodingTimeStamp().toMSec()
//              << ", max_b_frames = " << contextHandle->max_b_frames
//              << ", has_b_frames = " << contextHandle->has_b_frames
//              << ", phpts = " << pictureHandle->pts
//              << ", repeat_pict = " << pictureHandle->repeat_pict;
//          lastTs = ts;

          timeStamp = ts + STime(1, frameRate);
        }
      }

      if (len > 0)
      {
        inPtr += len;
        inSize -= len;
      }
      else
        break;
    }
  }
  else if (codecHandle)
  {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
    ::AVPacket packet = FFMpegCommon::toAVPacket(videoBuffer);
#endif

    for (int gotPicture=1; gotPicture && contextHandle->codec;)
    {
      // Get any remaining frames
      ::avcodec_get_frame_defaults(pictureHandle);
      gotPicture = 0;

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
      ::avcodec_decode_video(contextHandle, pictureHandle, &gotPicture, NULL, 0);
#else
      ::avcodec_decode_video2(contextHandle, pictureHandle, &gotPicture, &packet);
#endif

      //const int len = avcodec_decode_video2(contextHandle, pictureHandle, &gotPicture, &packet);
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

    output << SVideoBuffer(); // Flush
  }

  return output;
}


} } // End of namespaces
