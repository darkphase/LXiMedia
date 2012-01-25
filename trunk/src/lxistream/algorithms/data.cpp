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

#include "data.h"
#include <lxivecintrin/vectypes>

namespace LXiStream {
namespace Algorithms {

using namespace lxivec;

void Data::swapBytes(uint16_t * dst, const uint16_t * src, int n)
{
  for (int i = 0; i < n; i += uint16_v::count)
  {
    uint16_v a = uint16_v::load(src + i);
    a = (a >> 8) |
        (a << 8);

    store(dst + i, a);
  }
}

void Data::swapBytes(uint32_t * dst, const uint32_t * src, int n)
{
  for (int i = 0; i < n; i += uint32_v::count)
  {
    uint32_v a = uint32_v::load(src + i);
    a = (a >> 24) |
        ((a & 0x00FF0000) >> 8) |
        ((a & 0x0000FF00) << 8) |
        (a << 24);

    store(dst + i, a);
  }
}

void Data::swapBytes(uint64_t * dst, const uint64_t * src, int n)
{
  for (int i = 0; i < n; i += uint64_v::count)
  {
    uint64_v a = uint64_v::load(src + i);
    a = (a  << 56) |
        ((a << 40) & 0x00FF000000000000ull) |
        ((a << 24) & 0x0000FF0000000000ull) |
        ((a << 8)  & 0x000000FF00000000ull) |
        ((a >> 8)  & 0x00000000FF000000ull) |
        ((a >> 24) & 0x0000000000FF0000ull) |
        ((a >> 40) & 0x000000000000FF00ull) |
        (a  >> 56);

    store(dst + i, a);
  }
}

} } // End of namespaces
