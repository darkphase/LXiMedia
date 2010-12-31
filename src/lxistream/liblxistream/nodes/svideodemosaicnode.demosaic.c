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
#include <stdint.h>
#include <string.h>
#include "spixels.h"

void LXiStream_SVideoDemosaicNode_demosaic_GRBG8
 (const uint8_t * restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint8_t * restrict dstData, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const restrict srcLine1 = srcData + (srcStride * y);
    const uint8_t * const restrict srcLine2 = srcLine1 + srcStride;
    struct RGBAPixel * const restrict dstLine1 = dstData + (dstStride * y);
    struct RGBAPixel * const restrict dstLine2 = ((uint8_t *)dstLine1) + dstStride;

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine1[x+1].r =
      dstLine2[x+0].r = dstLine2[x+1].r = srcLine1[x+1];

      dstLine1[x+0].g = dstLine1[x+1].g = srcLine1[x+0];
      dstLine2[x+0].g = dstLine2[x+1].g = srcLine2[x+1];

      dstLine1[x+0].b = dstLine1[x+1].b =
      dstLine2[x+0].b = dstLine2[x+1].b = srcLine2[x+0];
    }
  }
} __attribute__((nonnull(1, 5)));

void LXiStream_SVideoDemosaicNode_demosaic_GBRG8
 (const uint8_t * restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint8_t * restrict dstData, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const restrict srcLine1 = srcData + (srcStride * y);
    const uint8_t * const restrict srcLine2 = srcLine1 + srcStride;
    struct RGBAPixel * const restrict dstLine1 = dstData + (dstStride * y);
    struct RGBAPixel * const restrict dstLine2 = ((uint8_t *)dstLine1) + dstStride;

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine1[x+1].r =
      dstLine2[x+0].r = dstLine2[x+1].r = srcLine2[x+0];

      dstLine1[x+0].g = dstLine1[x+1].g = srcLine1[x+0];
      dstLine2[x+0].g = dstLine2[x+1].g = srcLine2[x+1];

      dstLine1[x+0].b = dstLine1[x+1].b =
      dstLine2[x+0].b = dstLine2[x+1].b = srcLine1[x+1];
    }
  }
} __attribute__((nonnull(1, 5)));

void LXiStream_SVideoDemosaicNode_demosaic_BGGR8
 (const uint8_t * restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint8_t * restrict dstData, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const restrict srcLine1 = srcData + (srcStride * y);
    const uint8_t * const restrict srcLine2 = srcLine1 + srcStride;
    struct RGBAPixel * const restrict dstLine1 = dstData + (dstStride * y);
    struct RGBAPixel * const restrict dstLine2 = ((uint8_t *)dstLine1) + dstStride;

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].b = dstLine1[x+1].b =
      dstLine2[x+0].b = dstLine2[x+1].b = srcLine1[x+0];

      dstLine1[x+0].g = dstLine1[x+1].g = srcLine1[x+1];
      dstLine2[x+0].g = dstLine2[x+1].g = srcLine2[x+0];

      dstLine1[x+0].r = dstLine1[x+1].r =
      dstLine2[x+0].r = dstLine2[x+1].r = srcLine2[x+1];
    }
  }
} __attribute__((nonnull(1, 5)));

void LXiStream_SVideoDemosaicNode_demosaic_RGGB8
 (const uint8_t * restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint8_t * restrict dstData, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const restrict srcLine1 = srcData + (srcStride * y);
    const uint8_t * const restrict srcLine2 = srcLine1 + srcStride;
    struct RGBAPixel * const restrict dstLine1 = dstData + (dstStride * y);
    struct RGBAPixel * const restrict dstLine2 = ((uint8_t *)dstLine1) + dstStride;

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine1[x+1].r =
      dstLine2[x+0].r = dstLine2[x+1].r = srcLine1[x+0];

      dstLine1[x+0].g = dstLine1[x+1].g = srcLine1[x+1];
      dstLine2[x+0].g = dstLine2[x+1].g = srcLine2[x+0];

      dstLine1[x+0].b = dstLine1[x+1].b =
      dstLine2[x+0].b = dstLine2[x+1].b = srcLine2[x+1];
    }
  }
} __attribute__((nonnull(1, 5)));

void LXiStream_SVideoDemosaicNode_demosaic_postfilter
 (uint8_t * restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines)
{
  uint8_t backupLineData[srcStride * 2];
  uint8_t * const backupLine[2] = { backupLineData, backupLineData + srcStride };
  memcpy(backupLine[0], srcData, srcStride);

  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=1; y<srcNumLines-1; y++)
  {
    struct RGBAPixel * const restrict bakLine = backupLine[y % 2];
    struct RGBAPixel * const restrict inLine = srcData + (srcStride * y);
    memcpy(bakLine, inLine, srcStride);

    const struct RGBAPixel * const restrict srcLine1 = backupLine[(y - 1) % 2];
    const struct RGBAPixel * const restrict srcLine2 = bakLine;
    const struct RGBAPixel * const restrict srcLine3 = ((uint8_t *)inLine) + srcStride;
    struct RGBAPixel * const restrict dstLine = inLine;

    for (unsigned x=1; x<srcWidth-1; x++)
    {
      dstLine[x].r =
          (srcLine2[x-1].r >> 2) + (srcLine2[x+1].r >> 2) +
          (srcLine1[x+0].r >> 2) + (srcLine3[x+0].r >> 2);

      dstLine[x].g =
          (srcLine2[x-1].g >> 2) + (srcLine2[x+1].g >> 2) +
          (srcLine1[x+0].g >> 2) + (srcLine3[x+0].g >> 2);

      dstLine[x].b =
          (srcLine2[x-2].b >> 2) + (srcLine2[x+2].b >> 2) +
          (srcLine1[x+0].b >> 2) + (srcLine3[x+0].b >> 2);

      dstLine[x].a = 0xFF;
    }
  }
} __attribute__((nonnull(1)));
