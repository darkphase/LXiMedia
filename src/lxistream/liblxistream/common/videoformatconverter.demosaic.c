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

void LXiStream_Common_VideoFormatConverter_demosaic_GRBG8
 (const uint8_t * restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint8_t * restrict dstData, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const restrict srcLine1 = srcData + (srcStride * y);
    const uint8_t * const restrict srcLine2 = srcLine1 + srcStride;
    RGBAPixel * const restrict dstLine1 = dstData + (dstStride * y);
    RGBAPixel * const restrict dstLine2 = ((uint8_t *)dstLine1) + dstStride;

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine2[x+1].r = srcLine1[x+1];
      dstLine1[x+0].g = srcLine1[x+0];
      dstLine2[x+1].g = srcLine2[x+1];
      dstLine1[x+0].b = dstLine2[x+1].b = srcLine2[x+0];
      dstLine1[x+0].a = dstLine2[x+1].a = 0xFF;
      dstLine1[x+1].a = dstLine2[x+0].a = 0x00;
    }
  }
} __attribute__((nonnull(1, 5)));

void LXiStream_Common_VideoFormatConverter_demosaic_GBRG8
 (const uint8_t * restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint8_t * restrict dstData, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const restrict srcLine1 = srcData + (srcStride * y);
    const uint8_t * const restrict srcLine2 = srcLine1 + srcStride;
    RGBAPixel * const restrict dstLine1 = dstData + (dstStride * y);
    RGBAPixel * const restrict dstLine2 = ((uint8_t *)dstLine1) + dstStride;

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine2[x+1].r = srcLine2[x+0];
      dstLine1[x+0].g = srcLine1[x+0];
      dstLine2[x+1].g = srcLine2[x+1];
      dstLine1[x+0].b = dstLine2[x+1].b = srcLine1[x+1];
      dstLine1[x+0].a = dstLine2[x+1].a = 0xFF;
      dstLine1[x+1].a = dstLine2[x+0].a = 0x00;
    }
  }
} __attribute__((nonnull(1, 5)));

void LXiStream_Common_VideoFormatConverter_demosaic_BGGR8
 (const uint8_t * restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint8_t * restrict dstData, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const restrict srcLine1 = srcData + (srcStride * y);
    const uint8_t * const restrict srcLine2 = srcLine1 + srcStride;
    RGBAPixel * const restrict dstLine1 = dstData + (dstStride * y);
    RGBAPixel * const restrict dstLine2 = ((uint8_t *)dstLine1) + dstStride;

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine2[x+1].r = srcLine2[x+1];
      dstLine1[x+0].g = srcLine1[x+1];
      dstLine2[x+1].g = srcLine2[x+0];
      dstLine1[x+0].b = dstLine2[x+1].b = srcLine1[x+0];
      dstLine1[x+0].a = dstLine2[x+1].a = 0xFF;
      dstLine1[x+1].a = dstLine2[x+0].a = 0x00;
    }
  }
} __attribute__((nonnull(1, 5)));

void LXiStream_Common_VideoFormatConverter_demosaic_RGGB8
 (const uint8_t * restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint8_t * restrict dstData, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const restrict srcLine1 = srcData + (srcStride * y);
    const uint8_t * const restrict srcLine2 = srcLine1 + srcStride;
    RGBAPixel * const restrict dstLine1 = dstData + (dstStride * y);
    RGBAPixel * const restrict dstLine2 = ((uint8_t *)dstLine1) + dstStride;

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine2[x+1].r = srcLine1[x+0];
      dstLine1[x+0].g = srcLine1[x+1];
      dstLine2[x+1].g = srcLine2[x+0];
      dstLine1[x+0].b = dstLine2[x+1].b = srcLine2[x+1];
      dstLine1[x+0].a = dstLine2[x+1].a = 0xFF;
      dstLine1[x+1].a = dstLine2[x+0].a = 0x00;
    }
  }
} __attribute__((nonnull(1, 5)));

inline int8_t babs(int8_t val)
{
  return val >= 0 ? val : -val;
}

void LXiStream_Common_VideoFormatConverter_demosaic_postfilter
 (uint8_t * restrict data, unsigned width, unsigned stride, unsigned numLines)
{
  const int yo = ((const RGBAPixel *)data)->a == 0 ? 1 : 0;

  if ((numLines > 1) && (width > 1))
  {
    for (unsigned y=1; y<numLines-1; y++)
    {
      RGBAPixel * const restrict line2 = data + (stride * y);
      RGBAPixel * const restrict line1 = ((uint8_t *)line2) - stride;
      RGBAPixel * const restrict line3 = ((uint8_t *)line2) + stride;

      for (unsigned x=1+((y+yo)%2); x<width-1; x+=2)
      {
        const int8_t gw = line2[x-1].g >> 1;
        const int8_t ge = line2[x+1].g >> 1;
        const int8_t gn = line1[x+0].g >> 1;
        const int8_t gs = line3[x+0].g >> 1;

        if (babs(gw - ge) < babs(gn - gs))
        {
          line2[x].r = (line2[x-1].r >> 1) + (line2[x+1].r >> 1);
          line2[x].g = gw + ge;
          line2[x].b = (line2[x-1].b >> 1) + (line2[x+1].b >> 1);
        }
        else
        {
          line2[x].r = (line1[x+0].r >> 1) + (line3[x+0].r >> 1);
          line2[x].g = gn + gs;
          line2[x].b = (line1[x+0].b >> 1) + (line3[x+0].b >> 1);
        }

        line2[x].a = 0xFF;
      }

      // Side pixels
      const int n = (((y+yo)%2) == 0) ? (width - 1) : 0;
      line2[n].r = (line1[n].r >> 1) + (line3[n].r >> 1);
      line2[n].g = (line1[n].g >> 1) + (line3[n].g >> 1);
      line2[n].b = (line1[n].b >> 1) + (line3[n].b >> 1);
      line2[n].a = 0xFF;
    }

    // Top line
    RGBAPixel * const restrict topLine = data;
    for (unsigned x=1+(yo%2); x<width-1; x+=2)
    {
      topLine[x].r = (topLine[x-1].r >> 1) + (topLine[x+1].r >> 1);
      topLine[x].g = (topLine[x-1].g >> 1) + (topLine[x+1].g >> 1);
      topLine[x].b = (topLine[x-1].b >> 1) + (topLine[x+1].b >> 1);
      topLine[x].a = 0xFF;
    }

    topLine[0]       = topLine[1];
    topLine[width-1] = topLine[width-2];

    // Bottom line
    RGBAPixel * const restrict botLine = data + (stride * (numLines-1));
    for (unsigned x=1+(((numLines-1)+yo)%2); x<width-1; x+=2)
    {
      botLine[x].r = (botLine[x-1].r >> 1) + (botLine[x+1].r >> 1);
      botLine[x].g = (botLine[x-1].g >> 1) + (botLine[x+1].g >> 1);
      botLine[x].b = (botLine[x-1].b >> 1) + (botLine[x+1].b >> 1);
      botLine[x].a = 0xFF;
    }

    botLine[0]       = botLine[1];
    botLine[width-1] = botLine[width-2];
  }
} __attribute__((nonnull(1)));
