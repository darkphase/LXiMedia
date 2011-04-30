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
#include <string.h>
#include <stdint.h>

void LXiStream_SVideoBoxNode_boxVideo8
 (const uint8_t * __restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint8_t * __restrict dstData, unsigned dstWidth, unsigned dstStride, unsigned dstNumLines,
  int nullPixel)
{
  const unsigned inLineOffset = (dstNumLines < srcNumLines) ? ((srcNumLines - dstNumLines) / 2) : 0;
  const unsigned inPixelOffset = (dstWidth < srcWidth) ? ((srcWidth - dstWidth) / 2) : 0;
  const unsigned outLineOffset = (srcNumLines < dstNumLines) ? ((dstNumLines - srcNumLines) / 2) : 0;
  const unsigned outPixelOffset = (srcWidth < dstWidth) ? ((dstWidth - srcWidth) / 2) : 0;
  const unsigned outWidth = (srcWidth < dstWidth) ? srcWidth : dstWidth;
  unsigned line = 0;

  for (; line<outLineOffset; line++)
  {
    uint8_t * const __restrict dstLine = dstData + (line * dstStride);

    memset(dstLine, nullPixel, dstWidth);
  }

  for (; line<srcNumLines+outLineOffset; line++)
  {
    const uint8_t * const __restrict srcLine = srcData + ((line + inLineOffset - outLineOffset) * srcStride) + inPixelOffset;
    uint8_t * const __restrict dstLine = dstData + (line * dstStride);

    memset(dstLine, nullPixel, outPixelOffset);
    memcpy(dstLine + outPixelOffset, srcLine, outWidth);
    memset(dstLine + outPixelOffset + outWidth, nullPixel, outPixelOffset);
  }

  for (; line<dstNumLines; line++)
  {
    uint8_t * const __restrict dstLine = dstData + (line * dstStride);

    memset(dstLine, nullPixel, dstWidth);
  }
}


void LXiStream_SVideoBoxNode_boxVideo32
 (const uint32_t * __restrict srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
  uint32_t * __restrict dstData, unsigned dstWidth, unsigned dstStride, unsigned dstNumLines,
  uint32_t nullPixel)
{
  const unsigned inLineOffset = (dstNumLines < srcNumLines) ? ((srcNumLines - dstNumLines) / 2) : 0;
  const unsigned inPixelOffset = (dstWidth < srcWidth) ? ((srcWidth - dstWidth) / 2) : 0;
  const unsigned outLineOffset = (srcNumLines < dstNumLines) ? ((dstNumLines - srcNumLines) / 2) : 0;
  const unsigned outPixelOffset = (srcWidth < dstWidth) ? ((dstWidth - srcWidth) / 2) : 0;
  const unsigned outWidth = (srcWidth < dstWidth) ? srcWidth : dstWidth;
  unsigned line = 0, pix, x;

  for (; line<outLineOffset; line++)
  {
    uint32_t * const __restrict dstLine = (uint32_t *)((uint8_t *)dstData + (line * dstStride));

    for (pix=0; pix<dstWidth; pix++)
      dstLine[pix] = nullPixel;
  }

  for (; line<srcNumLines+outLineOffset; line++)
  {
    const uint32_t * const __restrict srcLine = ((uint32_t *)((uint8_t *)srcData + ((line + inLineOffset - outLineOffset) * srcStride))) + inPixelOffset;
    uint32_t * const __restrict dstLine = (uint32_t *)((uint8_t *)dstData + (line * dstStride));

    for (pix=0; pix<outPixelOffset; pix++)
      dstLine[pix] = nullPixel;

    for (x=0; x<outWidth; x++, pix++)
      dstLine[pix] = srcLine[x];

    for (x=0; x<outPixelOffset; x++, pix++)
      dstLine[pix] = nullPixel;
  }

  for (; line<dstNumLines; line++)
  {
    uint32_t * const __restrict dstLine = (uint32_t *)((uint8_t *)dstData + (line * dstStride));

    for (pix=0; pix<dstWidth; pix++)
      dstLine[pix] = nullPixel;
  }
}
