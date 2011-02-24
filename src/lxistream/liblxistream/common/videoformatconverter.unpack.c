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
#include <assert.h>
#ifdef __SSE__
  #include <xmmintrin.h>
#endif
#include "spixels.h"

unsigned LXiStream_Common_VideoFormatConverter_convertYUYVtoYUV422P
    (const YUYVPixel * restrict packed, size_t lineStride, unsigned width, unsigned numLines,
     uint8_t * restrict y, size_t yStride, uint8_t * restrict u, size_t uStride,
     uint8_t * restrict v, size_t vStride)
{
  // Check alignment
  assert(((size_t)packed & (size_t)15) == 0);
  assert((lineStride & 15) == 0);
  assert(((size_t)y & (size_t)15) == 0);
  assert((yStride & 15) == 0);
  assert(((size_t)u & (size_t)15) == 0);
  assert((uStride & 15) == 0);
  assert(((size_t)v & (size_t)15) == 0);
  assert((vStride & 15) == 0);

  for (unsigned l=0; l<numLines; l++)
  {
    const YUYVPixel * restrict packedl = ((const uint8_t *)packed) + (lineStride * l);
    uint8_t * restrict yl = y + (yStride * l);
    uint8_t * restrict ul = u + (uStride * l);
    uint8_t * restrict vl = v + (vStride * l);
    unsigned i = width / 2;

#ifdef __SSE__
    while (i >= ((sizeof(__m128i) * 4) / sizeof(YUYVPixel)))
    {
      i -= (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // i1 = VYUY.VYUY.VYUY.VYUY ... i4 = VYUY.VYUY.VYUY.VYUY
      __m128i i1 = _mm_load_si128((__m128i *)(packedl));
      __m128i i2 = _mm_load_si128((__m128i *)(packedl + 4));
      __m128i i3 = _mm_load_si128((__m128i *)(packedl + 8));
      __m128i i4 = _mm_load_si128((__m128i *)(packedl + 12));
      packedl += (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // Grab the Y values
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(_mm_slli_epi16(i1, 8), 8),
                                                      _mm_srli_epi16(_mm_slli_epi16(i2, 8), 8)));
      yl += sizeof(__m128i);
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(_mm_slli_epi16(i3, 8), 8),
                                                      _mm_srli_epi16(_mm_slli_epi16(i4, 8), 8)));
      yl += sizeof(__m128i);

      // Grab the U values
      _mm_store_si128((__m128i *)ul, _mm_packus_epi16(_mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 16), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i2, 16), 24)),
                                                      _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 16), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i4, 16), 24))));
      ul += sizeof(__m128i);

      // Grab the V values
      _mm_store_si128((__m128i *)vl, _mm_packus_epi16(_mm_packs_epi32(_mm_srli_epi32(i1, 24),
                                                                      _mm_srli_epi32(i2, 24)),
                                                      _mm_packs_epi32(_mm_srli_epi32(i3, 24),
                                                                      _mm_srli_epi32(i4, 24))));
      vl += sizeof(__m128i);
    }
#endif

    for (; i; i--)
    {
      *(yl++) = packedl->y0;
      *(ul++) = packedl->u;
      *(yl++) = packedl->y1;
      *(vl++) = packedl->v;
      packedl++;
    }
  }
}


unsigned LXiStream_Common_VideoFormatConverter_convertUYVYtoYUV422P
    (const YUYVPixel * restrict packed, size_t lineStride, unsigned width, unsigned numLines,
     uint8_t * restrict y, size_t yStride, uint8_t * restrict u, size_t uStride,
     uint8_t * restrict v, size_t vStride)
{
  // Check alignment
  assert(((size_t)packed & (size_t)15) == 0);
  assert((lineStride & 15) == 0);
  assert(((size_t)y & (size_t)15) == 0);
  assert((yStride & 15) == 0);
  assert(((size_t)u & (size_t)15) == 0);
  assert((uStride & 15) == 0);
  assert(((size_t)v & (size_t)15) == 0);
  assert((vStride & 15) == 0);

  for (unsigned l=0; l<numLines; l++)
  {
    const YUYVPixel * restrict packedl = ((const uint8_t *)packed) + (lineStride * l);
    uint8_t * restrict yl = y + (yStride * l);
    uint8_t * restrict ul = u + (uStride * l);
    uint8_t * restrict vl = v + (vStride * l);
    unsigned i = width / 2;

#ifdef __SSE__
    while (i >= ((sizeof(__m128i) * 4) / sizeof(YUYVPixel)))
    {
      i -= (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // i1 = YVYU.YVYU.YVYU.YVYU ... i4 = YVYU.YVYU.YVYU.YVYU
      __m128i i1 = _mm_load_si128((__m128i *)(packedl));
      __m128i i2 = _mm_load_si128((__m128i *)(packedl + 4));
      __m128i i3 = _mm_load_si128((__m128i *)(packedl + 8));
      __m128i i4 = _mm_load_si128((__m128i *)(packedl + 12));
      packedl += (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // Grab the Y values
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(i1, 8),
                                                      _mm_srli_epi16(i2, 8)));
      yl += sizeof(__m128i);
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(i3, 8),
                                                      _mm_srli_epi16(i4, 8)));
      yl += sizeof(__m128i);

      // Grab the U values
      _mm_store_si128((__m128i *)ul, _mm_packus_epi16(_mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 24), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i2, 24), 24)),
                                                      _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 24), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i4, 24), 24))));
      ul += sizeof(__m128i);

      // Grab the V values
      _mm_store_si128((__m128i *)vl, _mm_packus_epi16(_mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 8), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i2, 8), 24)),
                                                      _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 8), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i4, 8), 24))));
      vl += sizeof(__m128i);
    }
#endif

    for (; i; i--)
    {
      *(yl++) = packedl->y0;
      *(ul++) = packedl->u;
      *(yl++) = packedl->y1;
      *(vl++) = packedl->v;
      packedl++;
    }
  }
}


unsigned LXiStream_Common_VideoFormatConverter_convertYUYVtoYUV420P
    (const YUYVPixel * restrict packed, size_t lineStride, unsigned width, unsigned numLines,
     uint8_t * restrict y, size_t yStride, uint8_t * restrict u, size_t uStride,
     uint8_t * restrict v, size_t vStride)
{
  // Check alignment
  assert(((size_t)packed & (size_t)15) == 0);
  assert((lineStride & 15) == 0);
  assert(((size_t)y & (size_t)15) == 0);
  assert((yStride & 15) == 0);
  assert(((size_t)u & (size_t)15) == 0);
  assert((uStride & 15) == 0);
  assert(((size_t)v & (size_t)15) == 0);
  assert((vStride & 15) == 0);

  for (unsigned l=0, n=numLines/2; l<n; l++)
  {
    const YUYVPixel * restrict packedl = ((const uint8_t *)packed) + (lineStride * l * 2);
    uint8_t * restrict yl = y + (yStride * l * 2);
    uint8_t * restrict ul = u + (uStride * l);
    uint8_t * restrict vl = v + (vStride * l);
    unsigned i = width / 2;

#ifdef __SSE__
    while (i >= ((sizeof(__m128i) * 4) / sizeof(YUYVPixel)))
    {
      i -= (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // i1 = VYUY.VYUY.VYUY.VYUY ... i4 = VYUY.VYUY.VYUY.VYUY
      __m128i i1 = _mm_load_si128((__m128i *)(packedl));
      __m128i i2 = _mm_load_si128((__m128i *)(packedl + 4));
      __m128i i3 = _mm_load_si128((__m128i *)(packedl + 8));
      __m128i i4 = _mm_load_si128((__m128i *)(packedl + 12));
      packedl += (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // Grab the Y values
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(_mm_slli_epi16(i1, 8), 8),
                                                      _mm_srli_epi16(_mm_slli_epi16(i2, 8), 8)));
      yl += sizeof(__m128i);
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(_mm_slli_epi16(i3, 8), 8),
                                                      _mm_srli_epi16(_mm_slli_epi16(i4, 8), 8)));
      yl += sizeof(__m128i);

      // Grab the U values
      _mm_store_si128((__m128i *)ul, _mm_packus_epi16(_mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 16), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i2, 16), 24)),
                                                      _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 16), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i4, 16), 24))));
      ul += sizeof(__m128i);

      // Grab the V values
      _mm_store_si128((__m128i *)vl, _mm_packus_epi16(_mm_packs_epi32(_mm_srli_epi32(i1, 24),
                                                                      _mm_srli_epi32(i2, 24)),
                                                      _mm_packs_epi32(_mm_srli_epi32(i3, 24),
                                                                      _mm_srli_epi32(i4, 24))));
      vl += sizeof(__m128i);
    }
#endif

    for (; i; i--)
    {
      *(yl++) = packedl->y0;
      *(ul++) = packedl->u;
      *(yl++) = packedl->y1;
      *(vl++) = packedl->v;
      packedl++;
    }

    packedl = ((const uint8_t *)packed) + (lineStride * ((l * 2) + 1));
    yl = y + (yStride * ((l * 2) + 1));
    i = lineStride / sizeof(YUYVPixel);

#ifdef __SSE__
    while (i >= ((sizeof(__m128i) * 4) / sizeof(YUYVPixel)))
    {
      i -= (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // i1 = VYUY.VYUY.VYUY.VYUY ... i4 = VYUY.VYUY.VYUY.VYUY
      __m128i i1 = _mm_load_si128((__m128i *)(packedl));
      __m128i i2 = _mm_load_si128((__m128i *)(packedl + 4));
      __m128i i3 = _mm_load_si128((__m128i *)(packedl + 8));
      __m128i i4 = _mm_load_si128((__m128i *)(packedl + 12));
      packedl += (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // Grab the Y values
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(_mm_slli_epi16(i1, 8), 8),
                                                      _mm_srli_epi16(_mm_slli_epi16(i2, 8), 8)));
      yl += sizeof(__m128i);
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(_mm_slli_epi16(i3, 8), 8),
                                                      _mm_srli_epi16(_mm_slli_epi16(i4, 8), 8)));
      yl += sizeof(__m128i);
    }
#endif

    for (; i; i--)
    {
      *(yl++) = packedl->y0;
      *(yl++) = packedl->y1;
      packedl++;
    }
  }
}


unsigned LXiStream_Common_VideoFormatConverter_convertUYVYtoYUV420P
    (const YUYVPixel * restrict packed, size_t lineStride, unsigned width, unsigned numLines,
     uint8_t * restrict y, size_t yStride, uint8_t * restrict u, size_t uStride,
     uint8_t * restrict v, size_t vStride)
{
  // Check alignment
  assert(((size_t)packed & (size_t)15) == 0);
  assert((lineStride & 15) == 0);
  assert(((size_t)y & (size_t)15) == 0);
  assert((yStride & 15) == 0);
  assert(((size_t)u & (size_t)15) == 0);
  assert((uStride & 15) == 0);
  assert(((size_t)v & (size_t)15) == 0);
  assert((vStride & 15) == 0);

  for (unsigned l=0, n=numLines/2; l<n; l++)
  {
    const YUYVPixel * restrict packedl = ((const uint8_t *)packed) + (lineStride * l * 2);
    uint8_t * restrict yl = y + (yStride * l * 2);
    uint8_t * restrict ul = u + (uStride * l);
    uint8_t * restrict vl = v + (vStride * l);
    unsigned i = width / 2;

#ifdef __SSE__
    while (i >= ((sizeof(__m128i) * 4) / sizeof(YUYVPixel)))
    {
      i -= (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // i1 = YVYU.YVYU.YVYU.YVYU ... i4 = YVYU.YVYU.YVYU.YVYU
      __m128i i1 = _mm_load_si128((__m128i *)(packedl));
      __m128i i2 = _mm_load_si128((__m128i *)(packedl + 4));
      __m128i i3 = _mm_load_si128((__m128i *)(packedl + 8));
      __m128i i4 = _mm_load_si128((__m128i *)(packedl + 12));
      packedl += (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // Grab the Y values
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(i1, 8),
                                                      _mm_srli_epi16(i2, 8)));
      yl += sizeof(__m128i);
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(i3, 8),
                                                      _mm_srli_epi16(i4, 8)));
      yl += sizeof(__m128i);

      // Grab the U values
      _mm_store_si128((__m128i *)ul, _mm_packus_epi16(_mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 24), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i2, 24), 24)),
                                                      _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 24), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i4, 24), 24))));
      ul += sizeof(__m128i);

      // Grab the V values
      _mm_store_si128((__m128i *)vl, _mm_packus_epi16(_mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 8), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i2, 8), 24)),
                                                      _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 8), 24),
                                                                      _mm_srli_epi32(_mm_slli_epi32(i4, 8), 24))));
      vl += sizeof(__m128i);
    }
#endif

    for (; i; i--)
    {
      *(yl++) = packedl->y0;
      *(ul++) = packedl->u;
      *(yl++) = packedl->y1;
      *(vl++) = packedl->v;
      packedl++;
    }

    packedl = ((const uint8_t *)packed) + (lineStride * ((l * 2) + 1));
    yl = y + (yStride * ((l * 2) + 1));
    i = lineStride / sizeof(YUYVPixel);

#ifdef __SSE__
    while (i >= ((sizeof(__m128i) * 4) / sizeof(YUYVPixel)))
    {
      i -= (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // i1 = YVYU.YVYU.YVYU.YVYU ... i4 = YVYU.YVYU.YVYU.YVYU
      __m128i i1 = _mm_load_si128((__m128i *)(packedl));
      __m128i i2 = _mm_load_si128((__m128i *)(packedl + 4));
      __m128i i3 = _mm_load_si128((__m128i *)(packedl + 8));
      __m128i i4 = _mm_load_si128((__m128i *)(packedl + 12));
      packedl += (sizeof(__m128i) * 4) / sizeof(YUYVPixel);

      // Grab the Y values
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(i1, 8),
                                                      _mm_srli_epi16(i2, 8)));
      yl += sizeof(__m128i);
      _mm_store_si128((__m128i *)yl, _mm_packus_epi16(_mm_srli_epi16(i3, 8),
                                                      _mm_srli_epi16(i4, 8)));
      yl += sizeof(__m128i);
    }
#endif

    for (; i; i--)
    {
      *(yl++) = packedl->y0;
      *(yl++) = packedl->y1;
      packedl++;
    }
  }
}
