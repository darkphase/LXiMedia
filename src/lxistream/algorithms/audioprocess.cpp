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

#include "audioprocess.h"
#include <cmath>
#include <lxivecintrin/vectypes>

namespace LXiStream {
namespace Algorithms {

using namespace lxivec;

int16_t AudioProcess::avg(const int16_t * src, int n)
{
  int64_t result = 0;

  int i = 0;
  for (; i + int16_v::count <= n; i += int16_v::count)
    result += hsum(abs(int32_dv(int16_v::load(src + i))));

  for (; i < n; i++)
    result += lxivec::abs(int32_t(src[i]));

  return int16_t(result / n);
}

void AudioProcess::gain(int16_t * dst, const int16_t * src, int n, float g)
{
  const int gi = int((g + 0.5f) * 256.0f);

  for (int i = 0; i < n; i += int16_v::count)
  {
    int32_dv a = int16_v::load(src + i);
    a *= gi;
    a >>= 8;

    store(dst + i, int16_v(a));
  }
}

void AudioProcess::matrix(
    const int16_t * src, unsigned numSamples, unsigned srcNumChannels,
    int16_t * dst, const int * matrix, unsigned dstNumChannels)
{
  for (unsigned i=0; i<numSamples; i++)
  {
    for (unsigned dc=0; dc<dstNumChannels; dc++)
    {
      const int mp = dc * srcNumChannels;
      int result = int(src[0]) * matrix[mp];

      for (unsigned sc=1; sc<srcNumChannels; sc++)
        result += int(src[sc]) * matrix[mp + sc];

      result >>= 8;
      dst[dc] = int16_t(result >= -32768 ? (result <= 32767 ? result : 32767) : -32768);
    }

    src += srcNumChannels;
    dst += dstNumChannels;
  }
}

unsigned AudioProcess::resample(
    const int16_t * src, unsigned srcSampleRate, unsigned numSamples, unsigned numChannels,
    int16_t * dst, unsigned dstSampleRate, unsigned maxSamples,
    unsigned *pNextPos, float *pWeightOffset)
{
  if (numSamples > 0)
  {
    const unsigned nextPos = *pNextPos;
    const unsigned endPos = numSamples * numChannels;
    const float weightOffset = *pWeightOffset;
    const float dt = float(srcSampleRate) / float(dstSampleRate);
    const float idt = float(dstSampleRate) / float(srcSampleRate);
    static const unsigned delaySamples = 4; // The number of samples to be delayed for the next call (to prevent ticks).
    const unsigned dstNumSamples = (min(unsigned(float(numSamples - delaySamples - nextPos) * idt), maxSamples - 1) + 1) & ~(unsigned)(0x0001); // Only even numbers;

    for (unsigned i=0; i<dstNumSamples; i++)
    {
      const float srcPos = ((float)(i) * dt) + weightOffset;
      const float sampleBweight = srcPos - floorf(srcPos);
      const float sampleAweight = 1.0f - sampleBweight;
      const unsigned srcPosA = (nextPos + ((unsigned)srcPos)) * numChannels;
      const unsigned srcPosB = srcPosA + numChannels;
      const unsigned dstPos = i * numChannels;

      if (srcPosB < endPos)
      for (unsigned j=0; j<numChannels; j++)
      {
        dst[dstPos + j] =
            int16_t((float(src[srcPosA + j]) * sampleAweight) +
                    (float(src[srcPosB + j]) * sampleBweight));
      }
    }

    // Next position
    {
      const float np = (float(dstNumSamples) * dt) + weightOffset;
      *pWeightOffset = np - floorf(np);
      *pNextPos = nextPos + unsigned(np);
    }

    return dstNumSamples;
  }
  else
    return 0;
}

} } // End of namespaces
