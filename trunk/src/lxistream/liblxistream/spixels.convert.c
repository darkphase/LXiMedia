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

#include "spixels.h"
#ifdef __SSE2__
  #include <emmintrin.h>
#endif

YUVAPixel RGBA2YUVA(RGBAPixel p)
{
  YUVAPixel result;

  result.y = (uint8_t)(((( 66 * (int)(p.r)) + (129 * (int)(p.g)) + ( 25 * (int)(p.b)) + 128) >> 8) +  16);
  result.u = (uint8_t)((((-38 * (int)(p.r)) - ( 74 * (int)(p.g)) + (112 * (int)(p.b)) + 128) >> 8) + 128);
  result.v = (uint8_t)((((112 * (int)(p.r)) - ( 94 * (int)(p.g)) - ( 18 * (int)(p.b)) + 128) >> 8) + 128);
  result.a = p.a;

  return result;
}
