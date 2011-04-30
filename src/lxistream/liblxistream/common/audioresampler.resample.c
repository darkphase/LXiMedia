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
#define swap_i16(ap, bp) { int16_t *a = ap, *b = bp; int16_t t = *a; *a = *b; *b = t; }
#define merge_samples(a, aw, b, bw) ((int16_t)(((float)(a) * aw) + ((float)(b) * bw)))

unsigned LXiStream_Common_AudioResampler_resampleAudio
 (const int16_t * __restrict srcData, unsigned srcSampleRate, unsigned numSamples, unsigned srcNumChannels,
  int16_t * __restrict dstData, unsigned dstSampleRate, unsigned maxSamples, unsigned dstNumChannels,
  unsigned *pNextPos, float *pWeightOffset)
{
  if (numSamples > 0)
  {
    const unsigned nextPos = *pNextPos;
    const unsigned endPos = numSamples * srcNumChannels;
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
      const unsigned srcPosA = (nextPos + ((unsigned)srcPos)) * srcNumChannels;
      const unsigned srcPosB = srcPosA + srcNumChannels;
      const unsigned dstPos = i * dstNumChannels;

      if (srcPosB < endPos)
      {
        if (srcNumChannels <= dstNumChannels)
        {
          if ((srcNumChannels == 2) && (dstNumChannels == 4))
          {
            dstData[dstPos] = dstData[dstPos + 2] =
                merge_samples(srcData[srcPosA], sampleAweight,
                              srcData[srcPosB], sampleBweight);
            dstData[dstPos + 1] = dstData[dstPos + 3] =
                merge_samples(srcData[srcPosA + 1], sampleAweight,
                              srcData[srcPosB + 1], sampleBweight);
          }
          else if ((srcNumChannels == 2) && (dstNumChannels >= 3))
          {
            dstData[dstPos] =
                merge_samples(srcData[srcPosA], sampleAweight,
                              srcData[srcPosB], sampleBweight);
            dstData[dstPos + 1] = 0;
            dstData[dstPos + 2] =
                merge_samples(srcData[srcPosA + 1], sampleAweight,
                              srcData[srcPosB + 1], sampleBweight);

            for (j=3; j<dstNumChannels; j++)
              dstData[dstPos + j] = 0;
          }
          else if ((srcNumChannels == 1) && (dstNumChannels == 2))
          {
            dstData[dstPos] = dstData[dstPos + 1] =
                merge_samples(srcData[srcPosA], sampleAweight,
                              srcData[srcPosB], sampleBweight);
          }
          else if ((srcNumChannels == 1) && (dstNumChannels == 4))
          {
            dstData[dstPos] = dstData[dstPos + 1] =
            dstData[dstPos + 2] = dstData[dstPos + 3] =
                merge_samples(srcData[srcPosA], sampleAweight,
                              srcData[srcPosB], sampleBweight);
          }
          else if ((srcNumChannels == 1) && (dstNumChannels >= 3))
          {
            dstData[dstPos] = 0;
            dstData[dstPos + 1] =
                merge_samples(srcData[srcPosA], sampleAweight,
                              srcData[srcPosB], sampleBweight);

            for (j=2; j<dstNumChannels; j++)
              dstData[dstPos + j] = 0;
          }
          else
          {
            for (j=0; j<srcNumChannels; j++)
              dstData[dstPos + j] =
                  merge_samples(srcData[srcPosA + j], sampleAweight,
                                srcData[srcPosB + j], sampleBweight);

            for (j=srcNumChannels; j<dstNumChannels; j++)
              dstData[dstPos + j] = 0;
          }
        }
        else // srcNumChannels > dstNumChannels
        {
          if ((srcNumChannels == 4) && (dstNumChannels == 2))
          {
            dstData[dstPos] =
                (merge_samples(srcData[srcPosA], sampleAweight,
                               srcData[srcPosB], sampleBweight) >> 1) +
                (merge_samples(srcData[srcPosA + 2], sampleAweight,
                               srcData[srcPosB + 2], sampleBweight) >> 1);
            dstData[dstPos + 1] =
                (merge_samples(srcData[srcPosA + 1], sampleAweight,
                               srcData[srcPosB + 1], sampleBweight) >> 1) +
                (merge_samples(srcData[srcPosA + 3], sampleAweight,
                               srcData[srcPosB + 3], sampleBweight) >> 1);
          }
          else if ((srcNumChannels >= 3) && (dstNumChannels == 2))
          {
            dstData[dstPos] =
                (merge_samples(srcData[srcPosA], sampleAweight,
                               srcData[srcPosB], sampleBweight) >> 1) +
                (merge_samples(srcData[srcPosA + 1], sampleAweight,
                               srcData[srcPosB + 1], sampleBweight) >> 1);
            dstData[dstPos + 1] =
                (merge_samples(srcData[srcPosA + 2], sampleAweight,
                               srcData[srcPosB + 2], sampleBweight) >> 1) +
                (merge_samples(srcData[srcPosA + 1], sampleAweight,
                               srcData[srcPosB + 1], sampleBweight) >> 1);
            // \todo Merge in rear channels.
          }
          else
          {
            dstData[dstPos] = 0;
            for (j=0; j<srcNumChannels; j++)
              dstData[dstPos] +=
                  merge_samples(srcData[srcPosA + j], sampleAweight,
                                srcData[srcPosB + j], sampleBweight) / srcNumChannels;

            for (j=1; j<dstNumChannels; j++)
              dstData[dstPos + j] = dstData[dstPos];
          }
        }
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
