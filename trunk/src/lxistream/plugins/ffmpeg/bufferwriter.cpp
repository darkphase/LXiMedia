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

#include "bufferwriter.h"
#include "audioencoder.h"
#include "videoencoder.h"

namespace LXiStream {
namespace FFMpegBackend {


BufferWriter::BufferWriter(const QString &, QObject *parent)
  : SInterfaces::BufferWriter(parent),
    ioDevice(NULL),
    format(NULL),
    formatContext(NULL),
    ioContext(NULL),
    hasAudio(false), hasVideo(false),
    mpegClock(false), mpegTs(false)
{
}

BufferWriter::~BufferWriter()
{
  clear();
}

bool BufferWriter::openFormat(const QString &name)
{
  format = ::av_guess_format(name.toLatin1().data(), NULL, NULL);
  if (format)
  {
    formatContext = ::avformat_alloc_context();
    formatContext->oformat = format;

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
      formatContext->packet_size = 2048;
    }

    formatContext->max_delay = int(0.7 * AV_TIME_BASE);

    return true;
  }

  return false;
}

::AVStream * BufferWriter::createStream(AVCodec *codec)
{
  AVStream * const stream = ::avformat_new_stream(formatContext, codec);
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
      ::AVCodec * codec;
      if (ffEncoder)
        codec = ffEncoder->avCodecContext()->codec;
      else
        codec = ::avcodec_find_encoder_by_name(encoder->codec().name());

      if (codec)
      {
        stream = createStream(codec);

        const SAudioCodec baseCodec = encoder->codec();
        const unsigned sampleRate = baseCodec.sampleRate();

        ::avcodec_get_context_defaults3(stream->codec, stream->codec->codec);

        stream->codec->time_base = stream->time_base;
        stream->codec->bit_rate = baseCodec.bitRate() > 0 ? baseCodec.bitRate() : 2000000;
        stream->codec->frame_size = baseCodec.frameSize() > 0 ? baseCodec.frameSize() : 8192;
        stream->codec->sample_rate = sampleRate;
        stream->codec->channels = baseCodec.numChannels();
        stream->codec->channel_layout = FFMpegCommon::toFFMpegChannelLayout(baseCodec.channelSetup());

        stream->codec->time_base.num = 1;
        stream->codec->time_base.den = sampleRate;
      }
    }

    if (stream)
    {
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
      ::AVCodec * codec;
      if (ffEncoder)
        codec = ffEncoder->avCodecContext()->codec;
      else
        codec = ::avcodec_find_encoder_by_name(encoder->codec().name());

      if (codec)
      {
        stream = createStream(codec);

        const SVideoCodec baseCodec = encoder->codec();

        ::avcodec_get_context_defaults3(stream->codec, stream->codec->codec);

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
    }

    if (stream)
    {
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
      stream->avg_frame_rate.num = stream->r_frame_rate.num;
      stream->avg_frame_rate.den = stream->r_frame_rate.den;

      stream->pts.val = 0;
      stream->pts.num = stream->time_base.num;
      stream->pts.den = stream->time_base.den;

      formatContext->bit_rate += stream->codec->bit_rate;

      hasVideo = true;

      return true;
    }
  }

  return false;
}

bool BufferWriter::start(QIODevice *d)
{
  static const int ioBufferSize = 350 * 188;

  if (!streams.isEmpty())
  {
    ioDevice = d;

    ioContext = ::avio_alloc_context(
        (unsigned char *)::av_malloc(ioBufferSize),
        ioBufferSize,
        true,
        this,
        NULL,
        &BufferWriter::write,
        &BufferWriter::seek);

    ioContext->seekable = ioDevice->isSequential() ? 0 : AVIO_SEEKABLE_NORMAL;

    formatContext->pb = ioContext;

    ::avformat_write_header(formatContext, NULL);

//    ::av_dump_format(formatContext, 0, NULL, 1);

    return true;
  }

  return false;
}

void BufferWriter::stop(void)
{
  if (formatContext)
    ::av_write_trailer(formatContext);

  clear();
}

void BufferWriter::process(const SEncodedAudioBuffer &buffer)
{
  if (!buffer.isNull())
  foreach (::AVStream *stream, streams)
  if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
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
  if (!buffer.isNull())
  foreach (::AVStream *stream, streams)
  if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
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
  if (!buffer.isNull())
  foreach (::AVStream *stream, streams)
  if (stream->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
  {
    ::AVPacket packet = FFMpegCommon::toAVPacket(buffer, stream);

//    qDebug() << "D:" << packet.stream_index << packet.size << buffer.presentationTimeStamp().toMSec() << packet.pts << packet.dts;

    if (::av_interleaved_write_frame(formatContext, &packet) != 0)
      qDebug() << "FFMpegBackend::BufferWriter: Could not write data frame.";

    break;
  }
}

void BufferWriter::clear(void)
{
  if (!streams.isEmpty())
  {
    foreach (::AVStream *stream, streams)
    {
      if (stream->codec->codec)
        ::avcodec_close(stream->codec);

      ::av_freep(&stream->codec);
      ::av_freep(&stream);
    }

    streams.clear();
  }

  if (formatContext)
  {
    ::av_free(formatContext);
    formatContext = NULL;
  }

  if (ioContext)
  {
    ::av_free(ioContext->buffer);
    ::av_free(ioContext);
    ioContext = NULL;
  }

  ioDevice = NULL;
}

int BufferWriter::write(void *opaque, uint8_t *buf, int buf_size)
{
  BufferWriter * const me = reinterpret_cast<BufferWriter *>(opaque);

  if (me->ioDevice)
  {
    while (me->ioDevice->bytesToWrite() >= outBufferSize)
    if (!me->ioDevice->waitForBytesWritten(-1))
      return -1;

    int i = 0;
    while (i < buf_size)
    {
      const qint64 r = me->ioDevice->write((char *)buf + i, buf_size - i);
      if (r > 0)
        i += r;
      else
        break;
    }

    return i;
  }

  return -1;
}

int64_t BufferWriter::seek(void *opaque, int64_t offset, int whence)
{
  BufferWriter * const me = reinterpret_cast<BufferWriter *>(opaque);
  
  if (me->ioDevice->isSequential())
  {
    if (whence == SEEK_SET)
      return -1;
    else if (whence == SEEK_CUR)
      return -1;
    else if (whence == SEEK_END)
      return -1;
    else if (whence == AVSEEK_SIZE) // get size
      return me->ioDevice->size();
  }
  else  
  {
    if (whence == SEEK_SET)
      return me->ioDevice->seek(offset) ? 0 : -1;
    else if (whence == SEEK_CUR)
      return me->ioDevice->seek(me->ioDevice->pos() + offset) ? 0 : -1;
    else if (whence == SEEK_END)
      return me->ioDevice->seek(me->ioDevice->size() + offset) ? 0 : -1;
    else if (whence == AVSEEK_SIZE) // get size
      return me->ioDevice->size();
  }

  return -1;
}

} } // End of namespaces
