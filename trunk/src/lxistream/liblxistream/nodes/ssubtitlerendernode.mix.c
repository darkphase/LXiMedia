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

// Keep these structures in sync with the ones defined in ssubtitlerendernode.cpp
struct Lines
{
  uint8_t l[4][160];
};

struct Char
{
  unsigned advance, width, height;
  uint8_t pixels[0];
};

void LXiStream_SSubtitleRenderNode_mixSubtitle8
 (uint8_t * restrict srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
  uint8_t * restrict srcU, unsigned uStride, uint8_t * restrict srcV, unsigned vStride,
  unsigned wf, unsigned hf,
  const struct Lines * restrict lines, const struct Char * const * restrict characters)
{
  const unsigned lineHeight = characters[0]->height;
  unsigned line = srcHeight - (lineHeight * 4) - (lineHeight / 2);

  for (unsigned sl=0; sl<4; sl++)
  {
    unsigned lineWidth = 0;
    for (unsigned sc=0; sc<160; sc++)
    if (lines->l[sl][sc] > 0)
      lineWidth += characters[lines->l[sl][sc]]->advance;

    if ((lineWidth + (lineHeight * 2)) < srcWidth)
    for (unsigned ly=0; ly<lineHeight; ly++)
    {
      uint8_t * const restrict srcPix = srcData + (line * srcStride) + ((srcWidth / 2) - (lineWidth / 2));

      // Draw the shadow around the subtitles
      uint8_t * restrict pix = srcPix;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0; x<c->width; x++)
        if (__builtin_expect(pixels[x] == 3, 0))
          pix[x] >>= 2;

        pix += c->advance;
      }
      else
        break;

      // And draw the subtitles themselves
      pix = srcPix;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0; x<c->width; x++)
        if (__builtin_expect(pixels[x] > 3, 0))
          pix[x] = pixels[x];

        pix += c->advance;
      }
      else
        break;

      // And the color
      uint8_t * const restrict uPix = srcU + ((line / hf) * uStride) + (((srcWidth / wf) / 2) - ((lineWidth / wf) / 2));
      uint8_t * const restrict vPix = srcV + ((line / hf) * vStride) + (((srcWidth / wf) / 2) - ((lineWidth / wf) / 2));
      unsigned xx = 0;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0; x<c->width; x++)
        if (__builtin_expect(pixels[x] > 3, 0))
          uPix[(xx+x)/wf] = vPix[(xx+x)/wf] = (uint8_t)127;

        xx += c->advance;
      }
      else
        break;

      line++;
    }
  }
} __attribute__((nonnull));

void LXiStream_SSubtitleRenderNode_mixSubtitle8_stretch
 (uint8_t * restrict srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
  float srcAspect, uint8_t * restrict srcU, unsigned uStride, uint8_t * restrict srcV,
  unsigned vStride, unsigned wf, unsigned hf,
  const struct Lines * restrict lines, const struct Char * const * restrict characters)
{
  const unsigned lineHeight = characters[0]->height;
  unsigned line = srcHeight - (lineHeight * 4) - (lineHeight / 2);

  for (unsigned sl=0; sl<4; sl++)
  {
    unsigned lineWidth = 0;
    for (unsigned sc=0; sc<160; sc++)
    if (lines->l[sl][sc] > 0)
      lineWidth += characters[lines->l[sl][sc]]->advance;

    lineWidth = (unsigned)(lineWidth / srcAspect);

    if ((lineWidth + (lineHeight * 2)) < srcWidth)
    for (unsigned ly=0; ly<lineHeight; ly++)
    {
      uint8_t * const restrict srcPix = srcData + (line * srcStride) + ((srcWidth / 2) - (lineWidth / 2));

      // Draw the shadow around the subtitles
      uint8_t * restrict pix = srcPix;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0, n=(unsigned)(c->width/srcAspect); x<n; x++)
        if (__builtin_expect(pixels[(unsigned)(x*srcAspect)] == 3, 0))
          pix[x] >>= 2;

        pix += (unsigned)(c->advance/srcAspect);
      }
      else
        break;

      // And draw the subtitles themselves
      pix = srcPix;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0, n=(unsigned)(c->width/srcAspect)-1; x<n; x++)
        {
          const float srcPos = (float)(x) * srcAspect;
          const unsigned srcX = (unsigned)srcPos;

          if (__builtin_expect(pixels[srcX] > 3, 0) ||
              __builtin_expect(pixels[srcX + 1] > 3, 0))
          {
            const float sampleBweight = srcPos - (float)(srcX);
            const float sampleAweight = 1.0f - sampleBweight;

            pix[x] = (uint8_t)(((float)(pixels[srcX]) * sampleAweight) +
                               ((float)(pixels[srcX + 1]) * sampleBweight));
          }
        }

        pix += (unsigned)(c->advance/srcAspect);
      }
      else
        break;

      // And the color
      uint8_t * const restrict uPix = srcU + ((line / hf) * uStride) + (((srcWidth / wf) / 2) - ((lineWidth / wf) / 2));
      uint8_t * const restrict vPix = srcV + ((line / hf) * vStride) + (((srcWidth / wf) / 2) - ((lineWidth / wf) / 2));
      unsigned xx = 0;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0, n=(unsigned)(c->width/srcAspect); x<n; x++)
        if (__builtin_expect(pixels[(unsigned)(x*srcAspect)] > 3, 0))
          uPix[(xx+x)/wf] = vPix[(xx+x)/wf] = (uint8_t)127;

        xx += (unsigned)(c->advance/srcAspect);
      }
      else
        break;

      line++;
    }
  }
} __attribute__((nonnull));

void LXiStream_SSubtitleRenderNode_mixSubtitle32
 (uint32_t * restrict srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
  const struct Lines * restrict lines, const struct Char * const * restrict characters)
{
  const unsigned lineHeight = characters[0]->height;
  unsigned line = srcHeight - (lineHeight * 4) - (lineHeight / 2);

  for (unsigned sl=0; sl<4; sl++)
  {
    unsigned lineWidth = 0;
    for (unsigned sc=0; sc<160; sc++)
    if (lines->l[sl][sc] > 0)
      lineWidth += characters[lines->l[sl][sc]]->advance;

    if ((lineWidth + (lineHeight * 2)) < srcWidth)
    for (unsigned ly=0; ly<lineHeight; ly++)
    {
      uint8_t * const restrict srcPix = srcData + (line * srcStride) + ((srcWidth / 2) - (lineWidth / 2));

      // Draw the shadow around the subtitles
      uint32_t * restrict pix = srcPix;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0; x<c->width; x++)
        if (__builtin_expect(pixels[x] == 3, 0))
        {
          uint8_t * restrict p = (uint8_t *)(pix + x);
          p[0] >>= 2; p[1] >>= 2; p[2] >>= 2; p[3] >>= 2;
        }

        pix += c->advance;
      }
      else
        break;

      // And draw the subtitles themselves
      pix = srcPix;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0; x<c->width; x++)
        if (__builtin_expect(pixels[x] > 3, 0))
        {
          uint8_t * restrict p = (uint8_t *)(pix + x);
          p[0] = p[1] = p[2] = p[3] = pixels[x];
        }

        pix += c->advance;
      }
      else
        break;

      line++;
    }
  }
} __attribute__((nonnull));

void LXiStream_SSubtitleRenderNode_mixSubtitle32_stretch
 (uint32_t * restrict srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
  float srcAspect,
  const struct Lines * restrict lines, const struct Char * const * restrict characters)
{
  const unsigned lineHeight = characters[0]->height;
  unsigned line = srcHeight - (lineHeight * 4) - (lineHeight / 2);

  for (unsigned sl=0; sl<4; sl++)
  {
    unsigned lineWidth = 0;
    for (unsigned sc=0; sc<160; sc++)
    if (lines->l[sl][sc] > 0)
      lineWidth += characters[lines->l[sl][sc]]->advance;

    lineWidth = (unsigned)(lineWidth / srcAspect);

    if ((lineWidth + (lineHeight * 2)) < srcWidth)
    for (unsigned ly=0; ly<lineHeight; ly++)
    {
      uint8_t * const restrict srcPix = srcData + (line * srcStride) + ((srcWidth / 2) - (lineWidth / 2));

      // Draw the shadow around the subtitles
      uint32_t * restrict pix = srcPix;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0, n=(unsigned)(c->width/srcAspect); x<n; x++)
        if (__builtin_expect(pixels[(unsigned)(x*srcAspect)] == 3, 0))
        {
          uint8_t * restrict p = (uint8_t *)(pix + x);
          p[0] >>= 2; p[1] >>= 2; p[2] >>= 2; p[3] >>= 2;
        }

        pix += (unsigned)(c->advance/srcAspect);
      }
      else
        break;

      // And draw the subtitles themselves
      pix = srcPix;

      for (unsigned sc=0; sc<160; sc++)
      if (__builtin_expect(lines->l[sl][sc] > 0, 1))
      {
        const struct Char * const c = characters[lines->l[sl][sc]];
        const uint8_t * const pixels = c->pixels + (ly * c->width);

        for (unsigned x=0, n=(unsigned)(c->width/srcAspect)-1; x<n; x++)
        {
          const float srcPos = (float)(x) * srcAspect;
          const unsigned srcX = (unsigned)srcPos;

          if (__builtin_expect(pixels[srcX] > 3, 0) ||
              __builtin_expect(pixels[srcX + 1] > 3, 0))
          {
            const float sampleBweight = srcPos - (float)(srcX);
            const float sampleAweight = 1.0f - sampleBweight;

            uint8_t * restrict p = (uint8_t *)(pix + x);
            p[0] = p[1] = p[2] = p[3] = (uint8_t)(((float)(pixels[srcX]) * sampleAweight) +
                                                  ((float)(pixels[srcX + 1]) * sampleBweight));
          }
        }

        pix += (unsigned)(c->advance/srcAspect);
      }
      else
        break;

      line++;
    }
  }
} __attribute__((nonnull));
