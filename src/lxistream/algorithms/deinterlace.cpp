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

#include "deinterlace.h"
#include <lxivecintrin/vectypes>

namespace LXiStream {
namespace Algorithms {

using namespace lxivec;

void Deinterlace::blendFields(uint8_t * dst, const uint8_t * srca, const uint8_t * srcb, int n)
{
  for (int i = 0; i < n; i += uint8_v::count)
  {
    const int16_dv as = uint8_v::load(srca + i);
    const int16_dv bs = uint8_v::load(srcb + i);

    const uint8_v result = (as + bs) >> 1;
    store(dst + i, result);
  }
}

void Deinterlace::smartBlendFields(uint8_t * dst, const uint8_t * srca, const uint8_t * srcb, const uint8_t * srcc, int n)
{
  for (int i = 0; i < n; i += uint8_v::count)
  {
    const int16_dv as = uint8_v::load(srca + i);
    const int16_dv bs = uint8_v::load(srcb + i);
    const int16_dv cs = uint8_v::load(srcc + i);

    const bool16_dv mask = max(abs(as - bs), abs(bs - cs)) > (abs(as - cs) + 8);

    const uint8_v result = select(mask, (as + cs) >> 1, bs);
    store(dst + i, result);
  }
}

} } // End of namespaces
