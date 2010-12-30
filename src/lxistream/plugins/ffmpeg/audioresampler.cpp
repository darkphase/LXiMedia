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

#include "audioresampler.h"

namespace LXiStream {
namespace FFMpegBackend {


AudioResampler::AudioResampler(const QString &, QObject *parent)
  : SInterfaces::AudioResampler(parent),
    outFormat(SAudioFormat::Format_PCM_S16, 0, 0),
    reSampleContext(NULL),
    inSampleRate(0),
    inNumChannels(0),
    reopen(false)
{
}

AudioResampler::~AudioResampler()
{
  SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

  if (reSampleContext)
  {
    audio_resample_close(reSampleContext);
    reSampleContext = NULL;
  }
}

void AudioResampler::setFormat(const SAudioFormat &f)
{
  outFormat = f;
}

SAudioFormat AudioResampler::format(void)
{
  return outFormat;
}

SAudioBuffer AudioResampler::processBuffer(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull())
  if ((outFormat.sampleRate() > 0) || (outFormat.numChannels() > 0))
  {
    Q_ASSERT(audioBuffer.format() == SAudioFormat::Format_PCM_S16);

    const SAudioFormat::Channels outChannels = outFormat.numChannels() > 0 ? outFormat.channelSetup() : audioBuffer.format().channelSetup();
    const int outNumChannels = SAudioFormat::numChannels(outChannels);
    const int outSampleRate = outFormat.sampleRate() > 0 ? outFormat.sampleRate() : audioBuffer.format().sampleRate();

    if (audioBuffer.format() == SAudioFormat::Format_PCM_S16)
    if ((audioBuffer.format().sampleRate() != outSampleRate) ||
        (audioBuffer.format().numChannels() != outNumChannels))
    {
      if ((inSampleRate != audioBuffer.format().sampleRate()) ||
          (inNumChannels != audioBuffer.format().numChannels()))
      {
        SDebug::MutexLocker f(FFMpegCommon::mutex(), __FILE__, __LINE__);

        if (reSampleContext)
        {
          audio_resample_close(reSampleContext);
          reSampleContext = NULL;
        }

        inNumChannels = audioBuffer.format().numChannels();
        inSampleRate = audioBuffer.format().sampleRate();

#if ((LIBAVCODEC_VERSION_INT >> 16) >= 52)
        reSampleContext = av_audio_resample_init(outNumChannels, inNumChannels,
                                                 outSampleRate, inSampleRate,
                                                 SAMPLE_FMT_S16, SAMPLE_FMT_S16,
                                                 16, 10, 1, 0.95);
#else
        reSampleContext = audio_resample_init(outNumChannels, inNumChannels,
                                              outSampleRate, inSampleRate);
#endif
      }

      if (reSampleContext)
      {
        // Estimate buffer size
        const unsigned estNumSamples = STime::fromClock(audioBuffer.numSamples(), inSampleRate).toClock(outSampleRate) + 32;
        SAudioBuffer destBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16, outChannels, outSampleRate), estNumSamples);

        const unsigned numOutSamples =
            qMax(audio_resample(reSampleContext,
                                (short *)destBuffer.data(),
                                (short *)audioBuffer.data(),
                                audioBuffer.numSamples()), 0);

        if (numOutSamples > 0)
        {
          destBuffer.setNumSamples(numOutSamples);
          destBuffer.setTimeStamp(audioBuffer.timeStamp());
          return destBuffer;
        }
      }
    }
  }

  return audioBuffer;
}


} } // End of namespaces
