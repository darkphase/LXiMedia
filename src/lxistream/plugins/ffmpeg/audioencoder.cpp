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

#include "audioencoder.h"
#include "bufferwriter.h"
#include <QtConcurrent>
#include <QtEndian>

namespace LXiStream {
namespace FFMpegBackend {


AudioEncoder::AudioEncoder(const QString &, QObject *parent)
  : SInterfaces::AudioEncoder(parent),
    outCodec(),
    codecDict(NULL),
    codecHandle(NULL),
    contextHandle(NULL),
    contextHandleOwner(false),
    passThrough(false),
    inFrameSize(0),
    inFrameBufferRaw(NULL),
    inFrameBuffer(NULL),
    inFrameBufferSize(0),
    formatConvert(NULL),
    memorySem(memorySemCount),
    noDelay(false),
    enableWait(false),
    showErrors(0)
{
}

AudioEncoder::~AudioEncoder()
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

  delete [] inFrameBufferRaw;
}

const ::AVCodecContext * AudioEncoder::avCodecContext(void) const
{
  enableWait = true;

  return contextHandle;
}

bool AudioEncoder::openCodec(const SAudioCodec &c, SInterfaces::BufferWriter *bufferWriter, Flags flags)
{
  if (contextHandle)
    qFatal("AudioEncoder already opened a codec.");

  passThrough = false;
  outCodec = c;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  if (outCodec == "pcm_s16be")
  {
    passThrough = true;
    return true;
  }
#else
  if (outCodec == "pcm_s16le")
  {
    passThrough = true;
    return true;
  }
#endif

  if ((codecHandle = ::avcodec_find_encoder_by_name(outCodec.name())) == NULL)
  {
    qCritical() << "AudioEncoder: Audio codec not found " << outCodec.name();
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

  ::avcodec_get_context_defaults3(contextHandle, codecHandle);

  contextHandle->sample_fmt = ::AV_SAMPLE_FMT_NONE;

  if (codecHandle->sample_fmts)
  for (const ::AVSampleFormat *f=codecHandle->sample_fmts; *f != ::AV_SAMPLE_FMT_NONE; f++)
  if (FFMpegCommon::fromFFMpegSampleFormat(*f) != SAudioFormat::Format_Invalid)
  {
    contextHandle->sample_fmt = *f;
    break;
  }

  if (contextHandle->sample_fmt == ::AV_SAMPLE_FMT_NONE)
    contextHandle->sample_fmt = ::AV_SAMPLE_FMT_S16;

  contextHandle->sample_rate = outCodec.sampleRate();
  contextHandle->channels = outCodec.numChannels();
  contextHandle->channel_layout = FFMpegCommon::toFFMpegChannelLayout(outCodec.channelSetup());
  contextHandle->time_base.num = 1;
  contextHandle->time_base.den = contextHandle->sample_rate;

  contextHandle->bit_rate = (outCodec.bitRate() > 0) ? outCodec.bitRate() : (96000 * contextHandle->channels);
  if (contextHandle->codec_id == CODEC_ID_AC3)
    contextHandle->bit_rate = qMin(contextHandle->bit_rate, 384000); // Higher bitrates give problems muxing.
  else if (contextHandle->codec_id == CODEC_ID_FLAC)
    contextHandle->bit_rate = (contextHandle->sample_rate * contextHandle->channels * 2) * 8; // To prevent problems muxing.

  contextHandle->bit_rate_tolerance = contextHandle->bit_rate / 4;

  if (flags & Flag_Fast)
    contextHandle->flags2 |= CODEC_FLAG2_FAST;

  if (flags & Flag_NoDelay)
    noDelay = true;
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

  contextHandle->flags |= CODEC_FLAG_QSCALE;

  if (avcodec_open2(contextHandle, codecHandle, &codecDict) < 0)
  {
    qCritical() << "AudioEncoder: Could not open audio codec" << codecHandle->name;
    codecHandle = NULL;
    return false;
  }

  formatConvert.setDestFormat(FFMpegCommon::fromFFMpegSampleFormat(contextHandle->sample_fmt));

  inFrameSize = ::av_samples_get_buffer_size(
        NULL, contextHandle->channels,
        contextHandle->frame_size, contextHandle->sample_fmt, 1);
  inFrameDuration = STime(contextHandle->frame_size, SInterval(contextHandle->time_base.num, contextHandle->time_base.den));
  inFrameBufferRaw = new char[inFrameSize + (SBuffer::optimalAlignVal * 2)];
  inFrameBuffer = SBuffer::align(inFrameBufferRaw, SBuffer::optimalAlignVal);
  inFrameBufferSize = 0;

  outCodec.setFrameSize(contextHandle->frame_size);
  outCodec.setBitRate(contextHandle->bit_rate + contextHandle->bit_rate_tolerance);

  return true;
}

SAudioCodec AudioEncoder::codec(void) const
{
  return outCodec;
}

SEncodedAudioBufferList AudioEncoder::encodeBuffer(const SAudioBuffer &audioBuffer)
{
  SEncodedAudioBufferList output;

  if (!noDelay)
  {
    LXI_PROFILE_WAIT(encodeFuture.waitForFinished());

    output = delayedResult;
    delayedResult.clear();

    if (!audioBuffer.isNull())
      encodeFuture = QtConcurrent::run(this, &AudioEncoder::encodeBufferTask, audioBuffer, &delayedResult, enableWait);
    else // Flush
      encodeBufferTask(audioBuffer, &output, false);
  }
  else
    encodeBufferTask(audioBuffer, &output, false);

  return output;
}

void AudioEncoder::encodeBufferTask(const SAudioBuffer &audioBuffer, SEncodedAudioBufferList *output, bool wait)
{
  // We need to wait until all BufferWriters have written the previous buffer as
  // they may share the AVCodecContext.
  if (wait)
  {
    LXI_PROFILE_WAIT(memorySem.acquire(memorySemCount));
    memorySem.release(memorySemCount);
  }

  LXI_PROFILE_FUNCTION(TaskType_AudioProcessing);

  if (!audioBuffer.isNull())
  {
    if (passThrough)
    {
      SEncodedAudioBuffer destBuffer(outCodec, audioBuffer.memory());
      destBuffer.setPresentationTimeStamp(audioBuffer.timeStamp());
      destBuffer.setDuration(audioBuffer.duration());

      output->append(destBuffer);
    }
    else if (audioBuffer.format().numChannels() == unsigned(contextHandle->channels))
    {
      const SAudioBuffer preprocBuffer = formatConvert.convert(audioBuffer);
      if (!preprocBuffer.isNull())
      {
        const char *buffer = preprocBuffer.data();
        size_t size = preprocBuffer.size();
        size_t ptr = 0;

        STime timeStamp = preprocBuffer.timeStamp();
        if (timeStamp.isValid())
        {
          const int bps = ::av_get_bytes_per_sample(contextHandle->sample_fmt);
          if ((bps > 0) && (inFrameBufferSize > 0))
          {
            timeStamp -= STime::fromClock(
                  inFrameBufferSize / (bps * contextHandle->channels),
                  contextHandle->sample_rate);
          }
        }
        else
          timeStamp = STime::null;

        while (size > 0)
        {
          if ((inFrameBufferSize > 0) && (size >= size_t(inFrameSize) - inFrameBufferSize))
          {
            // Concatenate to the remaining buffer
            memcpy(inFrameBuffer + inFrameBufferSize, buffer + ptr, inFrameSize - inFrameBufferSize);
            size -= inFrameSize - inFrameBufferSize;
            ptr += inFrameSize - inFrameBufferSize;
            inFrameBufferSize = inFrameSize;

            const size_t consumed =
                encodeFrames(inFrameBuffer, inFrameSize, timeStamp, *output, wait);

            if ((consumed > 0) && (consumed <= inFrameBufferSize))
              inFrameBufferSize -= consumed;
            else
              inFrameBufferSize = 0;
          }
          else if (size >= size_t(inFrameSize))
          {
            const size_t consumed =
                encodeFrames(buffer + ptr, size, timeStamp, *output, wait);

            size -= consumed;
            ptr += consumed;
          }
          else
          {
            // Store the remaining buffer
            memcpy(inFrameBuffer + inFrameBufferSize, buffer + ptr, size);

            inFrameBufferSize += size;
            size = 0;
          }
        }
      }
    }
    else if (showErrors++ == 0)
    {
      qWarning() << "AudioEncoder: Incorrect number of channels:"
          << audioBuffer.format().numChannels() << "should be:"
          << contextHandle->channels;
    }
  }
  else
    output->append(SEncodedAudioBuffer()); // Flush
}

//size_t AudioEncoder::encodeFrames(const char *buffer, size_t size, STime &timeStamp, SEncodedAudioBufferList &output, bool wait) const
//{
//  ::AVFrame frame;
//  memset(&frame, 0, sizeof(frame));
//  ::avcodec_get_frame_defaults(&frame);
//  frame.nb_samples = contextHandle->frame_size;

//  ::AVPacket packet;
//  memset(&packet, 0, sizeof(packet));
//  ::av_init_packet(&packet);

//  size_t consumed = 0;
//  while ((consumed + inFrameSize) <= size)
//  {
//    if (::avcodec_fill_audio_frame(
//          &frame,
//          contextHandle->channels,
//          contextHandle->sample_fmt,
//          reinterpret_cast<const uint8_t *>(buffer + consumed),
//          inFrameSize, 1) >= 0)
//    {
//      packet.data = NULL;
//      packet.size = 0;

//      frame.pts = 0;

//      int gotPacket = 0;
//      if (::avcodec_encode_audio2(contextHandle, &packet, &frame, &gotPacket) >= 0)
//      {
//        if (gotPacket != 0)
//        {
//          SEncodedAudioBuffer destBuffer(outCodec, SBuffer::MemoryPtr(
//              new FFMpegCommon::SyncMemory(packet.size, wait ? &memorySem : NULL)));

//          destBuffer.resize(packet.size);
//          ::memcpy(destBuffer.data(), packet.data, packet.size);

//          destBuffer.setPresentationTimeStamp(timeStamp);
//          destBuffer.setDuration(inFrameDuration);

//          output += destBuffer;
//          timeStamp += STime::fromClock(frame.nb_samples, contextHandle->sample_rate);

//          ::av_free_packet(&packet);
//        }

//        consumed += inFrameSize;
//        continue;
//      }
//    }

//    consumed = size;
//  }

//  return consumed;
//}

size_t AudioEncoder::encodeFrames(const char *buffer, size_t size, STime &timeStamp, SEncodedAudioBufferList &output, bool wait) const
{
  size_t consumed = 0;
  while ((consumed + inFrameSize) <= size)
  {
    SEncodedAudioBuffer destBuffer(outCodec, SBuffer::MemoryPtr(
        new FFMpegCommon::SyncMemory(FF_MIN_BUFFER_SIZE, wait ? &memorySem : NULL)));

    const int len = ::avcodec_encode_audio(
          contextHandle,
          reinterpret_cast<uint8_t *>(destBuffer.data()), destBuffer.size(),
          reinterpret_cast<const short *>(buffer + consumed));

    if (len >= 0)
    {
      destBuffer.resize(len);
      destBuffer.setPresentationTimeStamp(timeStamp);
      destBuffer.setDuration(inFrameDuration);

      output += destBuffer;
      timeStamp += STime::fromClock(contextHandle->frame_size, contextHandle->sample_rate);

      consumed += inFrameSize;
      continue;
    }

    consumed = size;
  }

  return consumed;
}

} } // End of namespaces
