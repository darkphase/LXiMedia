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

#include "audiodecoder.h"
#include "bufferreader.h"
#include "networkbufferreader.h"

namespace LXiStream {
namespace FFMpegBackend {

AudioDecoder::AudioDecoder(const QString &, QObject *parent)
  : SInterfaces::AudioDecoder(parent),
    inCodec(),
    codecHandle(NULL),
    contextHandle(NULL),
    contextHandleOwner(false),
    postFilter(NULL),
    passThrough(false),
    timeStamp(STime::null)
{
}

AudioDecoder::~AudioDecoder()
{
  if (contextHandle && contextHandleOwner)
  {
    if (codecHandle)
      ::avcodec_close(contextHandle);

    ::av_free(contextHandle);
  }
}

bool AudioDecoder::openCodec(const SAudioCodec &c, SInterfaces::AbstractBufferReader *bufferReader, Flags flags)
{
  if (contextHandle)
    qFatal("AudioDecoder already opened a codec.");

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

  BufferReaderBase * ffBufferReader = qobject_cast<BufferReader *>(bufferReader);
  if (ffBufferReader == NULL)
    ffBufferReader = qobject_cast<NetworkBufferReader *>(bufferReader);

  if (ffBufferReader && (inCodec.streamId() >= 0))
  {
    contextHandle = ffBufferReader->getStream(inCodec.streamId())->codec;
    contextHandleOwner = false;
  }
  else
  {
    contextHandle = avcodec_alloc_context();
    contextHandleOwner = true;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
    contextHandle->codec_type = AVMEDIA_TYPE_AUDIO;
#else
    contextHandle->codec_type = CODEC_TYPE_AUDIO;
#endif

    if (inCodec.sampleRate() != 0)
      contextHandle->sample_rate = inCodec.sampleRate();

    if (inCodec.channelSetup() != SAudioFormat::Channel_None)
    {
      contextHandle->channels = inCodec.numChannels();
      contextHandle->channel_layout = FFMpegCommon::toFFMpegChannelLayout(inCodec.channelSetup());
    }

    contextHandle->bit_rate = inCodec.bitRate();
  }

  contextHandle->flags2 |= CODEC_FLAG2_CHUNKS;

  if ((flags & Flag_DownsampleToStereo) && (contextHandle->channels != 2))
    contextHandle->request_channel_layout = CH_LAYOUT_STEREO_DOWNMIX;

#ifdef OPT_ENABLE_THREADS
  contextHandle->thread_count = FFMpegCommon::decodeThreadCount(codecHandle->id);
  contextHandle->execute = &FFMpegCommon::execute;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  contextHandle->execute2 = &FFMpegCommon::execute2;
#endif
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 6, 0)
  if (avcodec_open(contextHandle, codecHandle) < 0)
#else
  if (avcodec_open2(contextHandle, codecHandle, NULL) < 0)
#endif
  {
    qCritical() << "AudioDecoder: Could not open audio codec " << codecHandle->name;
    codecHandle = NULL;
    return false;
  }

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
  struct PostFilter
  {
    static void Dts(qint16 *dst, const qint16 *src, size_t numBytes, unsigned numChannels)
    {
      if (numChannels == 6)
      {
        for (unsigned i=0, n=numBytes/sizeof(qint16); i<n; i+=6)
        {
          dst[i+0] = src[i+0];
          dst[i+1] = src[i+2];
          dst[i+2] = src[i+1];
          dst[i+3] = src[i+4];
          dst[i+4] = src[i+5];
          dst[i+5] = src[i+3];
        }
      }
      else
        memcpy(dst, src, numBytes);
    }
    
    static void Aac(qint16 *dst, const qint16 *src, size_t numBytes, unsigned numChannels)
    {
      if (numChannels == 6)
      {
        for (unsigned i=0, n=numBytes/sizeof(qint16); i<n; i+=6)
        {
          dst[i+0] = src[i+1];
          dst[i+1] = src[i+0];
          dst[i+2] = src[i+2];
          dst[i+3] = src[i+3];
          dst[i+4] = src[i+4];
          dst[i+5] = src[i+5];
        }
      }
      else
        memcpy(dst, src, numBytes);
    }
  };
  
  if (inCodec == "DTS")
    postFilter = &PostFilter::Dts;
  else if (inCodec == "AAC")
    postFilter = &PostFilter::Aac;
  else
#endif
    postFilter = PostFilterFunc(&memcpy);

  return true;
}

SAudioBufferList AudioDecoder::decodeBuffer(const SEncodedAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_AudioProcessing);

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
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
      ::AVPacket packet = FFMpegCommon::toAVPacket(audioBuffer);
#endif

      const uint8_t *inPtr = reinterpret_cast<const uint8_t *>(audioBuffer.data());
      int inSize = audioBuffer.size();

      STime currentTime = audioBuffer.presentationTimeStamp();
      if (!currentTime.isValid())
        currentTime = audioBuffer.decodingTimeStamp();

      if (qAbs(timeStamp - currentTime).toMSec() > defaultBufferLen)
        timeStamp = currentTime;

      SBuffer temp(AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
      temp.resize(AVCODEC_MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);

      while (inSize > 0)
      {
        // decode the audio
        int outSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
        const int len = ::avcodec_decode_audio2(contextHandle, reinterpret_cast<int16_t *>(temp.data()), &outSize, (quint8 *)inPtr, inSize);
#else
        packet.data = (uint8_t *)inPtr;
        packet.size = inSize;
        const int len = ::avcodec_decode_audio3(contextHandle, reinterpret_cast<int16_t *>(temp.data()), &outSize, &packet);
#endif

        if (len >= 0)
        {
          if ((outSize > 0) && (contextHandle->sample_rate > 0))
          {
            const SAudioFormat outFormat(SAudioFormat::Format_PCM_S16,
                                         FFMpegCommon::fromFFMpegChannelLayout(contextHandle->channel_layout, contextHandle->channels),
                                         contextHandle->sample_rate);

            const int sampleSize = outFormat.sampleSize() * outFormat.numChannels();
            const int totalSamples = outSize / sampleSize;
            const int defaultSamples = (contextHandle->sample_rate * defaultBufferLen) / 1000;
            const int maxSamples = defaultSamples + (defaultSamples / 2);

            for (int i=0; i<totalSamples; )
            {
              const int numSamples = ((totalSamples - i) <= maxSamples) ? (totalSamples - i) : defaultSamples;
              SAudioBuffer destBuffer(outFormat, numSamples);
              destBuffer.setTimeStamp(timeStamp);

              postFilter(
                  reinterpret_cast<qint16 *>(destBuffer.data()),
                  reinterpret_cast<const qint16 *>(temp.data()) + ((i * sampleSize) / sizeof(qint16)),
                  numSamples * sampleSize,
                  outFormat.numChannels());

//              static STime lastTs = STime::fromSec(0);
//              qDebug() << "Audio timestamp" << timeStamp.toMSec() << (timeStamp - lastTs).toMSec()
//                  << ", pts =" << audioBuffer.presentationTimeStamp().toMSec()
//                  << ", dts =" << audioBuffer.decodingTimeStamp().toMSec()
//                  << ", duration =" << destBuffer.duration().toMSec();
//              lastTs = timeStamp;

              output << destBuffer;

              timeStamp += STime::fromClock(destBuffer.numSamples(), contextHandle->sample_rate);

              i += numSamples;
            }
          }

          inPtr += len;
          inSize -= len;
          continue;
        }

        break;
      }
    }
  }
  else
    output << SAudioBuffer(); // Flush

  return output;
}


} } // End of namespaces
