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
    hasAudio(false), hasVideo(false),
    mpegClock(false)
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
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
  format = ::guess_stream_format(name.toAscii().data(), NULL, NULL);
#else
  format = ::av_guess_format(name.toAscii().data(), NULL, NULL);
#endif
  if (format)
  {
    formatContext = ::avformat_alloc_context();
    formatContext->oformat = format;

    ::AVFormatParameters formatParameters;
    memset(&formatParameters, 0, sizeof(formatParameters));
    if (::av_set_parameters(formatContext, &formatParameters) < 0)
      qCritical() << "BufferWriter::openFormat invalid ouptut format parameters.";

    formatContext->bit_rate = 0;

    if ((name == "dvd") || (name == "mp2") || (name == "mp3") ||
        (name == "mp4") || (name == "mpeg") || (name == "mpeg1video") ||
        (name == "mpeg2video") || (name == "mpegts") || (name == "mpegtsraw") ||
        (name == "mpegvideo") || (name == "svcd") || (name == "vcd") ||
        (name == "vob"))
    {
      mpegClock = true;
    }

    if (name == "dvd")
    {
      formatContext->mux_rate = 10080000;
      formatContext->packet_size = 2048;
    }

    formatContext->preload = int(0.5 * AV_TIME_BASE);
    formatContext->max_delay = int(0.7 * AV_TIME_BASE);

    return true;
  }

  return false;
}

bool BufferWriter::createStreams(const QList<SAudioCodec> &audioCodecs, const QList<SVideoCodec> &videoCodecs, STime duration)
{
  if (formatContext)
  {
    foreach (const SVideoCodec &codec, videoCodecs)
    if (!codec.isNull())
    {
      AVStream *stream  = ::av_new_stream(formatContext, streams.count());
      ::avcodec_get_context_defaults2(stream->codec, CODEC_TYPE_VIDEO);

      const SInterval frameRate = codec.frameRate();
      if (mpegClock || !frameRate.isValid())
      {
        stream->time_base.num = 1;
        stream->time_base.den = 90000;
      }
      else
      {
        stream->time_base.num = frameRate.num();
        stream->time_base.den = frameRate.den();
      }

      if (duration.isValid())
        stream->duration = duration.toClock(stream->time_base.num, stream->time_base.den);

      if (frameRate.isValid())
      {
        stream->r_frame_rate.num = stream->avg_frame_rate.num = frameRate.num();
        stream->r_frame_rate.den = stream->avg_frame_rate.den = frameRate.den();
      }

      stream->pts.val = 0;
      stream->pts.num = stream->time_base.num;
      stream->pts.den = stream->time_base.den;

      stream->codec->codec_id = FFMpegCommon::toFFMpegCodecID(codec);
      stream->codec->codec_type = CODEC_TYPE_VIDEO;
      stream->codec->time_base = stream->time_base;
      stream->codec->bit_rate = codec.bitRate() > 0 ? codec.bitRate() : 20000000;
      stream->codec->width = codec.size().width();
      stream->codec->height = codec.size().height();
      stream->codec->sample_aspect_ratio = stream->sample_aspect_ratio = ::av_d2q(codec.size().aspectRatio(), 256);

      if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
          stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

      stream->codec->extradata_size = codec.extraData().size();
      if (stream->codec->extradata_size > 0)
      {
        stream->codec->extradata = new uint8_t[stream->codec->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE];
        memcpy(stream->codec->extradata, codec.extraData().data(), stream->codec->extradata_size);
        memset(stream->codec->extradata + stream->codec->extradata_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
      }

      formatContext->bit_rate += stream->codec->bit_rate;

      hasVideo = true;
      streams.insert(videoStreamId + 0, stream);
    }

    foreach (const SAudioCodec &codec, audioCodecs)
    if (!codec.isNull())
    {
      ::AVStream *stream  = ::av_new_stream(formatContext, streams.count());
      ::avcodec_get_context_defaults2(stream->codec, CODEC_TYPE_AUDIO);

      const unsigned sampleRate = codec.sampleRate();
      if (mpegClock || (sampleRate == 0))
      {
        stream->time_base.num = 1;
        stream->time_base.den = 90000;
      }
      else
      {
        stream->time_base.num = 1;
        stream->time_base.den = sampleRate;
      }

      if (duration.isValid())
        stream->duration = duration.toClock(stream->time_base.num, stream->time_base.den);

      stream->pts.val = 0;
      stream->pts.num = stream->time_base.num;
      stream->pts.den = stream->time_base.den;

      stream->codec->codec_id = FFMpegCommon::toFFMpegCodecID(codec);
      stream->codec->codec_type = CODEC_TYPE_AUDIO;
      stream->codec->time_base = stream->time_base;
      stream->codec->bit_rate = codec.bitRate() > 0 ? codec.bitRate() : 2000000;
      stream->codec->frame_size = codec.frameSize() > 0 ? codec.frameSize() : 8192;
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

      formatContext->bit_rate += stream->codec->bit_rate;

      hasAudio = true;
      streams.insert(audioStreamId + 0, stream);
    }

    if (formatContext->mux_rate <= 0)
      formatContext->mux_rate = formatContext->bit_rate * 8;

    return true;
  }

  return false;
}

bool BufferWriter::start(WriteCallback *c)
{
  static const int ioBufferSize = 65536;

  if (!streams.isEmpty())
  {
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

    //::dump_format(formatContext, 0, NULL, 1);
    ::av_write_header(formatContext);

    return true;
  }

  return false;
}

void BufferWriter::stop(void)
{
  if (!streams.isEmpty())
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
}

void BufferWriter::process(const SEncodedAudioBuffer &buffer)
{
  QMap<quint32, ::AVStream *>::Iterator stream = streams.find(audioStreamId + 0);
  if (stream != streams.end())
  {
    ::AVPacket packet = FFMpegCommon::toAVPacket(buffer, *stream);

    //qDebug() << "A:" << buffer.presentationTimeStamp().toMSec() << packet.pts << packet.dts;

    ::av_interleaved_write_frame(formatContext, &packet);
  }
}

void BufferWriter::process(const SEncodedVideoBuffer &buffer)
{
  QMap<quint32, ::AVStream *>::Iterator stream = streams.find(videoStreamId + 0);
  if (stream != streams.end())
  {
    ::AVPacket packet = FFMpegCommon::toAVPacket(buffer, *stream);

    //qDebug() << "V:" << buffer.presentationTimeStamp().toMSec() << packet.pts << packet.dts;

    ::av_interleaved_write_frame(formatContext, &packet);
  }
}

void BufferWriter::process(const SEncodedDataBuffer &buffer)
{
  QMap<quint32, ::AVStream *>::Iterator stream = streams.find(videoStreamId + 0);
  if (stream != streams.end())
  {
    ::AVPacket packet = FFMpegCommon::toAVPacket(buffer, *stream);

    //qDebug() << "D:" << buffer.presentationTimeStamp().toMSec() << packet.pts << packet.dts;

    ::av_interleaved_write_frame(formatContext, &packet);
  }
}

int BufferWriter::write(void *opaque, uint8_t *buf, int buf_size)
{
  reinterpret_cast<BufferWriter *>(opaque)->callback->write(buf, buf_size);

  return 0;
}

} } // End of namespaces
