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
#include <stdint.h>

int16_t LXiStream_SAudioNormalizeNode_measure
 (const int16_t * __restrict srcData, unsigned numSamples, unsigned srcNumChannels)
{
  const unsigned totalSamples = numSamples * srcNumChannels;
  unsigned i;
  int64_t result = 0;

  for (i=0; i<totalSamples; i++)
    result += srcData[i] < 0 ? -srcData[i] : srcData[i];

  return (int16_t)(result / totalSamples);
}

void LXiStream_SAudioNormalizeNode_gain
 (const int16_t * srcData, unsigned numSamples, unsigned srcNumChannels,
  int16_t * dstData, int factor)
{
  const unsigned totalSamples = numSamples * srcNumChannels;
  unsigned i;
  int result;

  for (i=0; i<totalSamples; i++)
  {
    result = ((int)srcData[i]) * factor;
    result >>= 8;

    dstData[i] = (int16_t)(result >= -32768 ? (result <= 32767 ? result : 32767) : -32768);
  }
}
