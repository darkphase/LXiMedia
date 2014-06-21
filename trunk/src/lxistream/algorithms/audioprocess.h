/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include <sys/types.h>
#include <stdint.h>

namespace LXiStream {
namespace Algorithms {

class AudioProcess
{
public:
  static int16_t avg(const int16_t * src, int n);

  static void gain(int16_t * dst, const int16_t * src, int n, float g);

  static void matrix(
      const int16_t * src, unsigned numSamples, unsigned srcNumChannels,
      int16_t * dst, const int * matrix, unsigned dstNumChannels);

  static unsigned resample(
      const int16_t * src, unsigned srcSampleRate, unsigned numSamples, unsigned numChannels,
      int16_t * dst, unsigned dstSampleRate, unsigned maxSamples,
      unsigned *pNextPos, float *pWeightOffset);
};

} } // End of namespaces
