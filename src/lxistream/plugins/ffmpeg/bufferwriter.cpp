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

#include "bufferwriter.h"


namespace LXiStream {
namespace FFMpegBackend {


BufferWriter::BufferWriter(const QString &, QObject *parent)
  : SInterfaces::BufferWriter(parent),
    callback(NULL),
    format(NULL),
    formatContext(NULL),
    ioContext(NULL),
    hasAudio(false), hasVideo(false)
{
}

BufferWriter::~BufferWriter()
{
  if (formatContext)
  {
    for (unsigned i=0; i<formatContext->nb_streams; i++)
      ::av_free(formatContext->streams[i]);

    ::av_free(formatContext);
  }

  if (ioContext)
  {
    ::av_free(ioContext->buffer);
    ::av_free(ioContext);
  }
}

bool BufferWriter::openFormat(const QString &name)
{
  format = ::guess_stream_format(name.toAscii().data(), NULL, NULL);
  if (format)
  {
    formatContext = ::avformat_alloc_context();
    formatContext->oformat = format;
    formatContext->preload = AV_TIME_BASE / 100;
    formatContext->max_delay = AV_TIME_BASE / 100;

    return true;
  }

  return false;
}

bool BufferWriter::createStreams(const QList<SAudioCodec> &audioCodecs, const QList<SVideoCodec> &videoCodecs, STime duration)
{
  if (formatContext)
  {
    foreach (const SAudioCodec &codec, audioCodecs)
    if (!codec.isNull())
    {
      ::AVStream *stream  = ::av_new_stream(formatContext, streams.count());
      ::avcodec_get_context_defaults2(stream->codec, CODEC_TYPE_AUDIO);

      const unsigned sampleRate = codec.sampleRate();
      if (sampleRate == 0)
      {
        stream->time_base.num = 1;
        stream->time_base.den = 90000;
      }
      else
      {
        stream->time_base.num = 1;
        stream->time_base.den = sampleRate;
      }

      stream->duration = duration.toClock(stream->time_base.num, stream->time_base.den);

      stream->pts.val = 0;
      stream->pts.num = stream->time_base.num;
      stream->pts.den = stream->time_base.den;

      stream->codec->codec_id = FFMpegCommon::toFFMpegCodecID(codec);
      stream->codec->codec_type = CODEC_TYPE_AUDIO;
      stream->codec->time_base = stream->time_base;
      stream->codec->bit_rate = qMax(64000, codec.bitRate());
      stream->codec->sample_rate = sampleRate;
      stream->codec->channels = codec.numChannels();
      stream->codec->channel_layout = FFMpegCommon::toFFMpegChannelLayout(codec.channelSetup());

      stream->codec->extradata_size = codec.extraData().size();
      if (stream->codec->extradata_size > 0)
      {
        stream->codec->extradata = new uint8_t[stream->codec->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE];
        memcpy(stream->codec->extradata, codec.extraData().data(), stream->codec->extradata_size);
        memset(stream->codec->extradata + stream->codec->extradata_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
      }

      hasAudio = true;
      streams.insert(audioStreamId + 0, stream);
    }

    SInterval minFrameRate;
    foreach (const SVideoCodec &codec, videoCodecs)
    if (!codec.isNull())
    {
      AVStream *stream  = ::av_new_stream(formatContext, streams.count());
      ::avcodec_get_context_defaults2(stream->codec, CODEC_TYPE_VIDEO);

      const SInterval frameRate = codec.frameRate();
      if (!frameRate.isValid())
      {
        stream->time_base.num = 1;
        stream->time_base.den = 90000;
      }
      else
      {
        stream->time_base.num = frameRate.num();
        stream->time_base.den = frameRate.den();

        if (minFrameRate.isValid())
          minFrameRate = frameRate.toFrequency() < minFrameRate.toFrequency() ? frameRate : minFrameRate;
        else
          minFrameRate = frameRate;
      }

      stream->duration = duration.toClock(stream->time_base.num, stream->time_base.den);

      stream->pts.val = 0;
      stream->pts.num = stream->time_base.num;
      stream->pts.den = stream->time_base.den;

      stream->codec->codec_id = FFMpegCommon::toFFMpegCodecID(codec);
      stream->codec->codec_type = CODEC_TYPE_VIDEO;
      stream->codec->time_base = stream->time_base;
      stream->codec->bit_rate = qMax(64000, codec.bitRate());
      stream->codec->width = codec.size().width();
      stream->codec->height = codec.size().height();

      if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
          stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

      stream->codec->extradata_size = codec.extraData().size();
      if (stream->codec->extradata_size > 0)
      {
        stream->codec->extradata = new uint8_t[stream->codec->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE];
        memcpy(stream->codec->extradata, codec.extraData().data(), stream->codec->extradata_size);
        memset(stream->codec->extradata + stream->codec->extradata_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
      }

      hasVideo = true;
      streams.insert(videoStreamId + 0, stream);
    }

    if (minFrameRate.isValid())
    {
      formatContext->preload = 0;
      formatContext->max_delay = AV_TIME_BASE * 4 * minFrameRate.num() / minFrameRate.den();
    }

    return true;
  }

  return false;
}

bool BufferWriter::start(WriteCallback *c)
{
  static const int ioBufferSize = 65536;

  callback = c;
  ioContext =
      ::av_alloc_put_byte((unsigned char *)::av_malloc(ioBufferSize),
                          ioBufferSize,
                          true,
                          this,
                          NULL,
                          &BufferWriter::write,
                          NULL);

  ioContext->is_streamed = true;

  formatContext->pb = ioContext;

  if (::av_set_parameters(formatContext, NULL) < 0)
    qCritical() << "BufferWriter::writeHeader invalid ouptut format parameters.";

  //::dump_format(formatContext, 0, NULL, 1);
  ::av_write_header(formatContext);

  return true;
}

void BufferWriter::stop(void)
{
  ::av_write_trailer(formatContext);

  foreach (::AVStream *stream, streams)
  {
    delete [] stream->codec->extradata;
    ::av_freep(&stream->codec);
    ::av_freep(&stream);
  }

  streams.clear();

  ::av_free(formatContext);
  formatContext = NULL;

  if (ioContext)
  {
    ::av_free(ioContext->buffer);
    ::av_free(ioContext);
    ioContext = NULL;
  }

  callback = NULL;
}

void BufferWriter::process(const SEncodedAudioBuffer &buffer)
{
  QMap<quint32, ::AVStream *>::Iterator stream = streams.find(audioStreamId + 0);
  if (stream != streams.end())
  {
    ::AVPacket packet;
    ::av_init_packet(&packet);

    packet.size = buffer.size();
    packet.data = (uint8_t *)buffer.data();
    packet.stream_index = (*stream)->index;
    packet.pos = -1;

    packet.pts = buffer.presentationTimeStamp().isValid()
                 ? buffer.presentationTimeStamp().toClock((*stream)->time_base.num, (*stream)->time_base.den)
                 : AV_NOPTS_VALUE;
    packet.dts = buffer.decodingTimeStamp().isValid()
                 ? buffer.decodingTimeStamp().toClock((*stream)->time_base.num, (*stream)->time_base.den)
                 : packet.pts;
    
    if (packet.dts > packet.pts) 
      packet.pts = packet.dts;

    //qDebug() << "A:" << buffer.presentationTimeStamp().toMSec() << packet.pts << packet.dts;

    packet.flags |= PKT_FLAG_KEY;

    ::av_interleaved_write_frame(formatContext, &packet);
  }
}

void BufferWriter::process(const SEncodedVideoBuffer &buffer)
{
  QMap<quint32, ::AVStream *>::Iterator stream = streams.find(videoStreamId + 0);
  if (stream != streams.end())
  {
    ::AVPacket packet;
    ::av_init_packet(&packet);

    packet.size = buffer.size();
    packet.data = (uint8_t *)buffer.data();
    packet.stream_index = (*stream)->index;
    packet.pos = -1;

    packet.pts = buffer.presentationTimeStamp().isValid()
                 ? buffer.presentationTimeStamp().toClock((*stream)->time_base.num, (*stream)->time_base.den)
                 : AV_NOPTS_VALUE;
    packet.dts = buffer.decodingTimeStamp().isValid()
                 ? buffer.decodingTimeStamp().toClock((*stream)->time_base.num, (*stream)->time_base.den)
                 : packet.pts;
    
    if (packet.dts > packet.pts) 
      packet.pts = packet.dts;

    //qDebug() << "V:" << buffer.presentationTimeStamp().toMSec() << packet.pts << packet.dts;

    if (buffer.isKeyFrame())
      packet.flags |= PKT_FLAG_KEY;

    ::av_interleaved_write_frame(formatContext, &packet);
  }
}

void BufferWriter::process(const SEncodedDataBuffer &)
{
}

int BufferWriter::write(void *opaque, uint8_t *buf, int buf_size)
{
  reinterpret_cast<BufferWriter *>(opaque)->callback->write(buf, buf_size);

  return 0;
}

} } // End of namespaces
