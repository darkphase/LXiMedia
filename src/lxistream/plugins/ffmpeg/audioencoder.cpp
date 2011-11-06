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

#include "audioencoder.h"
#include "bufferwriter.h"
#include <QtEndian>

namespace LXiStream {
namespace FFMpegBackend {


AudioEncoder::AudioEncoder(const QString &, QObject *parent)
  : SInterfaces::AudioEncoder(parent),
    outCodec(),
    codecHandle(NULL),
    contextHandle(NULL),
    contextHandleOwner(false),
    passThrough(false),
    inSampleSize(sizeof(qint16)),
    inFrameSize(0),
    inFrameBufferRaw(NULL),
    inFrameBuffer(NULL),
    inFrameBufferSize(0),
    formatConvert(NULL),
    showErrors(0)
{
}

AudioEncoder::~AudioEncoder()
{
  if (contextHandle && contextHandleOwner)
  {
    if (codecHandle)
      ::avcodec_close(contextHandle);

    ::av_free(contextHandle);
  }

  delete [] inFrameBufferRaw;
}

bool AudioEncoder::openCodec(const SAudioCodec &c, SInterfaces::BufferWriter *bufferWriter, Flags flags)
{
  if (contextHandle)
    qFatal("AudioEncoder already opened a codec.");

  passThrough = false;
  outCodec = c;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  if (outCodec == "PCM/S16BE")
  {
    passThrough = true;
    return true;
  }
#else
  if (outCodec == "PCM/S16LE")
  {
    passThrough = true;
    return true;
  }
#endif

  if ((codecHandle = avcodec_find_encoder(FFMpegCommon::toFFMpegCodecID(outCodec))) == NULL)
  {
    qCritical() << "AudioEncoder: Audio codec not found " << outCodec.codec();
    return false;
  }

  BufferWriter * const ffBufferWriter = qobject_cast<BufferWriter *>(bufferWriter);
  if (ffBufferWriter)
  {
    contextHandle = ffBufferWriter->createStream()->codec;
    contextHandleOwner = false;
  }
  else
  {
    contextHandle = ::avcodec_alloc_context();
    contextHandleOwner = true;
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  ::avcodec_get_context_defaults2(contextHandle, AVMEDIA_TYPE_AUDIO);
#else
  ::avcodec_get_context_defaults2(contextHandle, CODEC_TYPE_AUDIO);
#endif

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  contextHandle->sample_fmt = ::AV_SAMPLE_FMT_NONE;
#else
  contextHandle->sample_fmt = ::SAMPLE_FMT_NONE;
#endif

  if (codecHandle->sample_fmts)
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  for (const ::AVSampleFormat *f=codecHandle->sample_fmts; *f != ::AV_SAMPLE_FMT_NONE; f++)
#else
  for (const ::SampleFormat *f=codecHandle->sample_fmts; *f != ::SAMPLE_FMT_NONE; f++)
#endif
  if (FFMpegCommon::fromFFMpegSampleFormat(*f) != SAudioFormat::Format_Invalid)
  {
    contextHandle->sample_fmt = *f;
    break;
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  if (contextHandle->sample_fmt == ::AV_SAMPLE_FMT_NONE)
    contextHandle->sample_fmt = ::AV_SAMPLE_FMT_S16;
#else
  if (contextHandle->sample_fmt == ::SAMPLE_FMT_NONE)
    contextHandle->sample_fmt = ::SAMPLE_FMT_S16;
#endif

  contextHandle->sample_rate = outCodec.sampleRate();
  contextHandle->channels = outCodec.numChannels();
  contextHandle->channel_layout = FFMpegCommon::toFFMpegChannelLayout(outCodec.channelSetup());
  contextHandle->time_base.num = 1;
  contextHandle->time_base.den = contextHandle->sample_rate;

  contextHandle->bit_rate = (outCodec.bitRate() > 0) ? outCodec.bitRate() : (96000 * contextHandle->channels);
  if (outCodec == "AC3")
    contextHandle->bit_rate = qMin(contextHandle->bit_rate, 384000); // Higher bitrates give problems muxing.
  else if (outCodec == "FLAC")
    contextHandle->bit_rate = (contextHandle->sample_rate * contextHandle->channels * 2) * 8; // To prevent problems muxing.

  contextHandle->bit_rate_tolerance = contextHandle->bit_rate / 4;

  if (flags & Flag_Fast)
    contextHandle->flags2 |= CODEC_FLAG2_FAST;

#ifdef OPT_ENABLE_THREADS
  contextHandle->thread_count = FFMpegCommon::encodeThreadCount(codecHandle->id);
  contextHandle->execute = &FFMpegCommon::execute;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  contextHandle->execute2 = &FFMpegCommon::execute2;
#endif
#endif

  if (ffBufferWriter)
  if (ffBufferWriter->avFormat()->flags & AVFMT_GLOBALHEADER)
    contextHandle->flags |= CODEC_FLAG_GLOBAL_HEADER;

  if (avcodec_open(contextHandle, codecHandle) < 0)
  {
    qCritical() << "AudioEncoder: Could not open audio codec" << codecHandle->name;
    codecHandle = NULL;
    return false;
  }

  formatConvert.setDestFormat(FFMpegCommon::fromFFMpegSampleFormat(contextHandle->sample_fmt));

  inSampleSize = SAudioFormat::sampleSize(formatConvert.destFormat());
  inFrameSize = contextHandle->frame_size * inSampleSize * outCodec.numChannels();
  inFrameBufferRaw = new char[inFrameSize + SBuffer::optimalAlignVal + SBuffer::optimalAlignVal];
  inFrameBuffer = SBuffer::align(inFrameBufferRaw);
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
  LXI_PROFILE_FUNCTION(TaskType_AudioProcessing);

  SEncodedAudioBufferList output;
  if (!audioBuffer.isNull())
  {
    if (passThrough)
    {
      output << SEncodedAudioBuffer(outCodec, audioBuffer.memory());
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
          timeStamp -= STime::fromClock(numSamples(inFrameBufferSize), contextHandle->sample_rate);
        else
          timeStamp = STime::null;

        while (size > 0)
        {
          if ((inFrameBufferSize > 0) && (size >= inFrameSize - inFrameBufferSize))
          {
            // Concatenate to the remaining buffer
            memcpy(inFrameBuffer + inFrameBufferSize, buffer + ptr, inFrameSize - inFrameBufferSize);

            SEncodedAudioBuffer destBuffer(outCodec, qMax(FF_MIN_BUFFER_SIZE, AVCODEC_MAX_AUDIO_FRAME_SIZE));
            int out_size = avcodec_encode_audio(
                contextHandle,
                reinterpret_cast<uint8_t *>(destBuffer.data()), destBuffer.capacity(),
                reinterpret_cast<const short *>(inFrameBuffer));

            if (out_size > 0)
            {
              size -= inFrameSize - inFrameBufferSize;
              ptr += inFrameSize - inFrameBufferSize;
              inFrameBufferSize = 0;

              destBuffer.resize(out_size);
              destBuffer.setPresentationTimeStamp(timeStamp);

              output << destBuffer;
              timeStamp += STime::fromClock(numSamples(inFrameSize), contextHandle->sample_rate);
            }
            else
              size = 0;
          }
          else if (size >= inFrameSize)
          {
            SEncodedAudioBuffer destBuffer(outCodec, qMax(FF_MIN_BUFFER_SIZE, AVCODEC_MAX_AUDIO_FRAME_SIZE));
            int out_size = avcodec_encode_audio(
                contextHandle,
                reinterpret_cast<uint8_t *>(destBuffer.data()), destBuffer.capacity(),
                reinterpret_cast<const short *>(buffer + ptr));

            if (out_size >= 0)
            {
              size -= inFrameSize;
              ptr += inFrameSize;

              if (out_size > 0)
              {
                destBuffer.resize(out_size);
                destBuffer.setPresentationTimeStamp(timeStamp);

                output << destBuffer;
                timeStamp += STime::fromClock(numSamples(inFrameSize), contextHandle->sample_rate);
              }
            }
            else
              size = 0;
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

  return output;
}



} } // End of namespaces
