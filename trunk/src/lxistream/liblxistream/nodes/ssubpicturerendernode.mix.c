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
#include <math.h>
#include <stdint.h>
#include <string.h>
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
  // Render Y
  for (unsigned y=0; y<rect->height; y++)
  {
    const unsigned yp = rect->y + y;
    uint8_t * restrict dstLine = yuvData->y + (yuvData->yStride * yp);
    const uint8_t * restrict srcLine = rect->lines + (rect->lineStride * y);
    for (unsigned x=0; x<rect->width; x++)
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
  for (unsigned y=0; y<rect->height; y++)
  {
    const unsigned yp = (rect->y + y) / yuvData->hr;
    uint8_t * restrict dstLineU = yuvData->u + (yuvData->uStride * yp);
    uint8_t * restrict dstLineV = yuvData->v + (yuvData->vStride * yp);
    const uint8_t * restrict srcLine = rect->lines + (rect->lineStride * y);
    for (unsigned x=0; x<rect->width; x+=yuvData->wr)
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
} __attribute__((nonnull));
