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

#include <sys/types.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "spixels.h"

// Keep this structure in sync with the one defined in ssubpicturerendernode.cpp
struct Rect
{
  int                           x, y;
  unsigned                      width, height;
  unsigned                      lineStride;
  const uint8_t               * lines;
};


void LXiStream_SSubpictureRenderNode_mixSubpictureYUV
   (struct YUVData *yuvData, struct Rect *rect, const void *palette, unsigned paletteSize)
{
  unsigned x, y;

  // Render Y
  for (y=0; y<rect->height; y++)
  {
    const unsigned yp = rect->y + y;
    uint8_t * __restrict dstLine = yuvData->y + (yuvData->yStride * yp);
    const uint8_t * __restrict srcLine = rect->lines + (rect->lineStride * y);
    for (x=0; x<rect->width; x++)
    if (srcLine[x] * sizeof(YUVAPixel) < paletteSize)
    {
      const YUVAPixel pixel = ((YUVAPixel *)palette)[srcLine[x]];
      const unsigned xp = rect->x + x;

      dstLine[xp] = (uint8_t)((
          (((int)dstLine[xp]) * (256 - ((int)pixel.a))) +
          (((int)pixel.y) * (1 + ((int)pixel.a)))) >> 8);
    }
  }

  // Render U and V
  for (y=0; y<rect->height; y++)
  {
    const unsigned yp = (rect->y + y) / yuvData->hr;
    uint8_t * __restrict dstLineU = yuvData->u + (yuvData->uStride * yp);
    uint8_t * __restrict dstLineV = yuvData->v + (yuvData->vStride * yp);
    const uint8_t * __restrict srcLine = rect->lines + (rect->lineStride * y);
    for (x=0; x<rect->width; x+=yuvData->wr)
    if (srcLine[x] * sizeof(YUVAPixel) < paletteSize)
    {
      const YUVAPixel pixel = ((YUVAPixel *)palette)[srcLine[x]];
      const unsigned xp = (rect->x + x) / yuvData->wr;

      dstLineU[xp] = (uint8_t)((
          (((int)dstLineU[xp]) * (256 - ((int)pixel.a))) +
          (((int)pixel.u) * (1 + ((int)pixel.a)))) >> 8);

      dstLineV[xp] = (uint8_t)((
          (((int)dstLineV[xp]) * (256 - ((int)pixel.a))) +
          (((int)pixel.v) * (1 + ((int)pixel.a)))) >> 8);
    }
  }
}
