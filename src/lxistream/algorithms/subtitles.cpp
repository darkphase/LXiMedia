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

#include "subtitles.h"
#include <lxivecintrin/vectypes>

namespace LXiStream {
namespace Algorithms {

using namespace lxivec;

void Subtitles::blendLineY(uint8_t * y, const uint8_t * shadow, const uint8_t * text, int n)
{
  for (int i = 0; i < n; i += uint8_v::count)
  {
    uint8_v yv = subs(uint8_v::loadu(y + i), uint8_v::load(shadow + i));
    yv = adds(yv, uint8_v::load(text + i));

    storeu(y + i, yv);
  }
}

void Subtitles::blendLineUV(uint8_t * u, uint8_t * v, int wf, const uint8_t * text, int n)
{
  const uint8_v zero = uint8_v::set(0);
  const uint8_v med = uint8_v::set(127);

  switch (wf)
  {
  case 1:
    for (int i = 0; i < n; i += uint8_v::count)
    {
      const bool8_v pv = uint8_v::load(text + i) == zero;

      storeu(u + i, select(pv, uint8_v::loadu(u + i), med));
      storeu(v + i, select(pv, uint8_v::loadu(v + i), med));
    }

    break;

  case 2:
    for (int i = 0, j = 0; j < n; i += uint8_v::count, j += uint8_v::count * 2)
    {
      const bool8_v pv = hadd(uint8_dv::load(text + j) >> 1) == zero;

      storeu(u + i, select(pv, uint8_v::loadu(u + i), med));
      storeu(v + i, select(pv, uint8_v::loadu(v + i), med));
    }

    break;

  case 4:
    for (int i = 0, j = 0; j < n; i += uint8_v::count, j += uint8_v::count * 4)
    {
      const bool8_v pv = hadd(hadd(uint8_qv::load(text + j) >> 1)) == zero;

      storeu(u + i, select(pv, uint8_v::loadu(u + i), med));
      storeu(v + i, select(pv, uint8_v::loadu(v + i), med));
    }

    break;
  }
}

} } // End of namespaces
