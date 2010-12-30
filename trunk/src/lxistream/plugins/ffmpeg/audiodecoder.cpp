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

#include "audiodecoder.h"

// Implemented in audiodecoder.postfilter.c
extern "C" void LXiStream_FFMpegBackend_AudioDecoder_postFilterDts(qint16 *, const qint16 *, size_t, unsigned);
extern "C" void LXiStream_FFMpegBackend_AudioDecoder_postFilterAac(qint16 *, const qint16 *, size_t, unsigned);

namespace LXiStream {
namespace FFMpegBackend {


AudioDecoder::AudioDecoder(const QString &, QObject *parent)
  : SInterfaces::AudioDecoder(parent),
    inCodec(),
    codecHandle(NULL),
    contextHandle(NULL),
    postFilter(NULL),
    passThrough(false),
    audioSamplesSeen(0)
{
}

AudioDecoder::~AudioDecoder()
{
  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

  if (codecHandle && contextHandle)
    avcodec_close(contextHandle);

  if (contextHandle)
  {
    if (contextHandle->extradata)
      delete [] contextHandle->extradata;

    av_free(contextHandle);
  }
}

bool AudioDecoder::openCodec(const SAudioCodec &c, Flags flags)
{
  if (contextHandle)
    qFatal("AudioDecoder already opened a codec.");

  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

  passThrough = false;
  inCodec = c;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  if (inCodec == "PCM/S16BE")
  {
    passThrough = true;
    return true;
  }
#else
  if (inCodec == "PCM/S16LE")
  {
    passThrough = true;
    return true;
  }
#endif

  if ((codecHandle = avcodec_find_decoder(FFMpegCommon::toFFMpegCodecID(inCodec))) == NULL)
  {
    qWarning() << "AudioDecoder: Audio codec not found " << inCodec.codec();
    return false;
  }

  contextHandle = avcodec_alloc_context();
  contextHandle->codec_type = CODEC_TYPE_AUDIO;
  contextHandle->flags2 |= CODEC_FLAG2_CHUNKS;

  if (inCodec.sampleRate() != 0)
    contextHandle->sample_rate = inCodec.sampleRate();

  if (inCodec.channelSetup() != SAudioFormat::Channel_None)
  {
    contextHandle->channels = inCodec.numChannels();
#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
    contextHandle->channel_layout = FFMpegCommon::toFFMpegChannelLayout(inCodec.channelSetup());
#endif
  }

  if ((flags & Flag_DownsampleToStereo) && (contextHandle->channels != 2))
  {
#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
    contextHandle->request_channel_layout = FFMpegCommon::toFFMpegChannelLayout(SAudioFormat::Channel_Stereo);
#else
    contextHandle->channels = 2;
#endif
  }

  contextHandle->bit_rate = inCodec.bitRate();

  contextHandle->extradata_size = inCodec.extraData().size();
  if (contextHandle->extradata_size > 0)
  {
    contextHandle->extradata = new uint8_t[contextHandle->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE];
    memcpy(contextHandle->extradata, inCodec.extraData().data(), contextHandle->extradata_size);
    memset(contextHandle->extradata + contextHandle->extradata_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
  }
  else
    contextHandle->extradata = NULL;

  if (avcodec_open(contextHandle, codecHandle) < 0)
  {
    qCritical() << "AudioDecoder: Could not open audio codec " << codecHandle->name;
    codecHandle = NULL;
    return false;
  }

  if (inCodec == "DTS")
    postFilter = &LXiStream_FFMpegBackend_AudioDecoder_postFilterDts;
  else if (inCodec == "AAC")
    postFilter = &LXiStream_FFMpegBackend_AudioDecoder_postFilterAac;
  else
    postFilter = PostFilterFunc(&memcpy);

  return true;
}

SAudioBufferList AudioDecoder::decodeBuffer(const SEncodedAudioBuffer &audioBuffer)
{
  SAudioBufferList output;
  if (!audioBuffer.isNull())
  {
    if (passThrough)
    {
      output << SAudioBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                          audioBuffer.codec().channelSetup(),
                                          audioBuffer.codec().sampleRate()),
                             audioBuffer.memory());
    }
    else if (codecHandle)
    {
      const uint8_t *inPtr = reinterpret_cast<const uint8_t *>(audioBuffer.data());
      int inSize = audioBuffer.size();

      STime timeStamp = audioBuffer.presentationTimeStamp();
      if (contextHandle->sample_rate > 0)
      {
        if (!timeStamp.isValid())
          timeStamp = STime::fromClock(audioSamplesSeen, contextHandle->sample_rate);
        else
          audioSamplesSeen = timeStamp.toClock(contextHandle->sample_rate);
      }

      if (!timeStamp.isValid())
        timeStamp = audioBuffer.decodingTimeStamp();

      qint16 tempBufferRaw[(AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE + SBuffer::optimalAlignVal) / sizeof(int16_t)];
      qint16 * const tempBuffer = SBuffer::align(tempBufferRaw);
      while (inSize > 0)
      {
        // decode the audio
        int outSize = sizeof(tempBufferRaw) - SBuffer::optimalAlignVal;
        int len = avcodec_decode_audio2(contextHandle, tempBuffer, &outSize, (uint8_t *)inPtr, inSize);
        if (len >= 0)
        {
          if (outSize > 0)
          {
            const SAudioFormat outFormat(SAudioFormat::Format_PCM_S16,
#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
                                         FFMpegCommon::fromFFMpegChannelLayout(contextHandle->channel_layout, contextHandle->channels),
#else
                                         SAudioFormat::guessChannels(contextHandle->channels),
#endif
                                         contextHandle->sample_rate);
            SAudioBuffer destBuffer(outFormat, outSize / (outFormat.sampleSize() * outFormat.numChannels()));
            destBuffer.setTimeStamp(timeStamp);

            postFilter(reinterpret_cast<qint16 *>(destBuffer.data()), tempBuffer, outSize, outFormat.numChannels());

//            qDebug() << "Audio timestamp" << timeStamp.toMSec()
//                << ", pts =" << audioBuffer.presentationTimeStamp().toMSec()
//                << ", dts =" << audioBuffer.decodingTimeStamp().toMSec();

            output << destBuffer;

            if (contextHandle->sample_rate > 0)
              timeStamp += STime::fromClock(destBuffer.numSamples(), contextHandle->sample_rate);

            audioSamplesSeen += destBuffer.numSamples();
          }

          inPtr += len;
          inSize -= len;
          continue;
        }

        break;
      }
    }
  }

  return output;
}


} } // End of namespaces
