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
#include <string.h>
#include <stdint.h>

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
 (uint8_t * __restrict srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
  uint8_t * __restrict srcU, unsigned uStride, uint8_t * __restrict srcV, unsigned vStride,
  unsigned wf, unsigned hf,
  const struct Lines * __restrict lines, const struct Char * const * __restrict characters)
{
  const unsigned lineHeight = characters[0]->height;
  unsigned line = srcHeight - (lineHeight * 4) - (lineHeight / 2);
  unsigned sl, sc, ly, x;

  for (sl=0; sl<4; sl++)
  {
    unsigned lineWidth = 0;
    for (sc=0; sc<160; sc++)
    if (lines->l[sl][sc] > 0)
      lineWidth += characters[lines->l[sl][sc]]->advance;

    if ((lineWidth + (lineHeight * 2)) < srcWidth)
    for (ly=0; ly<lineHeight; ly++)
    {
      uint8_t * const __restrict srcPix = srcData + (line * srcStride) + ((srcWidth / 2) - (lineWidth / 2));

      // Draw the shadow around the subtitles
      uint8_t * __restrict pix = srcPix;

      for (sc=0; sc<160; sc++)
      if (lines->l[sl][sc] > 0)
      {
        const struct Char * const __restrict c = characters[lines->l[sl][sc]];
        const uint8_t * const __restrict pixels = c->pixels + (ly * c->width);

        for (x=0; x<c->width; x++)
        if (pixels[x] < 128)
          pix[x] = ((pixels[x] * 2) >= pix[x]) ? 0 : (pix[x] - (pixels[x] * 2));

        pix += c->advance;
      }
      else
        break;

      // And draw the subtitles themselves
      pix = srcPix;

      for (sc=0; sc<160; sc++)
      if (lines->l[sl][sc] > 0)
      {
        const struct Char * const __restrict c = characters[lines->l[sl][sc]];
        const uint8_t * const __restrict pixels = c->pixels + (ly * c->width);

        for (x=0; x<c->width; x++)
        if (pixels[x] >= 128)
          pix[x] = (pixels[x] - 128) * 2;

        pix += c->advance;
      }
      else
        break;

      // And the color
      {
        uint8_t * const __restrict uPix = srcU + ((line / hf) * uStride) + (((srcWidth / wf) / 2) - ((lineWidth / wf) / 2));
        uint8_t * const __restrict vPix = srcV + ((line / hf) * vStride) + (((srcWidth / wf) / 2) - ((lineWidth / wf) / 2));
        unsigned xx = 0;

        for (sc=0; sc<160; sc++)
        if (lines->l[sl][sc] > 0)
        {
          const struct Char * const __restrict c = characters[lines->l[sl][sc]];
          const uint8_t * const __restrict pixels = c->pixels + (ly * c->width);

          for (x=0; x<c->width; x++)
          if (pixels[x] >= 128)
            uPix[(xx+x)/wf] = vPix[(xx+x)/wf] = (uint8_t)127;

          xx += c->advance;
        }
        else
          break;
      }

      line++;
    }
  }
}

void LXiStream_SSubtitleRenderNode_mixSubtitle8_stretch
 (uint8_t * __restrict srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
  float srcAspect, uint8_t * __restrict srcU, unsigned uStride, uint8_t * __restrict srcV,
  unsigned vStride, unsigned wf, unsigned hf,
  const struct Lines * __restrict lines, const struct Char * const * __restrict characters)
{
  const unsigned lineHeight = characters[0]->height;
  unsigned line = srcHeight - (lineHeight * 4) - (lineHeight / 2);
  unsigned sl, ly, sc, x, n;

  for (sl=0; sl<4; sl++)
  {
    unsigned lineWidth = 0;
    for (sc=0; sc<160; sc++)
    if (lines->l[sl][sc] > 0)
      lineWidth += (unsigned)((characters[lines->l[sl][sc]]->advance / srcAspect) + 0.5f);

    if ((lineWidth + (lineHeight * 2)) < srcWidth)
    for (ly=0; ly<lineHeight; ly++)
    {
      uint8_t * const __restrict srcPix = srcData + (line * srcStride) + ((srcWidth / 2) - (lineWidth / 2));

      // Draw the shadow around the subtitles
      uint8_t * __restrict pix = srcPix;

      for (sc=0; sc<160; sc++)
      if (lines->l[sl][sc] > 0)
      {
        const struct Char * const __restrict c = characters[lines->l[sl][sc]];
        const uint8_t * __restrict const pixels = c->pixels + (ly * c->width);

        for (x=0, n=(unsigned)(c->width/srcAspect); x<n; x++)
        {
          const float srcPos = (float)(x) * srcAspect;
          const unsigned srcX = (unsigned)srcPos;
          if ((pixels[srcX] < 128) || (pixels[srcX + 1] < 128))
          {
            const float sampleBweight = srcPos - (float)(srcX);
            const float sampleAweight = 1.0f - sampleBweight;
            const uint8_t sampleA = (pixels[srcX] < 128) ? (pixels[srcX] * 2) : 0;
            const uint8_t sampleB = (pixels[srcX + 1] < 128) ? (pixels[srcX + 1] * 2) : 0;

            const uint8_t sample =
                (uint8_t)((((float)sampleA) * sampleAweight) +
                          (((float)sampleB) * sampleBweight));

            pix[x] = (sample >= pix[x]) ? 0 : (pix[x] - sample);
          }
        }

        pix += (unsigned)((c->advance / srcAspect) + 0.5f);
      }
      else
        break;

      // And draw the subtitles themselves
      pix = srcPix;

      for (sc=0; sc<160; sc++)
      if (lines->l[sl][sc] > 0)
      {
        const struct Char * const __restrict c = characters[lines->l[sl][sc]];
        const uint8_t * const __restrict pixels = c->pixels + (ly * c->width);

        for (x=0, n=(unsigned)(c->width/srcAspect)-1; x<n; x++)
        {
          const float srcPos = (float)(x) * srcAspect;
          const unsigned srcX = (unsigned)srcPos;
          if ((pixels[srcX] >= 128) || (pixels[srcX + 1] >= 128))
          {
            const float sampleBweight = srcPos - (float)(srcX);
            const float sampleAweight = 1.0f - sampleBweight;
            const uint8_t sampleA = (pixels[srcX] >= 128) ? ((pixels[srcX] - 128) * 2) : 0;
            const uint8_t sampleB = (pixels[srcX + 1] >= 128) ? ((pixels[srcX + 1] - 128) * 2) : 0;

            pix[x] = (uint8_t)((((float)sampleA) * sampleAweight) +
                               (((float)sampleB) * sampleBweight));
          }
        }

        pix += (unsigned)((c->advance / srcAspect) + 0.5f);
      }
      else
        break;

      // And the color
      {
        uint8_t * const __restrict uPix = srcU + ((line / hf) * uStride) + (((srcWidth / wf) / 2) - ((lineWidth / wf) / 2));
        uint8_t * const __restrict vPix = srcV + ((line / hf) * vStride) + (((srcWidth / wf) / 2) - ((lineWidth / wf) / 2));
        unsigned xx = 0;

        for (sc=0; sc<160; sc++)
        if (lines->l[sl][sc] > 0)
        {
          const struct Char * const __restrict c = characters[lines->l[sl][sc]];
          const uint8_t * const __restrict pixels = c->pixels + (ly * c->width);

          for (x=0, n=(unsigned)(c->width/srcAspect); x<n; x++)
          if (pixels[(unsigned)(x*srcAspect)] >= 128)
            uPix[(xx+x)/wf] = vPix[(xx+x)/wf] = (uint8_t)127;

          xx += (unsigned)((c->advance / srcAspect) + 0.5f);
        }
        else
          break;
      }

      line++;
    }
  }
}

void LXiStream_SSubtitleRenderNode_mixSubtitle32
 (uint32_t * __restrict srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
  const struct Lines * __restrict lines, const struct Char * const * __restrict characters)
{
  const unsigned lineHeight = characters[0]->height;
  unsigned line = srcHeight - (lineHeight * 4) - (lineHeight / 2);
  unsigned sl, sc, ly, x;

  for (sl=0; sl<4; sl++)
  {
    unsigned lineWidth = 0;
    for (sc=0; sc<160; sc++)
    if (lines->l[sl][sc] > 0)
      lineWidth += characters[lines->l[sl][sc]]->advance;

    if ((lineWidth + (lineHeight * 2)) < srcWidth)
    for (ly=0; ly<lineHeight; ly++)
    {
      uint8_t * const __restrict srcPix = (uint8_t *)(srcData + (line * srcStride) + ((srcWidth / 2) - (lineWidth / 2)));

      // Draw the shadow around the subtitles
      uint32_t * __restrict pix = (uint32_t *)srcPix;

      for (sc=0; sc<160; sc++)
      if (lines->l[sl][sc] > 0)
      {
        const struct Char * const __restrict c = characters[lines->l[sl][sc]];
        const uint8_t * const __restrict pixels = c->pixels + (ly * c->width);

        for (x=0; x<c->width; x++)
        if (pixels[x] < 128)
        {
          uint8_t * const __restrict p = (uint8_t *)(pix + x);
          p[0] = ((pixels[x] * 2) >= p[0]) ? 0 : (p[0] - (pixels[x] * 2));
          p[1] = ((pixels[x] * 2) >= p[1]) ? 0 : (p[1] - (pixels[x] * 2));
          p[2] = ((pixels[x] * 2) >= p[2]) ? 0 : (p[2] - (pixels[x] * 2));
          p[3] = ((pixels[x] * 2) >= p[3]) ? 0 : (p[3] - (pixels[x] * 2));
        }

        pix += c->advance;
      }
      else
        break;

      // And draw the subtitles themselves
      pix = (uint32_t *)srcPix;

      for (sc=0; sc<160; sc++)
      if (lines->l[sl][sc] > 0)
      {
        const struct Char * const __restrict c = characters[lines->l[sl][sc]];
        const uint8_t * const __restrict pixels = c->pixels + (ly * c->width);

        for (x=0; x<c->width; x++)
        if (pixels[x] > 3)
        {
          uint8_t * const __restrict p = (uint8_t *)(pix + x);
          p[0] = p[1] = p[2] = p[3] = (pixels[x] - 128) * 2;
        }

        pix += c->advance;
      }
      else
        break;

      line++;
    }
  }
}

void LXiStream_SSubtitleRenderNode_mixSubtitle32_stretch
 (uint32_t * __restrict srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
  float srcAspect,
  const struct Lines * __restrict lines, const struct Char * const * __restrict characters)
{
  const unsigned lineHeight = characters[0]->height;
  unsigned line = srcHeight - (lineHeight * 4) - (lineHeight / 2);
  unsigned sl, sc, ly, x, n;

  for (sl=0; sl<4; sl++)
  {
    unsigned lineWidth = 0;
    for (sc=0; sc<160; sc++)
    if (lines->l[sl][sc] > 0)
      lineWidth += (unsigned)((characters[lines->l[sl][sc]]->advance / srcAspect) + 0.5f);

    if ((lineWidth + (lineHeight * 2)) < srcWidth)
    for (ly=0; ly<lineHeight; ly++)
    {
      uint8_t * const __restrict srcPix = (uint8_t *)(srcData + (line * srcStride) + ((srcWidth / 2) - (lineWidth / 2)));

      // Draw the shadow around the subtitles
      uint32_t * __restrict pix = (uint32_t *)srcPix;

      for (sc=0; sc<160; sc++)
      if (lines->l[sl][sc] > 0)
      {
        const struct Char * const __restrict c = characters[lines->l[sl][sc]];
        const uint8_t * const __restrict pixels = c->pixels + (ly * c->width);

        for (x=0, n=(unsigned)(c->width/srcAspect); x<n; x++)
        {
          const float srcPos = (float)(x) * srcAspect;
          const unsigned srcX = (unsigned)srcPos;
          if ((pixels[srcX] < 128) || (pixels[srcX + 1] < 128))
          {
            const float sampleBweight = srcPos - (float)(srcX);
            const float sampleAweight = 1.0f - sampleBweight;
            const uint8_t sampleA = (pixels[srcX] < 128) ? (pixels[srcX] * 2) : 0;
            const uint8_t sampleB = (pixels[srcX + 1] < 128) ? (pixels[srcX + 1] * 2) : 0;

            const uint8_t sample =
                (uint8_t)((((float)sampleA) * sampleAweight) +
                          (((float)sampleB) * sampleBweight));

            uint8_t * const __restrict p = (uint8_t *)(pix + x);
            p[0] = (sample >= p[0]) ? 0 : (p[0] - sample);
            p[1] = (sample >= p[1]) ? 0 : (p[1] - sample);
            p[2] = (sample >= p[2]) ? 0 : (p[2] - sample);
            p[3] = (sample >= p[3]) ? 0 : (p[3] - sample);
          }
        }

        pix += (unsigned)((c->advance / srcAspect) + 0.5f);
      }
      else
        break;

      // And draw the subtitles themselves
      pix = (uint32_t *)srcPix;

      for (sc=0; sc<160; sc++)
      if (lines->l[sl][sc] > 0)
      {
        const struct Char * const __restrict c = characters[lines->l[sl][sc]];
        const uint8_t * const __restrict pixels = c->pixels + (ly * c->width);

        for (x=0, n=(unsigned)(c->width/srcAspect)-1; x<n; x++)
        {
          const float srcPos = (float)(x) * srcAspect;
          const unsigned srcX = (unsigned)srcPos;
          if ((pixels[srcX] >= 128) || (pixels[srcX + 1] >= 128))
          {
            const float sampleBweight = srcPos - (float)(srcX);
            const float sampleAweight = 1.0f - sampleBweight;
            const uint8_t sampleA = (pixels[srcX] >= 128) ? ((pixels[srcX] - 128) * 2) : 0;
            const uint8_t sampleB = (pixels[srcX + 1] >= 128) ? ((pixels[srcX + 1] - 128) * 2) : 0;

            uint8_t * const __restrict p = (uint8_t *)(pix + x);
            p[0] = p[1] = p[2] = p[3] =
                (uint8_t)((((float)sampleA) * sampleAweight) +
                          (((float)sampleB) * sampleBweight));
          }
        }

        pix += (unsigned)((c->advance / srcAspect) + 0.5f);
      }
      else
        break;

      line++;
    }
  }
}
