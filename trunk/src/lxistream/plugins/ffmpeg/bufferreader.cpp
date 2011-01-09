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

#include "bufferreader.h"
#include "bufferwriter.h"

namespace LXiStream {
namespace FFMpegBackend {

const STime BufferReader::maxJumpTime = STime::fromSec(10);

BufferReader::BufferReader(const QString &, QObject *parent)
  : SInterfaces::BufferReader(parent),
    readCallback(NULL),
    produceCallback(NULL),
    format(NULL),
    formatContext(NULL),
    ioContext(NULL),
    running(false)
{
  for (unsigned i=0; i<MAX_STREAMS; i++)
    streamContext[i] = NULL;
}

BufferReader::~BufferReader()
{
  for (unsigned i=0; i<MAX_STREAMS; i++)
  if (streamContext[i])
  {
    delete [] streamContext[i]->dtsBuffer;
    delete streamContext[i];
  }

  if (formatContext)
    ::av_close_input_stream(formatContext);

  if (ioContext)
  {
    ::av_free(ioContext->buffer);
    ::av_free(ioContext);
  }
}

bool BufferReader::openFormat(const QString &name)
{
  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

  format = ::av_find_input_format(name.toAscii().data());

  // Do not allow redirecting.
  if (format)
  if (qstrcmp(format->name, "redir") == 0)
    format = NULL;

  return format != NULL;
}

bool BufferReader::start(ReadCallback *rc, ProduceCallback *pc, bool streamed)
{
  if (format)
  {
    if (ioContext)
      qFatal("BufferReader already opened a stream.");

    SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

    static const int ioBufferSize = 65536;

    readCallback = rc;
    produceCallback = pc;
    ioContext =
        ::av_alloc_put_byte((unsigned char *)::av_malloc(ioBufferSize),
                            ioBufferSize,
                            false,
                            this,
                            &BufferReader::read,
                            NULL,
                            &BufferReader::seek);

    ioContext->is_streamed = streamed;

    ::AVFormatParameters ap;
    memset(&ap, 0, sizeof(ap));

    if (::av_open_input_stream(&formatContext, ioContext, "", format, &ap) == 0)
    {
      if (::av_find_stream_info(formatContext) >= 0)
      {
        //::dump_format(formatContext, 0, "", false);

        bool hasAudio = false, hasVideo = false, hasSubtitle = false;

        for (unsigned i=0; i<formatContext->nb_streams; i++)
        {
          const AVStream * const stream = formatContext->streams[i];

          streamContext[i] = new StreamContext();
          streamContext[i]->timeBase = SInterval(stream->time_base.num, stream->time_base.den);
          streamContext[i]->selected = false;
          streamContext[i]->dtsChecked = false;
          streamContext[i]->needsDTSFraming = false;
          streamContext[i]->firstTimeStamp = STime();
          streamContext[i]->lastTimeStamp = STime();
          streamContext[i]->timeStampGap = STime::null;
          streamContext[i]->dtsBuffer = NULL;
          streamContext[i]->dtsBufferUsed = 0;

          if (stream->codec->codec_type == CODEC_TYPE_AUDIO)
          {
            streamContext[i]->audioCodec =
                SAudioCodec(FFMpegCommon::fromFFMpegCodecID(stream->codec->codec_id),
#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
                            FFMpegCommon::fromFFMpegChannelLayout(stream->codec->channel_layout, stream->codec->channels),
#else
                            SAudioFormat::guessChannels(stream->codec->channels),
#endif
                            stream->codec->sample_rate,
                            stream->codec->bit_rate);

            if (stream->codec->extradata && (stream->codec->extradata_size > 0))
              streamContext[i]->audioCodec.setExtraData(
                  QByteArray((const char *)stream->codec->extradata,
                             stream->codec->extradata_size));

            streamContext[i]->selected = !hasAudio;
            hasAudio = true;
          }
          else if (stream->codec->codec_type == CODEC_TYPE_VIDEO)
          {
            float ar = 1.0f;
            if ((stream->codec->sample_aspect_ratio.den != 0) &&
                (stream->codec->sample_aspect_ratio.num != 0))
            {
              ar = ::av_q2d(stream->codec->sample_aspect_ratio);
            }

            // Note that den and num are deliberately swapped! (see documentation of AVStream::r_frame_rate)
            const SInterval fr(stream->r_frame_rate.den, stream->r_frame_rate.num);

            streamContext[i]->videoCodec =
                SVideoCodec(FFMpegCommon::fromFFMpegCodecID(stream->codec->codec_id),
                            SSize(stream->codec->width, stream->codec->height, ar),
                            fr,
                            stream->codec->bit_rate);

            if (stream->codec->extradata && (stream->codec->extradata_size > 0))
              streamContext[i]->videoCodec.setExtraData(
                  QByteArray((const char *)stream->codec->extradata,
                             stream->codec->extradata_size));

            streamContext[i]->measurement.reserve(streamContext[i]->measurementSize);

            streamContext[i]->selected = !hasVideo;
            hasVideo = true;
          }
          else if (stream->codec->codec_type == CODEC_TYPE_SUBTITLE)
          {
            streamContext[i]->dataCodec =
                SDataCodec(FFMpegCommon::fromFFMpegCodecID(stream->codec->codec_id));

            if (stream->codec->extradata && (stream->codec->extradata_size > 0))
              streamContext[i]->dataCodec.setExtraData(
                  QByteArray((const char *)stream->codec->extradata,
                             stream->codec->extradata_size));

            streamContext[i]->selected = !hasSubtitle;
            hasSubtitle = true;
          }
        }

        if (hasVideo)
        { // Determine the framerate
          running = false;

          // Gather timestamps.
          for (unsigned i=0, f=0; (i<maxBufferCount) && (f==0); i++)
          {
            if (!BufferReader::process())
              break;

            // Check if we have enough data.
            f = 1;
            for (unsigned j=0; j<formatContext->nb_streams; j++)
            if (formatContext->streams[j]->codec->codec_type == CODEC_TYPE_VIDEO)
            if (streamContext[j]->measurement.count() < streamContext[j]->measurementSize)
              f = 0;
          }

          // And determine the framerate for each video stream          .
          for (unsigned i=0; i<formatContext->nb_streams; i++)
          if (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
          {
            if (streamContext[i]->measurement.count() >= streamContext[i]->measurementSize)
            {
              qSort(streamContext[i]->measurement);
//              for (unsigned j=7; j<=streamContext[i]->measurementSize-6; j++)
//                qDebug() << "Delta" << (streamContext[i]->measurement[j] - streamContext[i]->measurement[j-1]).toUSec();

              const STime delta =
                  streamContext[i]->measurement[streamContext[i]->measurementSize - 6] -
                  streamContext[i]->measurement[6];
              const SInterval refFrameRate = streamContext[i]->videoCodec.frameRate();
              const STime frameTime = delta / (streamContext[i]->measurementSize - 12);
              const SInterval frameRate(frameTime.count() * frameTime.interval().num(), frameTime.interval().den());

              if (qAbs(refFrameRate.toFrequency() - frameRate.toFrequency()) > 2.0f)
                streamContext[i]->videoCodec.setFrameRate(frameRate);
            }

            streamContext[i]->measurement.clear();
          }
        }

        return running = true;
      }
    }
  }

  return false;
}

void BufferReader::stop(void)
{
  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

  for (unsigned i=0; i<MAX_STREAMS; i++)
  if (streamContext[i])
  {
    delete [] streamContext[i]->dtsBuffer;
    delete streamContext[i];
    streamContext[i] = NULL;
  }

  if (formatContext)
  {
    ::av_close_input_stream(formatContext);
    formatContext = NULL;
  }

  if (ioContext)
  {
    ::av_free(ioContext->buffer);
    ::av_free(ioContext);
    ioContext = NULL;
  }

  readCallback = NULL;
  produceCallback = NULL;
}

bool BufferReader::process(void)
{
  if ((running && audioBuffers.isEmpty() && videoBuffers.isEmpty() && dataBuffers.isEmpty()) || !running)
  {
    ::AVPacket packet;
    ::av_init_packet(&packet);

    if (::av_read_frame(formatContext, &packet) >= 0)
    {
      if (unsigned(packet.stream_index) < formatContext->nb_streams)
      {
        const AVStream * const stream = formatContext->streams[packet.stream_index];
        StreamContext * const context = streamContext[packet.stream_index];

        if (stream && context)
        if (context->selected || !running)
        {
          if (stream->codec->codec_type == CODEC_TYPE_AUDIO)
          {
            // Detect DTS (Digital Theatre Surround) if needed.
            if (!context->dtsChecked &&
                ((stream->codec->codec_id == CODEC_ID_PCM_S16LE) ||
                 (stream->codec->codec_id == CODEC_ID_PCM_S16BE)))
            { // Not checked for DTS yet
              if (isDTS(packet.data, packet.size))
              {
                context->needsDTSFraming = true;
                context->dtsBuffer = new quint8[context->dtsBufferSize];
                context->dtsBufferUsed = 0;

                context->audioCodec =
                    SAudioCodec("DTS",
#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
                                FFMpegCommon::fromFFMpegChannelLayout(stream->codec->channel_layout,
                                                                      stream->codec->channels),
#else
                                SAudioFormat::guessChannels(stream->codec->channels),
#endif
                                stream->codec->sample_rate);
              }

              context->dtsChecked = true; // Checked for DTS.
            }

            if (!context->needsDTSFraming)
            {
              SEncodedAudioBuffer buffer(context->audioCodec, packet.size);

              memcpy(buffer.data(), packet.data, packet.size);
              buffer.resize(packet.size);

              const QPair<STime, STime> ts = correctTimeStamp(packet);
              buffer.setPresentationTimeStamp(ts.first);
              buffer.setDecodingTimeStamp(ts.second);

//              qDebug() << "Audio timestamp" << packet.stream_index
//                  << ", pts = " << buffer.presentationTimeStamp().toMSec();

              if (running)
                produceCallback->produce(buffer);
              else
                audioBuffers += QPair<StreamContext *, SEncodedAudioBuffer>(context, buffer);
            }
            else foreach (SEncodedAudioBuffer buffer, parseDTSFrames(context, packet.data, packet.size))
            { // Do DTS framing
              const QPair<STime, STime> ts = correctTimeStamp(packet);
              buffer.setPresentationTimeStamp(ts.first);
              buffer.setDecodingTimeStamp(ts.second);

//              qDebug() << "Audio timestamp" << packet.stream_index
//                  << ", pts = " << buffer.presentationTimeStamp().toMSec();

              if (running)
                produceCallback->produce(buffer);
              else
                audioBuffers += QPair<StreamContext *, SEncodedAudioBuffer>(context, buffer);
            }
          }
          else if (stream->codec->codec_type == CODEC_TYPE_VIDEO)
          {
            SEncodedVideoBuffer buffer(context->videoCodec, packet.size);

            memcpy(buffer.data(), packet.data, packet.size);
            buffer.resize(packet.size);

            const QPair<STime, STime> ts = correctTimeStamp(packet);
            buffer.setPresentationTimeStamp(ts.first);
            buffer.setDecodingTimeStamp(ts.second);

            buffer.setKeyFrame((packet.flags & PKT_FLAG_KEY) != 0);

//            qDebug() << "Video timestamp" << packet.stream_index
//                << ", dts =" << buffer.decodingTimeStamp().toMSec()
//                << ", pts =" << buffer.presentationTimeStamp().toMSec()
//                << ", key =" << buffer.isKeyFrame();

            if (running)
            {
              produceCallback->produce(buffer);
            }
            else
            {
              videoBuffers += QPair<StreamContext *, SEncodedVideoBuffer>(context, buffer);
              if (context->measurement.count() < context->measurementSize)
              {
                STime ts;
                if (packet.pts != AV_NOPTS_VALUE)
                  ts = STime::fromClock(packet.pts, stream->time_base.num, stream->time_base.den);
                else if (packet.dts != AV_NOPTS_VALUE)
                  ts = STime::fromClock(packet.dts, stream->time_base.num, stream->time_base.den);

                if (ts.isValid())
                {
                  // Remove invalid timestamps
                  foreach (const STime &t, context->measurement)
                  if (qAbs(t - ts) > STime::fromSec(context->measurementSize + 4))
                  {
                    context->measurement.clear();
                    break;
                  }

                  context->measurement += ts;
                }
              }
            }
          }
          else if (stream->codec->codec_type == CODEC_TYPE_SUBTITLE)
          {
            SEncodedDataBuffer buffer(context->dataCodec, packet.size);

            memcpy(buffer.data(), packet.data, packet.size);
            buffer.resize(packet.size);

            const QPair<STime, STime> ts = correctTimeStampToVideo(packet);
            buffer.setPresentationTimeStamp(ts.first);
            buffer.setDecodingTimeStamp(ts.second);
#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
            buffer.setDuration(STime(packet.convergence_duration, context->timeBase));
#else
            buffer.setDuration(STime(packet.duration, context->timeBase));
#endif

//            qDebug() << "Data timestamp" << packet.stream_index
//                << ", dts =" << buffer.decodingTimeStamp().toMSec()
//                << ", pts =" << buffer.presentationTimeStamp().toMSec()
//                << ", duration =" << buffer.duration().toMSec();

            if (running)
              produceCallback->produce(buffer);
            else
              dataBuffers += QPair<StreamContext *, SEncodedDataBuffer>(context, buffer);
          }
        }
      }

      ::av_free_packet(&packet);
    }
    else // Error or end-of-file.
      return false;
  }
  else // First send out cached buffers.
  {
    if (!dataBuffers.isEmpty())
    {
      if (dataBuffers.first().first->selected)
      {
        dataBuffers.first().second.setCodec(dataBuffers.first().first->dataCodec);
        produceCallback->produce(dataBuffers.takeFirst().second);
      }
      else
        dataBuffers.takeFirst();
    }
    else if (!audioBuffers.isEmpty() && !videoBuffers.isEmpty())
    {
      if (audioBuffers.first().second.presentationTimeStamp() <= videoBuffers.first().second.presentationTimeStamp())
      {
        if (audioBuffers.first().first->selected)
        {
          audioBuffers.first().second.setCodec(audioBuffers.first().first->audioCodec);
          produceCallback->produce(audioBuffers.takeFirst().second);
        }
        else
          audioBuffers.takeFirst();
      }
      else
      {
        if (videoBuffers.first().first->selected)
        {
          videoBuffers.first().second.setCodec(videoBuffers.first().first->videoCodec);
          produceCallback->produce(videoBuffers.takeFirst().second);
        }
        else
          videoBuffers.takeFirst();
      }
    }
    else if (!audioBuffers.isEmpty())
    {
      if (audioBuffers.first().first->selected)
      {
        audioBuffers.first().second.setCodec(audioBuffers.first().first->audioCodec);
        produceCallback->produce(audioBuffers.takeFirst().second);
      }
      else
        audioBuffers.takeFirst();
    }
    else if (!videoBuffers.isEmpty())
    {
      if (videoBuffers.first().first->selected)
      {
        videoBuffers.first().second.setCodec(videoBuffers.first().first->videoCodec);
        produceCallback->produce(videoBuffers.takeFirst().second);
      }
      else
        videoBuffers.takeFirst();
    }
  }

  return true;
}

STime BufferReader::duration(void) const
{
  if (formatContext->duration != AV_NOPTS_VALUE)
    return STime::fromClock(formatContext->duration, AV_TIME_BASE);

  return STime();
}

bool BufferReader::setPosition(STime pos)
{
  if (formatContext)
  {
    if (!pos.isNull())
    {
      audioBuffers.clear();
      videoBuffers.clear();
      dataBuffers.clear();
    }

    for (unsigned i=0; i<formatContext->nb_streams; i++)
    if (streamContext[i])
      streamContext[i]->lastTimeStamp = STime();

    for (unsigned i=0; i<formatContext->nb_streams; i++)
    if (streamContext[i])
    if (streamContext[i]->selected && streamContext[i]->lastTimeStamp.isValid() && streamContext[i]->firstTimeStamp.isValid())
    {
      const STime p = pos + streamContext[i]->firstTimeStamp;

      const qint64 timestamp =
        p.toClock(formatContext->streams[i]->time_base.num,
                  formatContext->streams[i]->time_base.den);

      const int flags = (p < streamContext[i]->lastTimeStamp) ? AVSEEK_FLAG_BACKWARD : 0;
      if (::av_seek_frame(formatContext, i, timestamp, flags) >= 0)
        return true;
    }
  }

  return false;
}

STime BufferReader::position(void) const
{
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (streamContext[i])
  if (streamContext[i]->selected && streamContext[i]->lastTimeStamp.isValid() && streamContext[i]->firstTimeStamp.isValid())
    return streamContext[i]->lastTimeStamp - streamContext[i]->firstTimeStamp;

  return STime();
}

QList<BufferReader::AudioStreamInfo> BufferReader::audioStreams(void) const
{
  QList<AudioStreamInfo> streams;

  if (formatContext)
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
  if (streamContext[i])
    streams += AudioStreamInfo(i, formatContext->streams[i]->language, streamContext[i]->audioCodec);

  return streams;
}

QList<BufferReader::VideoStreamInfo> BufferReader::videoStreams(void) const
{
  QList<VideoStreamInfo> streams;

  if (formatContext)
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
  if (streamContext[i])
    streams += VideoStreamInfo(i, formatContext->streams[i]->language, streamContext[i]->videoCodec);

  return streams;
}

QList<BufferReader::DataStreamInfo> BufferReader::dataStreams(void) const
{
  QList<DataStreamInfo> streams;

  if (formatContext)
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_SUBTITLE)
  if (streamContext[i])
    streams += DataStreamInfo(i, formatContext->streams[i]->language, streamContext[i]->dataCodec);

  return streams;
}

void BufferReader::selectStreams(const QList<quint16> &streamIds)
{
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (streamContext[i])
    streamContext[i]->selected = streamIds.contains(i);
}

QList<BufferReader::Chapter> BufferReader::chapters(void) const
{
  QMultiMap<STime, Chapter> chapters;

  if (formatContext)
  for (unsigned i=0; i<formatContext->nb_chapters; i++)
  {
    const SInterval interval(formatContext->chapters[i]->time_base.num, formatContext->chapters[i]->time_base.den);

    Chapter chapter;
#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
    if (formatContext->chapters[i]->title)
      chapter.title = formatContext->chapters[i]->title;
#endif
    chapter.begin = STime(formatContext->chapters[i]->start, interval);
    chapter.end = STime(formatContext->chapters[i]->end, interval);

    chapters.insert(chapter.begin, chapter);
  }

  return chapters.values();
}

/*! Returns true if the data contains DTS data.
 */
bool BufferReader::isDTS(const uint8_t *inPtr, int inSize)
{
  const int ofs = findDTSFrame(inPtr, inSize);
  if (ofs >= 0)
  {
    const unsigned type = findDTSFrameType(inPtr + ofs);

    if ((type == 1) || (type == 2))
      return true; // 16 bits DTS data
    else if (type == 3) // 14-bits BE; double-check.
    {
      for (int i=ofs; i<inSize-1; i+=2)
      {
        const qint16 value = qFromBigEndian(*reinterpret_cast<const qint16 *>(inPtr + i));
        if ((value > 16383) || (value < -16384))
          return false;
      }

      return true;
    }
    else if (type == 4) // 14-bits LE; double-check.
    {
      for (int i=ofs; i<inSize-1; i+=2)
      {
        const qint16 value = qFromLittleEndian(*reinterpret_cast<const qint16 *>(inPtr + i));
        if ((value > 16383) || (value < -16384))
          return false;
      }

      return true;
    }
  }

  return false;
}

/*! Looks for a DTS frame header in the specified buffer and returns the offset
    to this frame, or -1 if not found.
 */
int BufferReader::findDTSFrame(const uint8_t *inPtr, int inSize)
{
  for (int i=0; i<(inSize-10); i++)
  if (findDTSFrameType(inPtr + i) != 0)
    return i;

  return -1;
}

/*! Checks the DTS frame type; raw big-endian (1), raw little-endian (2), 14-bit
    big-endian (3) or 14-bit little-endian (4) or returns 0 if none of these are
    found.
*/
unsigned BufferReader::findDTSFrameType(const uint8_t *inPtr)
{
  if ((inPtr[0] == 0x7F) && (inPtr[1] == 0xFE) && (inPtr[2] == 0x80) && (inPtr[3] == 0x01))
    return 1;
  else if ((inPtr[0] == 0xFE) && (inPtr[1] == 0x7F) && (inPtr[2] == 0x01) && (inPtr[3] == 0x80))
    return 2;
  else if ((inPtr[0] == 0x1F) && (inPtr[1] == 0xFF) && (inPtr[2] == 0xE8) && (inPtr[3] == 0x00))
    return 3;
  else if ((inPtr[0] == 0xFF) && (inPtr[1] == 0x1F) && (inPtr[2] == 0x00) && (inPtr[3] == 0xE8))
    return 4;
  else
    return 0;
}

SEncodedAudioBufferList BufferReader::parseDTSFrames(StreamContext *context, const uint8_t *inPtr, int inSize)
{
  if (context->dtsBufferUsed + inSize <= int(context->dtsBufferSize))
  {
    memcpy(context->dtsBuffer + context->dtsBufferUsed, inPtr, inSize);
    context->dtsBufferUsed += inSize;
  }
  else // Should not happen
  {
    memcpy(context->dtsBuffer, inPtr, inSize);
    context->dtsBufferUsed = inSize;
  }

  SEncodedAudioBufferList result;
  while (context->dtsBufferUsed > 10)
  {
    const unsigned type = findDTSFrameType(context->dtsBuffer);
    unsigned frameSize = 0;

    // Determine frame size
    if (type == 1)
    {
      const quint16 * const data = reinterpret_cast<const quint16 *>(context->dtsBuffer);

      // Get the framesize from bits 46 - 60 in the frame.
      frameSize = ((qFromBigEndian(data[2]) << 12) |
                   (qFromBigEndian(data[3]) >> 2)) + 1;
    }
    else if (type == 2)
    {
      const quint16 * const data = reinterpret_cast<const quint16 *>(context->dtsBuffer);

      // Get the framesize from bits 46 - 60 in the frame.
      frameSize = ((qFromLittleEndian(data[2]) << 12) |
                   (qFromLittleEndian(data[3]) >> 2)) + 1;
    }
    else if (type == 3)
    {
      const quint16 * const data = reinterpret_cast<const quint16 *>(context->dtsBuffer);

      // Get the framesize from bits 46 - 60 in the frame. Note that the frame
      // is packed in 14 bits data, we have to compensate for this.
      frameSize = (((qFromBigEndian(data[3]) << 4) & 0x3FFF) |
                   ((qFromBigEndian(data[4]) & 0x3FFF) >> 10)) + 1;
      frameSize = ((frameSize * 16 / 14) + 1) & 0xFFFE;
    }
    else if (type == 4)
    {
      const quint16 * const data = reinterpret_cast<const quint16 *>(context->dtsBuffer);

      // Get the framesize from bits 46 - 60 in the frame. Note that the frame
      // is packed in 14 bits data, we have to compensate for this.
      frameSize = (((qFromLittleEndian(data[3]) << 4) & 0x3FFF) |
                   ((qFromLittleEndian(data[4]) & 0x3FFF) >> 10)) + 1;
      frameSize = ((frameSize * 16 / 14) + 1) & 0xFFFE;
    }

    if ((type == 0) || (frameSize == 0)) // Lost sync
    {
      const int ofs = findDTSFrame(context->dtsBuffer + 1, context->dtsBufferUsed - 1) + 1;
      if ((ofs > 0) && (unsigned(ofs) < context->dtsBufferUsed))
      {
        memmove(context->dtsBuffer, context->dtsBuffer + ofs, context->dtsBufferUsed - ofs);
        context->dtsBufferUsed -= ofs;
        continue;
      }
      else
        break;
    }

    // Get the DTS frame
    if ((frameSize > 0) && (context->dtsBufferUsed >= frameSize))
    {
      SEncodedAudioBuffer buffer(context->audioCodec, frameSize);
      memcpy(buffer.data(), context->dtsBuffer, frameSize);
      buffer.resize(frameSize);

      result << buffer;
      memmove(context->dtsBuffer, context->dtsBuffer + frameSize, context->dtsBufferUsed - frameSize);
      context->dtsBufferUsed -= frameSize;
    }
    else
      break;
  }

  return result;
}

QPair<STime, STime> BufferReader::correctTimeStamp(const AVPacket &packet)
{
  StreamContext * const context = streamContext[packet.stream_index];

  // Ensure the timestamps are zero-based. Non-zero bases streams can occur
  // when captured from DVB or a sample has been cut from a larger file.
  STime subtract = context->timeStampGap;
  if (context->firstTimeStamp.isValid())
    subtract += context->firstTimeStamp;

  // Determine the baseTimeStamp, decodingTimeStamp and presentationTimeStamp.
  STime baseTimeStamp, decodingTimeStamp, presentationTimeStamp;

  if (packet.dts != AV_NOPTS_VALUE)
  {
    baseTimeStamp = decodingTimeStamp =
                    STime(packet.dts, context->timeBase) - subtract;

    if (packet.pts != AV_NOPTS_VALUE)
    {
      presentationTimeStamp =
          STime(packet.pts, context->timeBase) - subtract;

      if (qAbs(presentationTimeStamp - decodingTimeStamp) <= maxJumpTime)
      {
        baseTimeStamp = qMin(presentationTimeStamp, decodingTimeStamp);
      }
      else
      {
        baseTimeStamp = decodingTimeStamp = presentationTimeStamp;
        presentationTimeStamp = STime(); // Compute after decoding
      }
    }
  }
  else if (packet.pts != AV_NOPTS_VALUE)
  {
    baseTimeStamp = decodingTimeStamp = presentationTimeStamp =
        STime(packet.pts, context->timeBase) - subtract;
  }
  else
    baseTimeStamp = context->lastTimeStamp;

  if (!context->firstTimeStamp.isValid() || (context->firstTimeStamp > baseTimeStamp))
  {
    // Ensure all streams stay syncronized.
    STime firstTimeStamp = baseTimeStamp;
    for (unsigned i=0; i<formatContext->nb_streams; i++)
    if (streamContext[i]->firstTimeStamp.isValid() && baseTimeStamp.isValid() &&
        (qAbs(streamContext[i]->firstTimeStamp - baseTimeStamp) < maxJumpTime))
    {
      firstTimeStamp = qMin(firstTimeStamp, streamContext[i]->firstTimeStamp);
    }

    context->firstTimeStamp = firstTimeStamp;
    for (unsigned i=0; i<formatContext->nb_streams; i++)
    if (streamContext[i]->firstTimeStamp.isValid() && baseTimeStamp.isValid() &&
        (qAbs(streamContext[i]->firstTimeStamp - baseTimeStamp) < maxJumpTime))
    {
      streamContext[i]->firstTimeStamp = firstTimeStamp;
    }

    if (decodingTimeStamp.isValid())
      decodingTimeStamp -= firstTimeStamp;

    if (presentationTimeStamp.isValid())
      presentationTimeStamp -= firstTimeStamp;

    baseTimeStamp -= firstTimeStamp;
  }

  if (context->lastTimeStamp.isValid())
  {
    const STime delta = baseTimeStamp - context->lastTimeStamp;
    if (qAbs(delta) > maxJumpTime)
    {
      context->timeStampGap += delta;
      baseTimeStamp -= delta;

      if (decodingTimeStamp.isValid())
        decodingTimeStamp -= delta;

      if (presentationTimeStamp.isValid())
        presentationTimeStamp -= delta;
    }
  }

  context->lastTimeStamp = baseTimeStamp;

  return QPair<STime, STime>(presentationTimeStamp, decodingTimeStamp);
}

QPair<STime, STime> BufferReader::correctTimeStampToVideo(const AVPacket &packet)
{
  StreamContext * const context = streamContext[packet.stream_index];

  // Ensure the timestamps are zero-based. Non-zero bases streams can occur
  // when captured from DVB or a sample has been cut from a larger file.
  STime subtract = STime::null;
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (streamContext[i]->selected && (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO))
  {
    subtract = streamContext[i]->timeStampGap;
    if (streamContext[i]->firstTimeStamp.isValid())
      subtract += streamContext[i]->firstTimeStamp;

    break;
  }

  // Determine the decodingTimeStamp and presentationTimeStamp.
  STime decodingTimeStamp, presentationTimeStamp;

  if (packet.dts != AV_NOPTS_VALUE)
    decodingTimeStamp = STime(packet.dts, context->timeBase) - subtract;

  if (packet.pts != AV_NOPTS_VALUE)
    presentationTimeStamp = STime(packet.pts, context->timeBase) - subtract;

  return QPair<STime, STime>(presentationTimeStamp, decodingTimeStamp);
}

int BufferReader::read(void *opaque, uint8_t *buf, int buf_size)
{
  return reinterpret_cast<BufferReader *>(opaque)->readCallback->read(buf, buf_size);
}

int64_t BufferReader::seek(void *opaque, int64_t offset, int whence)
{
  return reinterpret_cast<BufferReader *>(opaque)->readCallback->seek(offset, (whence == AVSEEK_SIZE) ? -1 : whence);
}


} } // End of namespaces
