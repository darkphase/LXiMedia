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
#include "audioencoder.h"
#include "videoencoder.h"

namespace LXiStream {
namespace FFMpegBackend {


BufferWriter::BufferWriter(const QString &, QObject *parent)
  : SInterfaces::BufferWriter(parent),
    callback(NULL),
    format(NULL),
    formatContext(NULL),
    ioContext(NULL),
    sequential(false),
    hasAudio(false), hasVideo(false),
    mpegClock(false), mpegTs(false)
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

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 0, 0)
    ::AVFormatParameters formatParameters;
    memset(&formatParameters, 0, sizeof(formatParameters));
    if (::av_set_parameters(formatContext, &formatParameters) < 0)
      qCritical() << "BufferWriter::openFormat invalid ouptut format parameters.";
#endif

    formatContext->bit_rate = 0;

    if ((name == "dvd") || (name == "mp2") || (name == "mp3") ||
        (name == "mp4") || (name == "mpeg") || (name == "mpeg1video") ||
        (name == "mpeg2video") || (name == "mpegts") || (name == "mpegtsraw") ||
        (name == "mpegvideo") || (name == "svcd") || (name == "vcd") ||
        (name == "vob"))
    {
      mpegClock = true;
    }

    if ((name == "mpegts") || (name == "mpegtsraw"))
    {
      mpegTs = true;
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

::AVStream * BufferWriter::createStream(void)
{
  AVStream * const stream = ::av_new_stream(formatContext, streams.count());
  streams.insert(stream->index, stream);

  return stream;
}

bool BufferWriter::addStream(const SInterfaces::AudioEncoder *encoder, STime duration)
{
  if (encoder)
  {
    ::AVStream *stream = NULL;

    const AudioEncoder * const ffEncoder = qobject_cast<const AudioEncoder *>(encoder);
    if (ffEncoder)
    foreach (::AVStream *s, streams)
    if (s->codec == ffEncoder->avCodecContext())
    {
      stream = s;
      break;
    }

    if (stream == NULL)
    {
      stream = createStream();

      const SAudioCodec baseCodec = encoder->codec();
      const unsigned sampleRate = baseCodec.sampleRate();

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
      ::avcodec_get_context_defaults2(stream->codec, AVMEDIA_TYPE_AUDIO);
#else
      ::avcodec_get_context_defaults2(stream->codec, CODEC_TYPE_AUDIO);
#endif

      stream->codec->codec_id = FFMpegCommon::toFFMpegCodecID(baseCodec);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
      stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
#else
      stream->codec->codec_type = CODEC_TYPE_AUDIO;
#endif
      stream->codec->time_base = stream->time_base;
      stream->codec->bit_rate = baseCodec.bitRate() > 0 ? baseCodec.bitRate() : 2000000;
      stream->codec->frame_size = baseCodec.frameSize() > 0 ? baseCodec.frameSize() : 8192;
      stream->codec->sample_rate = sampleRate;
      stream->codec->channels = baseCodec.numChannels();
      stream->codec->channel_layout = FFMpegCommon::toFFMpegChannelLayout(baseCodec.channelSetup());

      stream->codec->time_base.num = 1;
      stream->codec->time_base.den = sampleRate;
    }

    if (mpegClock || (stream->codec->time_base.num == 0))
    {
      stream->time_base.num = 1;
      stream->time_base.den = 90000;
    }
    else
    {
      stream->time_base.num = stream->codec->time_base.num;
      stream->time_base.den = stream->codec->time_base.den;
    }

    if (duration.isValid())
      stream->duration = duration.toClock(stream->time_base.num, stream->time_base.den);

    stream->pts.val = 0;
    stream->pts.num = stream->time_base.num;
    stream->pts.den = stream->time_base.den;

    formatContext->bit_rate += stream->codec->bit_rate;

    hasAudio = true;

    return true;
  }

  return false;
}

bool BufferWriter::addStream(const SInterfaces::VideoEncoder *encoder, STime duration)
{
  if (encoder)
  {
    ::AVStream *stream = NULL;

    const VideoEncoder * const ffEncoder = qobject_cast<const VideoEncoder *>(encoder);
    if (ffEncoder)
    foreach (::AVStream *s, streams)
    if (s->codec == ffEncoder->avCodecContext())
    {
      stream = s;
      break;
    }

    if (stream == NULL)
    {
      stream = createStream();

      const SVideoCodec baseCodec = encoder->codec();

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
      ::avcodec_get_context_defaults2(stream->codec, AVMEDIA_TYPE_VIDEO);
#else
      ::avcodec_get_context_defaults2(stream->codec, CODEC_TYPE_VIDEO);
#endif

      stream->codec->codec_id = FFMpegCommon::toFFMpegCodecID(baseCodec);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
      stream->codec->codec_type = AVMEDIA_TYPE_VIDEO;
#else
      stream->codec->codec_type = CODEC_TYPE_VIDEO;
#endif
      stream->codec->time_base = stream->time_base;
      stream->codec->bit_rate = baseCodec.bitRate() > 0 ? baseCodec.bitRate() : 20000000;
      stream->codec->width = baseCodec.size().width();
      stream->codec->height = baseCodec.size().height();
      stream->codec->sample_aspect_ratio = ::av_d2q(baseCodec.size().aspectRatio(), 256);

      const SInterval frameRate = baseCodec.frameRate();
      if (frameRate.isValid())
      {
        stream->codec->time_base.num = frameRate.num();
        stream->codec->time_base.den = frameRate.den();
      }
      else
      {
        stream->codec->time_base.num = 1;
        stream->codec->time_base.den = 90000;
      }
    }

    if (mpegClock)
    {
      stream->time_base.num = 1;
      stream->time_base.den = 90000;
    }
    else
    {
      stream->time_base.num = stream->codec->time_base.num;
      stream->time_base.den = stream->codec->time_base.den;
    }

    if (duration.isValid())
      stream->duration = duration.toClock(stream->time_base.num, stream->time_base.den);

    stream->sample_aspect_ratio = stream->codec->sample_aspect_ratio;

    // den/num deliberately swapped.
    stream->r_frame_rate.num = stream->codec->time_base.den;
    stream->r_frame_rate.den = stream->codec->time_base.num;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
    stream->avg_frame_rate.num = stream->r_frame_rate.num;
    stream->avg_frame_rate.den = stream->r_frame_rate.den;
#endif

    stream->pts.val = 0;
    stream->pts.num = stream->time_base.num;
    stream->pts.den = stream->time_base.den;

    formatContext->bit_rate += stream->codec->bit_rate;

    hasVideo = true;

    return true;
  }

  return false;
}

bool BufferWriter::start(WriteCallback *c, bool seq)
{
  static const int ioBufferSize = 350 * 188;

  if (!streams.isEmpty())
  {
    callback = c;
    sequential = seq;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
    ioContext = ::avio_alloc_context(
#else
    ioContext = ::av_alloc_put_byte(
#endif
        (unsigned char *)::av_malloc(ioBufferSize),
        ioBufferSize,
        true,
        this,
        NULL,
        &BufferWriter::write,
        &BufferWriter::seek);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
    ioContext->seekable = sequential ? 0 : AVIO_SEEKABLE_NORMAL;
#else
    ioContext->is_streamed = sequential;
#endif

    formatContext->pb = ioContext;

    if (mpegTs)
      formatContext->mux_rate = formatContext->bit_rate + (formatContext->bit_rate / 2);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
    ::avformat_write_header(formatContext, NULL);
#else
    ::av_write_header(formatContext);
#endif

//    ::av_dump_format(formatContext, 0, NULL, 1);

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
  foreach (::AVStream *stream, streams)
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
#else
  if (stream->codec->codec_type == CODEC_TYPE_AUDIO)
#endif
  {
    ::AVPacket packet = FFMpegCommon::toAVPacket(buffer, stream);

//    qDebug() << "A:" << packet.stream_index << packet.size << buffer.presentationTimeStamp().toMSec() << packet.pts << packet.dts;

    if (::av_interleaved_write_frame(formatContext, &packet) != 0)
      qDebug() << "FFMpegBackend::BufferWriter: Could not write audio frame.";

    break;
  }
}

void BufferWriter::process(const SEncodedVideoBuffer &buffer)
{
  foreach (::AVStream *stream, streams)
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
#else
  if (stream->codec->codec_type == CODEC_TYPE_VIDEO)
#endif
  {
    ::AVPacket packet = FFMpegCommon::toAVPacket(buffer, stream);

//    qDebug() << "V:" << packet.stream_index << packet.size << buffer.presentationTimeStamp().toMSec() << packet.pts << packet.dts;

    if (::av_interleaved_write_frame(formatContext, &packet) != 0)
      qDebug() << "FFMpegBackend::BufferWriter: Could not write video frame.";

    break;
  }
}

void BufferWriter::process(const SEncodedDataBuffer &buffer)
{
  foreach (::AVStream *stream, streams)
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  if (stream->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
#else
  if (stream->codec->codec_type == CODEC_TYPE_SUBTITLE)
#endif
  {
    ::AVPacket packet = FFMpegCommon::toAVPacket(buffer, stream);

//    qDebug() << "D:" << packet.stream_index << packet.size << buffer.presentationTimeStamp().toMSec() << packet.pts << packet.dts;

    if (::av_interleaved_write_frame(formatContext, &packet) != 0)
      qDebug() << "FFMpegBackend::BufferWriter: Could not write data frame.";

    break;
  }
}

int BufferWriter::write(void *opaque, uint8_t *buf, int buf_size)
{
  reinterpret_cast<BufferWriter *>(opaque)->callback->write(buf, buf_size);

  return 0;
}

int64_t BufferWriter::seek(void *opaque, int64_t offset, int whence)
{
  BufferWriter * const me = reinterpret_cast<BufferWriter *>(opaque);
  
  if (me->sequential)
  {
    if (whence == SEEK_SET)
      return -1;
    else if (whence == SEEK_CUR)
      return -1;
    else if (whence == SEEK_END)
      return -1;
    else if (whence == AVSEEK_SIZE) // get size
      return me->callback->seek(offset, -1);

    return -1;
  }
  else  
    return me->callback->seek(offset, (whence == AVSEEK_SIZE) ? -1 : whence);
}

} } // End of namespaces
