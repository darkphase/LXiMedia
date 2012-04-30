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

#include "videoprocess.h"
#include <cmath>
#include <lxivecintrin/vectypes>

namespace LXiStream {
namespace Algorithms {

using namespace lxivec;

#if defined(__GNUC__)
struct __attribute__((packed)) VideoProcess::YUVAPixel { uint8_t y, u, v, a; };
#elif defined(_MSC_VER)
# pragma pack(1)
struct VideoProcess::YUVAPixel { uint8_t y, u, v, a; };
# pragma pack()
#else
struct VideoProcess::YUVAPixel { uint8_t y, u, v, a; };
#endif

void VideoProcess::mixSubpictureYUV(
    const YUVData &yuvData, const SubpictureRect &rect,
    const void *palette, unsigned paletteSize)
{
  // Render Y
  for (unsigned y=0; y<rect.height; y++)
  {
    const unsigned yp = rect.y + y;
    uint8_t * dstLine = yuvData.y + (yuvData.yStride * yp);
    const uint8_t * srcLine = rect.lines + (rect.lineStride * y);
    for (unsigned x=0; x<rect.width; x++)
    if (srcLine[x] * sizeof(YUVAPixel) < paletteSize)
    {
      const YUVAPixel pixel = ((YUVAPixel *)palette)[srcLine[x]];
      const unsigned xp = rect.x + x;

      dstLine[xp] = (uint8_t)((
          (((int)dstLine[xp]) * (256 - ((int)pixel.a))) +
          (((int)pixel.y) * (1 + ((int)pixel.a)))) >> 8);
    }
  }

  // Render U and V
  for (unsigned y=0; y<rect.height; y++)
  {
    const unsigned yp = (rect.y + y) / yuvData.hr;
    uint8_t * dstLineU = yuvData.u + (yuvData.uStride * yp);
    uint8_t * dstLineV = yuvData.v + (yuvData.vStride * yp);
    const uint8_t * srcLine = rect.lines + (rect.lineStride * y);
    for (unsigned x=0; x<rect.width; x+=yuvData.wr)
    if (srcLine[x] * sizeof(YUVAPixel) < paletteSize)
    {
      const YUVAPixel pixel = ((YUVAPixel *)palette)[srcLine[x]];
      const unsigned xp = (rect.x + x) / yuvData.wr;

      dstLineU[xp] = (uint8_t)((
          (((int)dstLineU[xp]) * (256 - ((int)pixel.a))) +
          (((int)pixel.u) * (1 + ((int)pixel.a)))) >> 8);

      dstLineV[xp] = (uint8_t)((
          (((int)dstLineV[xp]) * (256 - ((int)pixel.a))) +
          (((int)pixel.v) * (1 + ((int)pixel.a)))) >> 8);
    }
  }
}

} } // End of namespaces
