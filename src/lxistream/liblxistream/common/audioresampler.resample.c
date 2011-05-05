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

#include <sys/types.h>
#include <math.h>
#include <stdint.h>

__inline unsigned min(unsigned a, unsigned b) { return a < b ? a : b; }
#define merge_samples(a, aw, b, bw)

unsigned LXiStream_Common_AudioResampler_resampleAudio
 (const int16_t * __restrict srcData, unsigned srcSampleRate, unsigned numSamples, unsigned numChannels,
  int16_t * __restrict dstData, unsigned dstSampleRate, unsigned maxSamples,
  unsigned *pNextPos, float *pWeightOffset)
{
  if (numSamples > 0)
  {
    const unsigned nextPos = *pNextPos;
    const unsigned endPos = numSamples * numChannels;
    const float weightOffset = *pWeightOffset;
    const float dt = (float)(srcSampleRate) / (float)(dstSampleRate);
    const float idt = (float)(dstSampleRate) / (float)(srcSampleRate);
    static const unsigned delaySamples = 4; // The number of samples to be delayed for the next call (to prevent ticks).
    const unsigned dstNumSamples = (min((unsigned)((float)(numSamples - delaySamples - nextPos) * idt), maxSamples - 1) + 1) & ~(unsigned)(0x0001); // Only even numbers;
    unsigned i, j;

    for (i=0; i<dstNumSamples; i++)
    {
      const float srcPos = ((float)(i) * dt) + weightOffset;
      const float sampleBweight = srcPos - floorf(srcPos);
      const float sampleAweight = 1.0f - sampleBweight;
      const unsigned srcPosA = (nextPos + ((unsigned)srcPos)) * numChannels;
      const unsigned srcPosB = srcPosA + numChannels;
      const unsigned dstPos = i * numChannels;

      if (srcPosB < endPos)
      for (j=0; j<numChannels; j++)
      {
        dstData[dstPos + j] =
            ((int16_t)(((float)(srcData[srcPosA + j]) * sampleAweight) +
                       ((float)(srcData[srcPosB + j]) * sampleBweight)));
      }
    }

    // Next position
    {
      const float np = ((float)(dstNumSamples) * dt) + weightOffset;
      *pWeightOffset = np - floorf(np);
      *pNextPos = nextPos + (unsigned)np;
    }

    return dstNumSamples;
  }
  else
    return 0;
}
