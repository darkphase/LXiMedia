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

#include "bufferreaderbase.h"

namespace LXiStream {
namespace FFMpegBackend {

const int   BufferReaderBase::StreamContext::measurementSize = 96;
const int   BufferReaderBase::maxBufferCount = StreamContext::measurementSize * 8;
const STime BufferReaderBase::maxJumpTime = STime::fromSec(10);

BufferReaderBase::BufferReaderBase(void)
  : produceCallback(NULL),
    formatContext(NULL)
{
}

BufferReaderBase::~BufferReaderBase()
{
  clear();
}

::AVStream * BufferReaderBase::getStream(int index) const
{
  if (formatContext && (index < int(formatContext->nb_streams)))
    return formatContext->streams[index];

  return NULL;
}

bool BufferReaderBase::start(SInterfaces::AbstractBufferReader::ProduceCallback *produceCallback, ::AVFormatContext *formatContext, bool fast)
{
  this->produceCallback = produceCallback;
  this->formatContext = formatContext;

  if (!fast)
    formatContext->flags |= AVFMT_FLAG_GENPTS;

  formatContext->probesize = fast ? 524288 : 4194304;
  formatContext->max_analyze_duration = fast ? (AV_TIME_BASE / 4) : (AV_TIME_BASE * 2);

  if (::avformat_find_stream_info(formatContext, NULL) >= 0)
  {
    //::dump_format(formatContext, 0, "", false);

    bool hasAudio = false, hasVideo = false, hasSubtitle = false;

    for (unsigned i=0; i<formatContext->nb_streams; i++)
    {
      const ::AVStream * const stream = formatContext->streams[i];

      streamContext += initStreamContext(stream);

      if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
      {
        if (!hasAudio)
            selectedStreams += SInterfaces::AbstractBufferReader::StreamId(SInterfaces::AbstractBufferReader::StreamId::Type_Audio, stream->index);

        hasAudio = true;
      }
      else if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
      {
        if (!hasVideo)
            selectedStreams += SInterfaces::AbstractBufferReader::StreamId(SInterfaces::AbstractBufferReader::StreamId::Type_Video, stream->index);

        hasVideo = true;
      }
      else if (stream->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
      {
        if (!hasSubtitle)
            selectedStreams += SInterfaces::AbstractBufferReader::StreamId(SInterfaces::AbstractBufferReader::StreamId::Type_Subtitle, stream->index);

        hasSubtitle = true;
      }
    }

    QList<Packet> readAheadBuffer;
    bool prependBuffer = false;
    for (int i=0, f=0; (i<maxBufferCount) && (f==0); i++)
    {
      const Packet packet = read();
      if (packet.streamIndex >= 0)
      {
        if (packet.streamIndex < streamContext.count())
        {
          correctTimeStamp(packet);

          if (hasVideo)
          {
            const ::AVStream * const stream = formatContext->streams[packet.streamIndex];
            StreamContext * const context = streamContext[packet.streamIndex];

            if (stream && context)
            {
              if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
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
                    if (context->measurement.isEmpty())
                      prependBuffer = true;

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
                  f = 1; // Finished
              }
            }
          }
          else
          {
            // Continue until all streams have been seen.
            f = 1;
            foreach (StreamContext *context, streamContext)
            if (!context->lastTimeStamp.isValid())
              f = 0;
          }
        }
        else
          break;
      }

      // Make sure a video buffer is first, to properly correct the timestamps.
      if (prependBuffer)
      {
        readAheadBuffer.prepend(packet);
        prependBuffer = false;
      }
      else
        readAheadBuffer.append(packet);
    }

    // And determine the framerate for each video stream          .
    if (hasVideo)
    for (int i=0; i<streamContext.count(); i++)
    if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
      if (streamContext[i]->measurement.count() >= streamContext[i]->measurementSize)
      {
        qSort(streamContext[i]->measurement);

        // Get the frame intervals (skip first 4 frames)
        QMap<qint64, QAtomicInt> intervals;
        for (int j=5; j<streamContext[i]->measurementSize; j++)
          intervals[(streamContext[i]->measurement[j] - streamContext[i]->measurement[j-1]).toUSec()].ref();

        // Keep only the ones that reoccur
        int maxCount = 0;
        for (QMap<qint64, QAtomicInt>::ConstIterator j=intervals.begin(); j!=intervals.end(); j++)
          maxCount = qMax(j->load(), maxCount);

        QMap<qint64, double> filteredIntervals;
        for (QMap<qint64, QAtomicInt>::ConstIterator j=intervals.begin(); j!=intervals.end(); j++)
        if ((j.key() > 10000) && (j.key() < 100000))
        {
//          qDebug() << "Interval" << j.key() << j->load() << maxCount;

          if (j->load() > ((maxCount / 2) + (maxCount / 4)))
            filteredIntervals[j.key()] = double(maxCount);
          else if (j->load() > ((maxCount / 3) + (maxCount / 9)))
            filteredIntervals[j.key()] = double(maxCount) / 2.0;
          else if (j->load() > ((maxCount / 4) + (maxCount / 16)))
            filteredIntervals[j.key()] = double(maxCount) / 3.0;
          else if ((j->load() > ((maxCount + 1) / 16)) && (j->load() < ((maxCount + 1) / 8)))
            filteredIntervals[j.key()] = double(maxCount + 1) / 12.0;
        }

        // Compute frame rate
        if (!filteredIntervals.isEmpty())
        {
          double sum = 0.0, count = 0.0;
          for (QMap<qint64, double>::ConstIterator j=filteredIntervals.begin(); j!=filteredIntervals.end(); j++)
          {
            sum += double(j.key()) * j.value();
            count += j.value();
          }

          const SInterval refFrameRate = streamContext[i]->videoCodec.frameRate();
          SInterval frameRate(qint64(sum), qint64(count * 1000000.0));
          qDebug() << "BufferReaderBase::start framerate =" << frameRate.toFrequency()
                   << ", refFrameRate =" << refFrameRate.toFrequency();

          // Check if a standard framerate is a better match.
          static const double standard[] = { 10.0, 12.5, 15.0, 24.0, 25.0, 30.0, 50.0, 60.0 };
          for (size_t j=0; j<(sizeof(standard)/sizeof(*standard)); j++)
          if (qAbs(standard[j] - frameRate.toFrequency()) < 0.2)
            frameRate = SInterval::fromFrequency(standard[j]);

          // Check if a 1/1, 1/2, 1/3 or 1/4 of the refFrameRate is a better match.
          for (int j=1; j<=4; j++)
          if (qAbs((refFrameRate.toFrequency() / j) - frameRate.toFrequency()) < 0.8)
            frameRate = SInterval(refFrameRate.num() * j, refFrameRate.den());

          qDebug() << "BufferReaderBase::start setFrameRate" << frameRate.toFrequency();
          streamContext[i]->videoCodec.setFrameRate(frameRate.simplified());
        }
      }

      streamContext[i]->measurement.clear();
    }

    foreach (StreamContext *context, streamContext)
    if (context->lastTimeStamp.isValid())
      context->lastTimeStamp = STime::null;

    packetBuffer = readAheadBuffer;

    return true;
  }

  return false;
}

void BufferReaderBase::stop(void)
{
  clear();
}


QString BufferReaderBase::formatName(void) const
{
  if (formatContext)
  if (formatContext->iformat)
    return formatContext->iformat->long_name;

  return QString::null;
}

BufferReaderBase::Packet BufferReaderBase::read(void)
{
  if (packetBuffer.isEmpty())
  {
    Packet result;

    ::AVPacket avPacket;
    ::av_init_packet(&avPacket);
    avPacket.data = NULL;
    avPacket.size = 0;

    if (::av_read_frame(formatContext, &avPacket) >= 0)
    {
      result = Packet(avPacket);
      ::av_free_packet(&avPacket);
    }

    return result;
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
      if (stream)
      {
        // Add a new stream context if the stream is new.
        if ((streamContext.count() <= packet.streamIndex) ||
            (streamContext[packet.streamIndex] == NULL))
        {
          for (int i=streamContext.count(); i<packet.streamIndex; i++)
            streamContext += NULL;

          if (packet.streamIndex >= streamContext.count())
            streamContext += initStreamContext(stream);
          else
            streamContext[packet.streamIndex] = initStreamContext(stream);
        }

        StreamContext * const context = streamContext[packet.streamIndex];
        if (context)
        {
          if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
          {
            if (isSelected(stream))
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
                                  stream->codec->sample_rate,
                                  stream->index);
                }

                context->dtsChecked = true; // Checked for DTS.
              }

              if (!context->needsDTSFraming)
              {
                SEncodedAudioBuffer buffer(context->audioCodec, packet.memory());

                const QPair<STime, STime> ts = correctTimeStampToVideo(packet);
                buffer.setPresentationTimeStamp(ts.first);
                buffer.setDecodingTimeStamp(ts.second);
                buffer.setDuration(STime(packet.duration, context->timeBase));

//                  qDebug() << "Audio timestamp" << packet.streamIndex
//                      << ", pts = " << buffer.presentationTimeStamp().toMSec()
//                      << ", dts = " << buffer.decodingTimeStamp().toMSec()
//                      << ", ppts = " << packet.pts << ", pdts = " << packet.dts
//                      << ", duration =" << buffer.duration().toMSec();

                if (produceCallback)
                  produceCallback->produce(buffer);
              }
              else foreach (SEncodedAudioBuffer buffer, parseDTSFrames(context, packet))
              { // Do DTS framing
                const QPair<STime, STime> ts = correctTimeStampToVideo(packet);
                buffer.setPresentationTimeStamp(ts.first);
                buffer.setDecodingTimeStamp(ts.second);
                buffer.setDuration(STime(packet.duration, context->timeBase));

//                qDebug() << "Audio timestamp" << packet.streamIndex
//                    << ", pts = " << buffer.presentationTimeStamp().toMSec()
//                    << ", dts = " << buffer.decodingTimeStamp().toMSec()
//                    << ", ppts = " << packet.pts << ", pdts = " << packet.dts
//                    << ", duration =" << buffer.duration().toMSec();

                if (produceCallback)
                  produceCallback->produce(buffer);
              }
            }
          }
          else if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
          {
            if (isSelected(stream))
            {
              SEncodedVideoBuffer buffer(context->videoCodec, packet.memory());

              const QPair<STime, STime> ts = correctTimeStamp(packet);
              buffer.setPresentationTimeStamp(ts.first);
              buffer.setDecodingTimeStamp(ts.second);
              buffer.setDuration(STime(packet.duration, context->timeBase));

              buffer.setKeyFrame((packet.flags & AV_PKT_FLAG_KEY) != 0);

//              qDebug() << "Video timestamp" << packet.streamIndex
//                  << ", dts =" << buffer.decodingTimeStamp().toMSec()
//                  << ", pts =" << buffer.presentationTimeStamp().toMSec()
//                  << ", duration =" << buffer.duration().toMSec()
//                  << ", ppts = " << packet.pts << ", pdts = " << packet.dts
//                  << ", key =" << buffer.isKeyFrame();

              if (produceCallback)
                produceCallback->produce(buffer);
            }
          }
          else if (stream->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
          {
            if (isSelected(stream))
            {
              SEncodedDataBuffer buffer(context->dataCodec, packet.memory());

              const QPair<STime, STime> ts = correctTimeStampToVideoOnly(packet);
              buffer.setPresentationTimeStamp(ts.first);
              buffer.setDecodingTimeStamp(ts.second);
              buffer.setDuration(STime(packet.convergenceDuration, context->timeBase));

//              qDebug() << "Data timestamp" << packet.streamIndex
//                  << ", dts =" << buffer.decodingTimeStamp().toMSec()
//                  << ", pts =" << buffer.presentationTimeStamp().toMSec()
//                  << ", ppts = " << packet.pts << ", pdts = " << packet.dts
//                  << ", duration =" << buffer.duration().toMSec();

              if (produceCallback)
                produceCallback->produce(buffer);
            }
          }
        }
      }
    }

    return true;
  }
  else
    return false;
}

bool BufferReaderBase::buffer(void)
{
  formatContext->flags &= ~int(AVFMT_FLAG_NOFILLIN);

  ::AVPacket avPacket;
  ::av_init_packet(&avPacket);
  if (::av_read_frame(formatContext, &avPacket) >= 0)
  {
    packetBuffer.append(avPacket);
    ::av_free_packet(&avPacket);
    return true;
  }

  return false;
}

STime BufferReaderBase::bufferDuration(void) const
{
  const int count = packetBuffer.count();
  if (count > 0)
  {
    STime first = timeStamp(packetBuffer.first());
    STime last = timeStamp(packetBuffer.last());
    for (int i=1; (i<16) && (i<count); i++)
    {
      first = qMin(first, timeStamp(packetBuffer[i]));
      last = qMax(last, timeStamp(packetBuffer[count - i - 1]));
    }

    if (first.isValid() && last.isValid())
      return last - first;
  }

  return STime();
}

STime BufferReaderBase::duration(void) const
{
  if (formatContext->duration != AV_NOPTS_VALUE)
    return STime::fromClock(formatContext->duration, AV_TIME_BASE);

  return STime();
}

bool BufferReaderBase::setPosition(STime pos)
{
  if (formatContext)
  {
    if (!pos.isNull() || !pos.isValid())
      packetBuffer.clear();
    else if (!packetBuffer.isEmpty())
      return true; // Not started yet and seeking to start.

    if (pos.isValid())
    if (::av_seek_frame(formatContext, -1, pos.toClock(AV_TIME_BASE), 0) >= 0)
    {
      foreach (StreamContext *context, streamContext)
      if (context)
        context->lastTimeStamp = STime::null;

      return true;
    }
  }

  return false;
}

STime BufferReaderBase::position(void) const
{
  foreach (StreamContext *context, streamContext)
  if (context)
  if (context->lastTimeStamp.isValid())
    return context->lastTimeStamp;

  return STime();
}

QList<SInterfaces::AbstractBufferReader::Chapter> BufferReaderBase::chapters(void) const
{
  QMultiMap<STime, SInterfaces::AbstractBufferReader::Chapter> chapters;

  if (formatContext)
  for (unsigned i=0; i<formatContext->nb_chapters; i++)
  {
    const SInterval interval(formatContext->chapters[i]->time_base.num, formatContext->chapters[i]->time_base.den);

    SInterfaces::AbstractBufferReader::Chapter chapter;
    chapter.title = readMetadata(formatContext->chapters[i]->metadata, "title");

    chapter.begin = STime(formatContext->chapters[i]->start, interval);
    chapter.end = STime(formatContext->chapters[i]->end, interval);

    chapters.insert(chapter.begin, chapter);
  }

  return chapters.values();
}

QList<SInterfaces::AbstractBufferReader::AudioStreamInfo> BufferReaderBase::audioStreams(void) const
{
  QList<SInterfaces::AbstractBufferReader::AudioStreamInfo> streams;

  if (formatContext)
  for (int i=0; i<streamContext.count(); i++)
  if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
  if (streamContext[i])
  {
    SInterfaces::AbstractBufferReader::AudioStreamInfo streamInfo(
        SInterfaces::AbstractBufferReader::StreamId(
            SInterfaces::AbstractBufferReader::StreamId::Type_Audio,
            formatContext->streams[i]->index),
        readMetadata(formatContext->streams[i]->metadata, "language"),
        readMetadata(formatContext->streams[i]->metadata, "title"),
        streamContext[i]->audioCodec);

    if (formatContext->streams[i]->id > 0)
    {
      streamInfo.type |= SInterfaces::AbstractBufferReader::StreamId::Type_Flag_Native;
      streamInfo.id = formatContext->streams[i]->id;
    }

    streams += streamInfo;
  }

  return streams;
}

QList<SInterfaces::AbstractBufferReader::VideoStreamInfo> BufferReaderBase::videoStreams(void) const
{
  QList<SInterfaces::AbstractBufferReader::VideoStreamInfo> streams;

  if (formatContext)
  for (int i=0; i<streamContext.count(); i++)
  if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
  if (streamContext[i])
  {
    SInterfaces::AbstractBufferReader::VideoStreamInfo streamInfo(
        SInterfaces::AbstractBufferReader::StreamId(
            SInterfaces::AbstractBufferReader::StreamId::Type_Video,
            formatContext->streams[i]->index),
        readMetadata(formatContext->streams[i]->metadata, "language"),
        readMetadata(formatContext->streams[i]->metadata, "title"),
        streamContext[i]->videoCodec);

    if (formatContext->streams[i]->id > 0)
    {
      streamInfo.type |= SInterfaces::AbstractBufferReader::StreamId::Type_Flag_Native;
      streamInfo.id = formatContext->streams[i]->id;
    }

    streams += streamInfo;
  }

  return streams;
}

QList<SInterfaces::AbstractBufferReader::DataStreamInfo> BufferReaderBase::dataStreams(void) const
{
  QList<SInterfaces::AbstractBufferReader::DataStreamInfo> streams;

  if (formatContext)
  for (int i=0; i<streamContext.count(); i++)
  if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
  if (streamContext[i])
  {
    SInterfaces::AbstractBufferReader::DataStreamInfo streamInfo(
        SInterfaces::AbstractBufferReader::StreamId(
            SInterfaces::AbstractBufferReader::DataStreamInfo::Type_Subtitle,
            formatContext->streams[i]->index),
        readMetadata(formatContext->streams[i]->metadata, "language"),
        readMetadata(formatContext->streams[i]->metadata, "title"),
        streamContext[i]->dataCodec);

    if (formatContext->streams[i]->id > 0)
    {
      streamInfo.type |= SInterfaces::AbstractBufferReader::StreamId::Type_Flag_Native;
      streamInfo.id = formatContext->streams[i]->id;
    }

    streams += streamInfo;
  }

  return streams;
}

void BufferReaderBase::selectStreams(const QVector<SInterfaces::AbstractBufferReader::StreamId> &streams)
{
  selectedStreams = streams;
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

void BufferReaderBase::clear(void)
{
  foreach (StreamContext *context, streamContext)
  if (context)
  {
    delete [] context->dtsBuffer;
    delete context;
  }

  streamContext.clear();

  if (formatContext)
  {
    for (unsigned i=0; i<formatContext->nb_streams; i++)
    if (formatContext->streams[i]->codec->codec)
      ::avcodec_close(formatContext->streams[i]->codec);

    ::avformat_close_input(&formatContext);
  }

  produceCallback = NULL;
}

BufferReaderBase::StreamContext * BufferReaderBase::initStreamContext(const ::AVStream *stream)
{
  StreamContext * const streamContext = new StreamContext();
  streamContext->timeBase = SInterval(stream->time_base.num, stream->time_base.den);
  streamContext->dtsChecked = false;
  streamContext->needsDTSFraming = false;
  streamContext->lastTimeStamp = STime();
  streamContext->timeStampGap = STime::null;
  streamContext->dtsBuffer = NULL;
  streamContext->dtsBufferUsed = 0;

  if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
  {
    ::AVCodec * const codec = ::avcodec_find_decoder(stream->codec->codec_id);
    if (codec)
    {
      streamContext->audioCodec = SAudioCodec(
          codec->name,
          FFMpegCommon::fromFFMpegChannelLayout(
              stream->codec->channel_layout,
              stream->codec->channels),
          stream->codec->sample_rate,
          stream->index,
          stream->codec->bit_rate);
    }
  }
  else if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
  {
    float ar = 1.0f;
    if ((stream->codec->sample_aspect_ratio.den != 0) &&
        (stream->codec->sample_aspect_ratio.num != 0))
    {
      ar = ::av_q2d(stream->codec->sample_aspect_ratio);
    }

    if (qFuzzyCompare(ar, 1.0f) &&
        (stream->sample_aspect_ratio.den != 0) &&
        (stream->sample_aspect_ratio.num != 0))
    {
      ar = ::av_q2d(stream->sample_aspect_ratio);
    }

    // Note that den and num are deliberately swapped! (see documentation of AVStream::r_frame_rate)
    const SInterval fr(stream->r_frame_rate.den, stream->r_frame_rate.num);

    ::AVCodec * const codec = ::avcodec_find_decoder(stream->codec->codec_id);
    if (codec)
    {
      streamContext->videoCodec = SVideoCodec(
          codec->name,
          SSize(stream->codec->width, stream->codec->height, ar),
          fr,
          stream->index,
          stream->codec->bit_rate);
    }

    streamContext->measurement.reserve(streamContext->measurementSize);
  }
  else if (stream->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
  {
    if (stream->codec->codec_id == CODEC_ID_TEXT)
    {
      streamContext->dataCodec = SDataCodec(
          "sub_rawutf8",
          QByteArray(),
          stream->index);
    }
    else
    {
      ::AVCodec * const codec = ::avcodec_find_decoder(stream->codec->codec_id);
      if (codec)
      {
        streamContext->dataCodec = SDataCodec(
            codec->name,
            QByteArray(),
            stream->index);
      }
    }
  }

  return streamContext;
}

QString BufferReaderBase::readMetadata(::AVDictionary *dictionary, const char *tagName)
{
  if (dictionary)
  {
    AVDictionaryEntry * const entry = ::av_dict_get(dictionary, tagName, NULL, 0);
    if (entry && entry->value)
      return QString::fromUtf8(entry->value);
  }

  return QString::null;
}

bool BufferReaderBase::isSelected(const ::AVStream *stream) const
{
  foreach (const SInterfaces::AbstractBufferReader::StreamId &selectedStream, selectedStreams)
  {
    if ((selectedStream.type & SInterfaces::AbstractBufferReader::StreamId::Type_Flag_Native) != 0)
    {
      if (selectedStream.id == stream->id)
        return true;
    }
    else if (selectedStream.id == stream->index)
      return true;
  }

  return false;
}

QPair<STime, STime> BufferReaderBase::correctTimeStamp(const Packet &packet)
{
  StreamContext * const context = streamContext[packet.streamIndex];

  // Determine the baseTimeStamp, decodingTimeStamp and presentationTimeStamp.
  STime baseTimeStamp, decodingTimeStamp, presentationTimeStamp;

  if (packet.dts != AV_NOPTS_VALUE)
  {
    baseTimeStamp = decodingTimeStamp =
                    STime(packet.dts, context->timeBase) - context->timeStampGap;

    if (packet.pts != AV_NOPTS_VALUE)
    {
      presentationTimeStamp =
          STime(packet.pts, context->timeBase) - context->timeStampGap;

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
        STime(packet.pts, context->timeBase) - context->timeStampGap;
  }
  else
    baseTimeStamp = context->lastTimeStamp;

  if (context->lastTimeStamp.isValid())
  {
    if (!context->lastTimeStamp.isNull())
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
  }
  else
  {
    context->timeStampGap = baseTimeStamp;
    baseTimeStamp -= context->timeStampGap;

    if (decodingTimeStamp.isValid())
      decodingTimeStamp -= context->timeStampGap;

    if (presentationTimeStamp.isValid())
      presentationTimeStamp -= context->timeStampGap;
  }

  context->lastTimeStamp = baseTimeStamp;

  return QPair<STime, STime>(presentationTimeStamp, decodingTimeStamp);
}

QPair<STime, STime> BufferReaderBase::correctTimeStampToVideo(const Packet &packet)
{
  StreamContext * const context = streamContext[packet.streamIndex];

  STime timeStampGap;
  for (int i=0; i<streamContext.count(); i++)
  if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
  if (streamContext[i])
  {
    timeStampGap = streamContext[i]->timeStampGap;
    break;
  }

  if (timeStampGap.isValid() && (qAbs(context->timeStampGap - timeStampGap).toSec() < 15))
  {
    // Determine the decodingTimeStamp and presentationTimeStamp.
    STime decodingTimeStamp, presentationTimeStamp;

    if (packet.dts != AV_NOPTS_VALUE)
      decodingTimeStamp = STime(packet.dts, context->timeBase) - timeStampGap;

    if (packet.pts != AV_NOPTS_VALUE)
      presentationTimeStamp = STime(packet.pts, context->timeBase) - timeStampGap;

    return qMakePair(presentationTimeStamp, decodingTimeStamp);
  }
  else
    return correctTimeStamp(packet);
}

QPair<STime, STime> BufferReaderBase::correctTimeStampToVideoOnly(const Packet &packet) const
{
  StreamContext * const context = streamContext[packet.streamIndex];

  STime timeStampGap = STime::null;
  for (int i=0; i<streamContext.count(); i++)
  if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
  if (streamContext[i])
  {
    timeStampGap = streamContext[i]->timeStampGap;
    break;
  }

  // Determine the decodingTimeStamp and presentationTimeStamp.
  STime decodingTimeStamp, presentationTimeStamp;

  if (packet.dts != AV_NOPTS_VALUE)
    decodingTimeStamp = STime(packet.dts, context->timeBase) - timeStampGap;

  if (packet.pts != AV_NOPTS_VALUE)
    presentationTimeStamp = STime(packet.pts, context->timeBase) - timeStampGap;

  return qMakePair(presentationTimeStamp, decodingTimeStamp);
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
