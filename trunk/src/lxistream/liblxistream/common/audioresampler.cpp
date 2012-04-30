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

#include "audioresampler.h"
#include "../../algorithms/audioprocess.h"

namespace LXiStream {
namespace Common {


AudioResampler::AudioResampler(const QString &, QObject *parent)
    : SInterfaces::AudioResampler(parent),
      outSampleRate(48000),
      nextPos(0),
      weightOffset(0.0f)
{
}

void AudioResampler::setSampleRate(unsigned rate)
{
  outSampleRate = rate;
}

unsigned AudioResampler::sampleRate(void)
{
  return outSampleRate;
}

SAudioBuffer AudioResampler::processBuffer(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull() && (audioBuffer.format() == SAudioFormat::Format_PCM_S16))
  {
    Q_ASSERT(audioBuffer.format() == SAudioFormat::Format_PCM_S16);

    if (audioBuffer.format() != lastBuffer.format())
    {
      lastBuffer.clear();
      nextPos = 0;
      weightOffset = 0.0f;
    }

    if (outSampleRate > 0)
    {
      if (audioBuffer.format().sampleRate() > 0)
      if (outSampleRate != audioBuffer.format().sampleRate())
      {
        SAudioFormat outFormat = audioBuffer.format();
        outFormat.setSampleRate(outSampleRate);

        // Estimate buffer size
        const unsigned estNumSamples = STime::fromClock(audioBuffer.numSamples(), audioBuffer.format().sampleRate()).toClock(outSampleRate) + 32;
        const SAudioBuffer srcBuffer = SAudioBufferList() << lastBuffer << audioBuffer;
        SAudioBuffer destBuffer(outFormat, estNumSamples);

        const unsigned numOutSamples =
            Algorithms::AudioProcess::resample(
                reinterpret_cast<const qint16 *>(srcBuffer.data()),
                srcBuffer.format().sampleRate(),
                srcBuffer.numSamples(),
                srcBuffer.format().numChannels(),
                reinterpret_cast<qint16 *>(destBuffer.data()),
                outSampleRate,
                estNumSamples,
                &nextPos,
                &weightOffset);

        nextPos -= !lastBuffer.isNull() ? lastBuffer.numSamples() : 0;
        lastBuffer = audioBuffer;

        if (numOutSamples > 0)
        {
          destBuffer.setNumSamples(numOutSamples);
          destBuffer.setTimeStamp(srcBuffer.timeStamp());

          return destBuffer;
        }
      }
    }
  }

  return audioBuffer;
}

void AudioResampler::compensate(float)
{
}

} } // End of namespaces
