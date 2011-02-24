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

#include "spixels.h"
#include <sys/types.h>
#ifdef __SSE__
  #include <xmmintrin.h>
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
