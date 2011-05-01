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
#include <QtEndian>

namespace LXiStream {
namespace FFMpegBackend {


AudioEncoder::AudioEncoder(const QString &, QObject *parent)
  : SInterfaces::AudioEncoder(parent),
    outCodec(),
    codecHandle(NULL),
    contextHandle(NULL),
    passThrough(false),
    inFrameSize(0),
    inFrameBufferRaw(NULL),
    inFrameBuffer(NULL),
    inFrameBufferSize(0),
    showErrors(0)
{
}

AudioEncoder::~AudioEncoder()
{
  if (codecHandle && contextHandle)
    avcodec_close(contextHandle);

  if (contextHandle)
    av_free(contextHandle);

  delete [] inFrameBufferRaw;
}

bool AudioEncoder::openCodec(const SAudioCodec &c, Flags flags)
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

  contextHandle = avcodec_alloc_context();
  contextHandle->sample_rate = outCodec.sampleRate();
  contextHandle->sample_fmt = SAMPLE_FMT_S16;
  contextHandle->channels = outCodec.numChannels();
  contextHandle->channel_layout = FFMpegCommon::toFFMpegChannelLayout(outCodec.channelSetup());
  contextHandle->bit_rate = (outCodec.bitRate() > 0) ? outCodec.bitRate() : (96000 * contextHandle->channels);

  if (outCodec == "AC3")
    contextHandle->bit_rate = qMin(contextHandle->bit_rate, 384000); // Higher AC3 bitrates give problems muxing.

  contextHandle->bit_rate_tolerance = contextHandle->bit_rate / 2;

  if (flags & Flag_Fast)
    contextHandle->flags2 |= CODEC_FLAG2_FAST;

#ifdef OPT_ENABLE_THREADS
  contextHandle->thread_count = FFMpegCommon::encodeThreadCount(codecHandle->id);
  contextHandle->execute = &FFMpegCommon::execute;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  contextHandle->execute2 = &FFMpegCommon::execute2;
#endif
#endif

  if (avcodec_open(contextHandle, codecHandle) < 0)
  {
    qCritical() << "AudioEncoder: Could not open audio codec" << codecHandle->name;
    codecHandle = NULL;
    return false;
  }

  inFrameSize = contextHandle->frame_size * sizeof(qint16) * outCodec.numChannels();
  inFrameBufferRaw = new qint16[(inFrameSize + SBuffer::optimalAlignVal + SBuffer::optimalAlignVal) / sizeof(qint16)];
  inFrameBuffer = SBuffer::align(inFrameBufferRaw);
  inFrameBufferSize = 0;

  outCodec.setBitRate(contextHandle->bit_rate);

  if (contextHandle->extradata_size > 0)
    outCodec.setExtraData(QByteArray((const char *)contextHandle->extradata, contextHandle->extradata_size));

  return true;
}

SAudioCodec AudioEncoder::codec(void) const
{
  return outCodec;
}

SEncodedAudioBufferList AudioEncoder::encodeBuffer(const SAudioBuffer &audioBuffer)
{
  SEncodedAudioBufferList output;
  if (!audioBuffer.isNull())
  {
    if (passThrough)
    {
      output << SEncodedAudioBuffer(outCodec, audioBuffer.memory());
    }
    else if (audioBuffer.format().numChannels() == contextHandle->channels)
    {
      const qint16 *buffer = reinterpret_cast<const qint16 *>(audioBuffer.data());
      size_t size = audioBuffer.size();
      size_t ptr = 0;

      STime timeStamp = audioBuffer.timeStamp();
      if (timeStamp.isValid())
        timeStamp -= STime::fromClock(numSamples(inFrameBufferSize), contextHandle->sample_rate);
      else
        timeStamp = STime::null;

      while (size > 0)
      {
        if ((inFrameBufferSize > 0) && (size >= inFrameSize - inFrameBufferSize))
        {
          // Concatenate to the remaining buffer
          memcpy(reinterpret_cast<quint8 *>(inFrameBuffer) + inFrameBufferSize,
                 reinterpret_cast<const quint8 *>(buffer) + ptr,
                 inFrameSize - inFrameBufferSize);

          SEncodedAudioBuffer destBuffer(outCodec, qMax(FF_MIN_BUFFER_SIZE, AVCODEC_MAX_AUDIO_FRAME_SIZE));
          int out_size = avcodec_encode_audio(contextHandle,
                                              (uint8_t *)destBuffer.data(),
                                              destBuffer.capacity(),
                                              inFrameBuffer);
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
          int out_size = avcodec_encode_audio(contextHandle,
                                              (uint8_t *)destBuffer.data(),
                                              destBuffer.capacity(),
                                              reinterpret_cast<const qint16 *>(reinterpret_cast<const quint8 *>(buffer) + ptr));
          if (out_size > 0)
          {
            size -= inFrameSize;
            ptr += inFrameSize;

            destBuffer.resize(out_size);
            destBuffer.setPresentationTimeStamp(timeStamp);

            output << destBuffer;
            timeStamp += STime::fromClock(numSamples(inFrameSize), contextHandle->sample_rate);
          }
          else
            size = 0;
        }
        else
        {
          // Store the remaining buffer
          memcpy(reinterpret_cast<quint8 *>(inFrameBuffer) + inFrameBufferSize,
                 reinterpret_cast<const quint8 *>(buffer) + ptr,
                 size);

          inFrameBufferSize += size;
          size = 0;
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
