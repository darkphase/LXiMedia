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

} } // End of namespaces
