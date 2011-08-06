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

#include "bufferreaderbase.h"

namespace LXiStream {
namespace FFMpegBackend {

const STime BufferReaderBase::maxJumpTime = STime::fromSec(10);

BufferReaderBase::BufferReaderBase(void)
  : produceCallback(NULL),
    formatContext(NULL)
{
  for (unsigned i=0; i<MAX_STREAMS; i++)
    streamContext[i] = NULL;
}

BufferReaderBase::~BufferReaderBase()
{
  for (unsigned i=0; i<MAX_STREAMS; i++)
  if (streamContext[i])
  {
    delete [] streamContext[i]->dtsBuffer;
    delete streamContext[i];
  }

  if (formatContext)
    ::av_close_input_stream(formatContext);
}

bool BufferReaderBase::start(ProduceCallback *produceCallback, ::AVFormatContext *formatContext)
{
  this->produceCallback = produceCallback;
  this->formatContext = formatContext;

  formatContext->flags |= AVFMT_FLAG_GENPTS;
  formatContext->max_analyze_duration = 3 * AV_TIME_BASE;
  if (::av_find_stream_info(formatContext) >= 0)
  {
    //::dump_format(formatContext, 0, "", false);

    bool hasAudio = false, hasVideo = false, hasSubtitle = false;

    for (unsigned i=0; i<formatContext->nb_streams; i++)
    {
      const ::AVStream * const stream = formatContext->streams[i];

      streamContext[i] = initStreamContext(stream);

      if (stream->codec->codec_type == CODEC_TYPE_AUDIO)
      {
        if (!hasAudio)
          selectedStreams.insert(StreamId(StreamId::Type_Audio, stream->index));

        hasAudio = true;
      }
      else if (stream->codec->codec_type == CODEC_TYPE_VIDEO)
      {
        if (!hasVideo)
          selectedStreams.insert(StreamId(StreamId::Type_Video, stream->index));

        hasVideo = true;
      }
      else if (stream->codec->codec_type == CODEC_TYPE_SUBTITLE)
      {
        if (!hasSubtitle)
          selectedStreams.insert(StreamId(StreamId::Type_Subtitle, stream->index));

        hasSubtitle = true;
      }
    }

    if (hasVideo)
    { // Determine the framerate
      QList<Packet> readAheadBuffer;
      for (unsigned i=0, f=0; (i<maxBufferCount) && (f==0); i++)
      {
        f = 1;

        const Packet packet = read(false);
        if (packet.streamIndex >= 0)
        {
          if (unsigned(packet.streamIndex) < formatContext->nb_streams)
          {
            const ::AVStream * const stream = formatContext->streams[packet.streamIndex];
            if (stream && (streamContext[packet.streamIndex] == NULL))
              streamContext[packet.streamIndex] = initStreamContext(stream);

            StreamContext * const context = streamContext[packet.streamIndex];

            if (stream && context)
            {
              if (stream->codec->codec_type == CODEC_TYPE_VIDEO)
              {
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
                else
                  f = 0; // Finished
              }
            }
          }
          else
            break;
        }

        readAheadBuffer += packet;
      }

      // And determine the framerate for each video stream          .
      for (unsigned i=0; i<formatContext->nb_streams; i++)
      if (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
      {
        if (streamContext[i]->measurement.count() >= streamContext[i]->measurementSize)
        {
          qSort(streamContext[i]->measurement);

          QList<qint64> intervals;
          for (int j=1; j<streamContext[i]->measurementSize; j++)
            intervals += (streamContext[i]->measurement[j] - streamContext[i]->measurement[j-1]).toUSec();

          qSort(intervals);
          const qint64 median = intervals[intervals.count() / 2];
          for (QList<qint64>::Iterator j=intervals.begin(); j!=intervals.end();)
          if (qAbs(*j - median) < (median / 2))
            j++;
          else
            j = intervals.erase(j);

          while ((intervals.count() % 6) != 0)
            intervals.takeLast();

//              foreach (int interval, intervals)
//                qDebug() << "Interval" << interval;

          if (intervals.count() > 0)
          {
            qint64 sum = 0;
            foreach (int interval, intervals)
              sum += interval;

            const SInterval refFrameRate = streamContext[i]->videoCodec.frameRate();
            SInterval frameRate(sum, intervals.count() * 1000000);

            // Check if a 1/1, 1/2, 1/3 or 1/4 of the refFrameRate is a better match.
            for (int j=1; j<=4; j++)
            if (qAbs((refFrameRate.toFrequency() / j) - frameRate.toFrequency()) < 1.0)
              frameRate = SInterval(refFrameRate.num() * j, refFrameRate.den());

//                qDebug() << "Framerate" << frameRate.toFrequency() << refFrameRate.toFrequency();
            streamContext[i]->videoCodec.setFrameRate(frameRate.simplified());
          }
        }

        streamContext[i]->measurement.clear();
      }

      packetBuffer = readAheadBuffer;
    }

    return true;
  }

  return false;
}

void BufferReaderBase::stop(void)
{
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

  produceCallback = NULL;
}

BufferReaderBase::Packet BufferReaderBase::read(bool fast)
{
  if (packetBuffer.isEmpty())
  {
    ::AVPacket avPacket;
    ::av_init_packet(&avPacket);

  #if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
    if (fast)
      formatContext->flags |= AVFMT_FLAG_NOFILLIN;
    else
      formatContext->flags &= ~int(AVFMT_FLAG_NOFILLIN);
  #endif

    if (::av_read_frame(formatContext, &avPacket) >= 0)
      return avPacket;

    return Packet();
  }
  else
    return packetBuffer.takeFirst();
}

STime BufferReaderBase::timeStamp(const Packet &packet) const
{
  STime result;

  if ((packet.streamIndex >= 0) && (unsigned(packet.streamIndex) < formatContext->nb_streams))
  {
    const ::AVStream * const stream = formatContext->streams[packet.streamIndex];
    if (stream)
    {
      if (packet.pts != AV_NOPTS_VALUE)
        result = STime::fromClock(packet.pts, stream->time_base.num, stream->time_base.den);
      else if (packet.dts != AV_NOPTS_VALUE)
        result = STime::fromClock(packet.dts, stream->time_base.num, stream->time_base.den);
    }
  }

  return result;
}

bool BufferReaderBase::demux(const Packet &packet)
{
  if (packet.streamIndex >= 0)
  {
    if (unsigned(packet.streamIndex) < formatContext->nb_streams)
    {
      const ::AVStream * const stream = formatContext->streams[packet.streamIndex];
      if (stream && (streamContext[packet.streamIndex] == NULL))
        streamContext[packet.streamIndex] = initStreamContext(stream);

      StreamContext * const context = streamContext[packet.streamIndex];

      if (stream && context)
      {
        if (stream->codec->codec_type == CODEC_TYPE_AUDIO)
        {
          if (selectedStreams.contains(StreamId(StreamId::Type_Audio, stream->index)))
          {
            // Detect DTS (Digital Theatre Surround) if needed.
            if (!context->dtsChecked &&
                ((stream->codec->codec_id == CODEC_ID_PCM_S16LE) ||
                 (stream->codec->codec_id == CODEC_ID_PCM_S16BE)))
            { // Not checked for DTS yet
              if (isDTS(packet))
              {
                context->needsDTSFraming = true;
                context->dtsBuffer = new quint8[context->dtsBufferSize];
                context->dtsBufferUsed = 0;

                context->audioCodec =
                    SAudioCodec("DTS",
                                FFMpegCommon::fromFFMpegChannelLayout(stream->codec->channel_layout,
                                                                      stream->codec->channels),
                                stream->codec->sample_rate);
              }

              context->dtsChecked = true; // Checked for DTS.
            }

            if (!context->needsDTSFraming)
            {
              SEncodedAudioBuffer buffer(context->audioCodec, packet.memory());

              const QPair<STime, STime> ts = correctTimeStamp(packet);
              buffer.setPresentationTimeStamp(ts.first);
              buffer.setDecodingTimeStamp(ts.second);

//                  qDebug() << "Audio timestamp" << packet.streamIndex
//                      << ", pts = " << buffer.presentationTimeStamp().toMSec()
//                      << ", dts = " << buffer.decodingTimeStamp().toMSec()
//                      << ", ppts = " << packet.pts << ", pdts = " << packet.dts;

              if (produceCallback)
                produceCallback->produce(buffer);
            }
            else foreach (SEncodedAudioBuffer buffer, parseDTSFrames(context, packet))
            { // Do DTS framing
              const QPair<STime, STime> ts = correctTimeStamp(packet);
              buffer.setPresentationTimeStamp(ts.first);
              buffer.setDecodingTimeStamp(ts.second);

//                  qDebug() << "Audio timestamp" << packet.streamIndex
//                      << ", pts = " << buffer.presentationTimeStamp().toMSec()
//                      << ", dts = " << buffer.decodingTimeStamp().toMSec()
//                      << ", ppts = " << packet.pts << ", pdts = " << packet.dts;

              if (produceCallback)
                produceCallback->produce(buffer);
            }
          }
        }
        else if (stream->codec->codec_type == CODEC_TYPE_VIDEO)
        {
          if (selectedStreams.contains(StreamId(StreamId::Type_Video, stream->index)))
          {
            SEncodedVideoBuffer buffer(context->videoCodec, packet.memory());

            const QPair<STime, STime> ts = correctTimeStamp(packet);
            buffer.setPresentationTimeStamp(ts.first);
            buffer.setDecodingTimeStamp(ts.second);

            buffer.setKeyFrame((packet.flags & PKT_FLAG_KEY) != 0);

//                qDebug() << "Video timestamp" << packet.streamIndex
//                    << ", dts =" << buffer.decodingTimeStamp().toMSec()
//                    << ", pts =" << buffer.presentationTimeStamp().toMSec()
//                    << ", key =" << buffer.isKeyFrame();

            if (produceCallback)
              produceCallback->produce(buffer);
          }
        }
        else if (stream->codec->codec_type == CODEC_TYPE_SUBTITLE)
        {
          if (selectedStreams.contains(StreamId(StreamId::Type_Subtitle, stream->index)))
          {
            SEncodedDataBuffer buffer(context->dataCodec, packet.memory());

            const QPair<STime, STime> ts = correctTimeStampToVideo(packet);
            buffer.setPresentationTimeStamp(ts.first);
            buffer.setDecodingTimeStamp(ts.second);
            buffer.setDuration(STime(packet.convergenceDuration, context->timeBase));

//                qDebug() << "Data timestamp" << packet.streamIndex
//                    << ", dts =" << buffer.decodingTimeStamp().toMSec()
//                    << ", pts =" << buffer.presentationTimeStamp().toMSec()
//                    << ", duration =" << buffer.duration().toMSec();

            if (produceCallback)
              produceCallback->produce(buffer);
          }
        }
      }
    }

    return true;
  }
  else
    return false;
}

STime BufferReaderBase::duration(void) const
{
  if (formatContext->duration != AV_NOPTS_VALUE)
    return STime::fromClock(formatContext->duration, AV_TIME_BASE);

  return STime();
}

bool BufferReaderBase::setPosition(STime pos, bool fast)
{
  if (formatContext)
  {
    if (!pos.isNull() || !pos.isValid())
      packetBuffer.clear();
    else if (!packetBuffer.isEmpty())
      return true; // Not started yet and seeking to start.

    if (pos.isValid())
    if (::av_seek_frame(formatContext, -1, pos.toClock(AV_TIME_BASE), fast ? AVSEEK_FLAG_BYTE : 0) >= 0)
    {
      for (unsigned i=0; i<formatContext->nb_streams; i++)
      if (streamContext[i])
        streamContext[i]->lastTimeStamp = STime();

      return true;
    }
  }

  return false;
}

STime BufferReaderBase::position(void) const
{
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (streamContext[i])
  if (streamContext[i]->lastTimeStamp.isValid() && streamContext[i]->firstTimeStamp.isValid())
    return streamContext[i]->lastTimeStamp - streamContext[i]->firstTimeStamp;

  return STime();
}

QList<BufferReaderBase::Chapter> BufferReaderBase::chapters(void) const
{
  QMultiMap<STime, Chapter> chapters;

  if (formatContext)
  for (unsigned i=0; i<formatContext->nb_chapters; i++)
  {
    const SInterval interval(formatContext->chapters[i]->time_base.num, formatContext->chapters[i]->time_base.den);

    Chapter chapter;
    if (formatContext->chapters[i]->title)
      chapter.title = formatContext->chapters[i]->title;

    chapter.begin = STime(formatContext->chapters[i]->start, interval);
    chapter.end = STime(formatContext->chapters[i]->end, interval);

    chapters.insert(chapter.begin, chapter);
  }

  return chapters.values();
}

QList<BufferReaderBase::AudioStreamInfo> BufferReaderBase::audioStreams(void) const
{
  QList<AudioStreamInfo> streams;

  if (formatContext)
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
  if (streamContext[i])
  {
    streams += AudioStreamInfo(
        formatContext->streams[i]->index,
        formatContext->streams[i]->language,
        readMetadata(formatContext->streams[i]->metadata, "title"),
        streamContext[i]->audioCodec);
  }

  return streams;
}

QList<BufferReaderBase::VideoStreamInfo> BufferReaderBase::videoStreams(void) const
{
  QList<VideoStreamInfo> streams;

  if (formatContext)
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
  if (streamContext[i])
  {
    streams += VideoStreamInfo(
        formatContext->streams[i]->index,
        formatContext->streams[i]->language,
        readMetadata(formatContext->streams[i]->metadata, "title"),
        streamContext[i]->videoCodec);
  }

  return streams;
}

QList<BufferReaderBase::DataStreamInfo> BufferReaderBase::dataStreams(void) const
{
  QList<DataStreamInfo> streams;

  if (formatContext)
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_SUBTITLE)
  if (streamContext[i])
  {
    streams += DataStreamInfo(
        DataStreamInfo::Type_Subtitle,
        formatContext->streams[i]->index,
        formatContext->streams[i]->language,
        readMetadata(formatContext->streams[i]->metadata, "title"),
        streamContext[i]->dataCodec);
  }

  return streams;
}

void BufferReaderBase::selectStreams(const QList<StreamId> &streams)
{
  selectedStreams = QSet<StreamId>::fromList(streams);
}

/*! Returns true if the data contains DTS data.
 */
bool BufferReaderBase::isDTS(const SBuffer &buffer)
{
  const quint8 *inPtr = reinterpret_cast<const quint8 *>(buffer.data());
  int inSize = buffer.size();

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
int BufferReaderBase::findDTSFrame(const quint8 *inPtr, int inSize)
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
unsigned BufferReaderBase::findDTSFrameType(const quint8 *inPtr)
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

SEncodedAudioBufferList BufferReaderBase::parseDTSFrames(StreamContext *context, const SBuffer &buffer)
{
  const quint8 *inPtr = reinterpret_cast<const quint8 *>(buffer.data());
  int inSize = buffer.size();

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

BufferReaderBase::StreamContext * BufferReaderBase::initStreamContext(const ::AVStream *stream)
{
  StreamContext * const streamContext = new StreamContext();
  streamContext->timeBase = SInterval(stream->time_base.num, stream->time_base.den);
  streamContext->dtsChecked = false;
  streamContext->needsDTSFraming = false;
  streamContext->firstTimeStamp = STime();
  streamContext->lastTimeStamp = STime();
  streamContext->timeStampGap = STime::null;
  streamContext->dtsBuffer = NULL;
  streamContext->dtsBufferUsed = 0;

  if (stream->codec->codec_type == CODEC_TYPE_AUDIO)
  {
    streamContext->audioCodec =
        SAudioCodec(FFMpegCommon::fromFFMpegCodecID(stream->codec->codec_id),
                    FFMpegCommon::fromFFMpegChannelLayout(stream->codec->channel_layout, stream->codec->channels),
                    stream->codec->sample_rate,
                    stream->codec->bit_rate);

    if (stream->codec->extradata && (stream->codec->extradata_size > 0))
      streamContext->audioCodec.setExtraData(
          QByteArray((const char *)stream->codec->extradata,
                     stream->codec->extradata_size));
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

    streamContext->videoCodec =
        SVideoCodec(FFMpegCommon::fromFFMpegCodecID(stream->codec->codec_id),
                    SSize(stream->codec->width, stream->codec->height, ar),
                    fr,
                    stream->codec->bit_rate);

    if (stream->codec->extradata && (stream->codec->extradata_size > 0))
      streamContext->videoCodec.setExtraData(
          QByteArray((const char *)stream->codec->extradata,
                     stream->codec->extradata_size));

    streamContext->measurement.reserve(streamContext->measurementSize);
  }
  else if (stream->codec->codec_type == CODEC_TYPE_SUBTITLE)
  {
    streamContext->dataCodec =
        SDataCodec(FFMpegCommon::fromFFMpegCodecID(stream->codec->codec_id));

    if (stream->codec->extradata && (stream->codec->extradata_size > 0))
      streamContext->dataCodec.setExtraData(
          QByteArray((const char *)stream->codec->extradata,
                     stream->codec->extradata_size));
  }

  return streamContext;
}

QString BufferReaderBase::readMetadata(::AVMetadata *metadata, const char *tagName)
{
  if (metadata)
  {
    AVMetadataTag * const tag = ::av_metadata_get(metadata, tagName, NULL, 0);
    if (tag && tag->value)
      return QString::fromUtf8(tag->value);
  }

  return QString::null;
}

QPair<STime, STime> BufferReaderBase::correctTimeStamp(const Packet &packet)
{
  StreamContext * const context = streamContext[packet.streamIndex];

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

QPair<STime, STime> BufferReaderBase::correctTimeStampToVideo(const Packet &packet)
{
  StreamContext * const context = streamContext[packet.streamIndex];

  // Ensure the timestamps are zero-based. Non-zero bases streams can occur
  // when captured from DVB or a sample has been cut from a larger file.
  STime subtract = STime::null;
  for (unsigned i=0; i<formatContext->nb_streams; i++)
  if (formatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
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

BufferReaderBase::Packet::Packet(void)
  : SBuffer(),
    pts(AV_NOPTS_VALUE), dts(AV_NOPTS_VALUE),
    pos(-1),
    convergenceDuration(0),
    streamIndex(-1),
    flags(0),
    duration(0)
{
}

BufferReaderBase::Packet::Packet(const ::AVPacket &from)
  : SBuffer(reinterpret_cast<const char *>(from.data), from.size),
    pts(from.pts), dts(from.dts),
    pos(from.pos),
    convergenceDuration(from.convergence_duration),
    streamIndex(from.stream_index),
    flags(from.flags),
    duration(from.duration)
{
}

} } // End of namespaces
