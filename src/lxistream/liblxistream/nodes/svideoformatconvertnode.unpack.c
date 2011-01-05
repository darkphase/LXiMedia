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

unsigned LXiStream_SVideoFormatConvertNode_convertYUYVtoYUV422P
 (const struct YUYVPixel * restrict packed, unsigned numLines, size_t lineStride, uint8_t * restrict y, uint8_t * restrict u, uint8_t * restrict v)
{
  // Check alignment
  assert(((size_t)packed & (size_t)15) == 0);

  size_t i;

  #ifndef __SSE__
    for (i = (lineStride * numLines) / sizeof(struct YUYVPixel); i; i--)
    {
      *(y++) = packed->y0;
      *(u++) = packed->u;
      *(y++) = packed->y1;
      *(v++) = packed->v;
      packed++;
    }
  #else
    for (i = (lineStride * numLines) / (sizeof(__m128i) * 4); i; i--)
    {
      // i1 = VYUY.VYUY.VYUY.VYUY ... i4 = VYUY.VYUY.VYUY.VYUY
      __m128i i1 = _mm_load_si128((__m128i *)(packed));
      __m128i i2 = _mm_load_si128((__m128i *)(packed + 4));
      __m128i i3 = _mm_load_si128((__m128i *)(packed + 8));
      __m128i i4 = _mm_load_si128((__m128i *)(packed + 12));
      packed += sizeof(__m128i);

      // Grab the Y values
      _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(_mm_slli_epi16(i1, 8), 8),
                                                      _mm_srli_epi16(_mm_slli_epi16(i2, 8), 8)));
      y += sizeof(__m128i);
      _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(_mm_slli_epi16(i3, 8), 8),
                                                      _mm_srli_epi16(_mm_slli_epi16(i4, 8), 8)));
      y += sizeof(__m128i);

      // Grab the U values
      _mm_storeu_si128((__m128i *)u, _mm_packus_epi16( _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 16), 24),
                                                                       _mm_srli_epi32(_mm_slli_epi32(i2, 16), 24)),
                                                       _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 16), 24),
                                                                       _mm_srli_epi32(_mm_slli_epi32(i4, 16), 24))));
      u += sizeof(__m128i);

      // Grab the V values
      _mm_storeu_si128((__m128i *)v, _mm_packus_epi16( _mm_packs_epi32(_mm_srli_epi32(i1, 24),
                                                                       _mm_srli_epi32(i2, 24)),
                                                       _mm_packs_epi32(_mm_srli_epi32(i3, 24),
                                                                       _mm_srli_epi32(i4, 24))));
      v += sizeof(__m128i);
    }
  #endif
}


unsigned LXiStream_SVideoFormatConvertNode_convertUYVYtoYUV422P
 (const struct UYVYPixel * restrict packed, unsigned numLines, size_t lineStride, uint8_t * restrict y, uint8_t * restrict u, uint8_t * restrict v)
{
  // Check alignment
  assert(((size_t)packed & (size_t)15) == 0);

  size_t i;

  #ifndef __SSE__
    for (i = (lineStride * numLines) / sizeof(struct UYVYPixel); i; i--)
    {
      *(y++) = packed->y0;
      *(u++) = packed->u;
      *(y++) = packed->y1;
      *(v++) = packed->v;
      packed++;
    }
  #else
    for (i = (lineStride * numLines) / (sizeof(__m128i) * 4); i; i--)
    {
      // i1 = YVYU.YVYU.YVYU.YVYU ... i4 = YVYU.YVYU.YVYU.YVYU
      __m128i i1 = _mm_load_si128((__m128i *)(packed));
      __m128i i2 = _mm_load_si128((__m128i *)(packed + 4));
      __m128i i3 = _mm_load_si128((__m128i *)(packed + 8));
      __m128i i4 = _mm_load_si128((__m128i *)(packed + 12));
      packed += sizeof(__m128i);

      // Grab the Y values
      _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(i1, 8),
                                                      _mm_srli_epi16(i2, 8)));
      y += sizeof(__m128i);
      _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(i3, 8),
                                                      _mm_srli_epi16(i4, 8)));
      y += sizeof(__m128i);

      // Grab the U values
      _mm_storeu_si128((__m128i *)u, _mm_packus_epi16( _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 24), 24),
                                                                       _mm_srli_epi32(_mm_slli_epi32(i2, 24), 24)),
                                                       _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 24), 24),
                                                                       _mm_srli_epi32(_mm_slli_epi32(i4, 24), 24))));
      u += sizeof(__m128i);

      // Grab the V values
      _mm_storeu_si128((__m128i *)v, _mm_packus_epi16( _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 8), 24),
                                                                       _mm_srli_epi32(_mm_slli_epi32(i2, 8), 24)),
                                                       _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 8), 24),
                                                                       _mm_srli_epi32(_mm_slli_epi32(i4, 8), 24))));
      v += sizeof(__m128i);
    }
  #endif
}


unsigned LXiStream_SVideoFormatConvertNode_convertYUYVtoYUV420P
 (const struct YUYVPixel * restrict packed, unsigned numLines, size_t lineStride, uint8_t * restrict y, uint8_t * restrict u, uint8_t * restrict v)
{
  // Check alignment
  assert(((size_t)packed & (size_t)15) == 0);

  int l;
  size_t i;

  #ifndef __SSE__
    for (l=(int)numLines; l>0; l-=2)
    {
      for (i = lineStride / sizeof(struct YUYVPixel); i; i--)
      {
        *(y++) = packed->y0;
        *(u++) = packed->u;
        *(y++) = packed->y1;
        *(v++) = packed->v;
        packed++;
      }

      for (i = lineStride / sizeof(struct YUYVPixel); i; i--)
      {
        *(y++) = packed->y0;
        *(y++) = packed->y1;
        packed++;
      }
    }
  #else
    for (l=0; l<(int)numLines; l+=2)
    {
      for (i = lineStride / (sizeof(__m128i) * 4); i; i--)
      {
        // i1 = VYUY.VYUY.VYUY.VYUY ... i4 = VYUY.VYUY.VYUY.VYUY
        __m128i i1 = _mm_load_si128((__m128i *)(packed));
        __m128i i2 = _mm_load_si128((__m128i *)(packed + 4));
        __m128i i3 = _mm_load_si128((__m128i *)(packed + 8));
        __m128i i4 = _mm_load_si128((__m128i *)(packed + 12));
        packed += sizeof(__m128i);

        // Grab the Y values
        _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(_mm_slli_epi16(i1, 8), 8),
                                                        _mm_srli_epi16(_mm_slli_epi16(i2, 8), 8)));
        y += sizeof(__m128i);
        _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(_mm_slli_epi16(i3, 8), 8),
                                                        _mm_srli_epi16(_mm_slli_epi16(i4, 8), 8)));
        y += sizeof(__m128i);

        // Grab the U values
        _mm_storeu_si128((__m128i *)u, _mm_packus_epi16( _mm_packs_epi32( _mm_srli_epi32(_mm_slli_epi32(i1, 16), 24),
                                                                          _mm_srli_epi32(_mm_slli_epi32(i2, 16), 24)),
                                                         _mm_packs_epi32( _mm_srli_epi32(_mm_slli_epi32(i3, 16), 24),
                                                                          _mm_srli_epi32(_mm_slli_epi32(i4, 16), 24))));
        u += sizeof(__m128i);

        // Grab the V values
        _mm_storeu_si128((__m128i *)v, _mm_packus_epi16( _mm_packs_epi32( _mm_srli_epi32(i1, 24),
                                                                          _mm_srli_epi32(i2, 24)),
                                                         _mm_packs_epi32( _mm_srli_epi32(i3, 24),
                                                                          _mm_srli_epi32(i4, 24))));
        v += sizeof(__m128i);
      }

      for (i = (lineStride % (sizeof(__m128i) * 4)) / 4; i; i--)
      {
        *(y++) = packed->y0;
        *(u++) = packed->u;
        *(y++) = packed->y1;
        *(v++) = packed->v;
        packed++;
      }

      for (i = lineStride / (sizeof(__m128i) * 4); i; i--)
      {
        // i1 = VYUY.VYUY.VYUY.VYUY ... i4 = VYUY.VYUY.VYUY.VYUY
        __m128i i1 = _mm_load_si128((__m128i *)(packed));
        __m128i i2 = _mm_load_si128((__m128i *)(packed + 4));
        __m128i i3 = _mm_load_si128((__m128i *)(packed + 8));
        __m128i i4 = _mm_load_si128((__m128i *)(packed + 12));
        packed += sizeof(__m128i);

        // Grab the Y values
        _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(_mm_slli_epi16(i1, 8), 8),
                                                        _mm_srli_epi16(_mm_slli_epi16(i2, 8), 8)));
        y += sizeof(__m128i);
        _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(_mm_slli_epi16(i3, 8), 8),
                                                        _mm_srli_epi16(_mm_slli_epi16(i4, 8), 8)));
        y += sizeof(__m128i);

      }

      for (i = (lineStride % (sizeof(__m128i) * 4)) / 4; i; i--)
      {
        *(y++) = packed->y0;
        *(y++) = packed->y1;
        packed++;
      }
    }
  #endif
}


unsigned LXiStream_SVideoFormatConvertNode_convertUYVYtoYUV420P
 (const struct UYVYPixel * restrict packed, unsigned numLines, size_t lineStride, uint8_t * restrict y, uint8_t * restrict u, uint8_t * restrict v)
{
  // Check alignment
  assert(((size_t)packed & (size_t)15) == 0);
  assert(((size_t)y & (size_t)15) == 0);
  assert(((size_t)u & (size_t)15) == 0);
  assert(((size_t)v & (size_t)15) == 0);

  int l;
  size_t i;

  #ifndef __SSE__
    for (l=(int)numLines; l>0; l-=2)
    {
      for (i = lineStride / sizeof(struct UYVYPixel); i; i--)
      {
        *(y++) = packed->y0;
        *(u++) = packed->u;
        *(y++) = packed->y1;
        *(v++) = packed->v;
        packed++;
      }

      for (i = lineStride / sizeof(struct UYVYPixel); i; i--)
      {
        *(y++) = packed->y0;
        *(y++) = packed->y1;
        packed++;
      }
    }
  #else
    for (l=0; l<(int)numLines; l+=2)
    {
      for (i = lineStride / (sizeof(__m128i) * 4); i; i--)
      {
        // i1 = YVYU.YVYU.YVYU.YVYU ... i4 = YVYU.YVYU.YVYU.YVYU
        __m128i i1 = _mm_load_si128((__m128i *)(packed));
        __m128i i2 = _mm_load_si128((__m128i *)(packed + 4));
        __m128i i3 = _mm_load_si128((__m128i *)(packed + 8));
        __m128i i4 = _mm_load_si128((__m128i *)(packed + 12));
        packed += sizeof(__m128i);

        // Grab the Y values
        _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(i1, 8),
                                                        _mm_srli_epi16(i2, 8)));
        y += sizeof(__m128i);
        _mm_store_si128((__m128i *)y, _mm_packus_epi16( _mm_srli_epi16(i3, 8),
                                                        _mm_srli_epi16(i4, 8)));
        y += sizeof(__m128i);

        // Grab the U values
        _mm_storeu_si128((__m128i *)u, _mm_packus_epi16( _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 24), 24),
                                                                         _mm_srli_epi32(_mm_slli_epi32(i2, 24), 24)),
                                                         _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 24), 24),
                                                                         _mm_srli_epi32(_mm_slli_epi32(i4, 24), 24))));
        u += sizeof(__m128i);

        // Grab the V values
        _mm_storeu_si128((__m128i *)v, _mm_packus_epi16( _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i1, 8), 24),
                                                                         _mm_srli_epi32(_mm_slli_epi32(i2, 8), 24)),
                                                         _mm_packs_epi32(_mm_srli_epi32(_mm_slli_epi32(i3, 8), 24),
                                                                         _mm_srli_epi32(_mm_slli_epi32(i4, 8), 24))));
        v += sizeof(__m128i);
      }

      for (i = (lineStride % (sizeof(__m128i) * 4)) / 4; i; i--)
      {
        *(y++) = packed->y0;
        *(u++) = packed->u;
        *(y++) = packed->y1;
        *(v++) = packed->v;
        packed++;
      }

      for (i = lineStride / (sizeof(__m128i) * 4); i; i--)
      {
        // i1 = YVYU.YVYU.YVYU.YVYU ... i4 = YVYU.YVYU.YVYU.YVYU
        __m128i i1 = _mm_load_si128((__m128i *)(packed));
        __m128i i2 = _mm_load_si128((__m128i *)(packed + 4));
        __m128i i3 = _mm_load_si128((__m128i *)(packed + 8));
        __m128i i4 = _mm_load_si128((__m128i *)(packed + 12));
        packed += sizeof(__m128i);

        // Grab the Y values
        _mm_store_si128((__m128i *)y, _mm_packus_epi16(_mm_srli_epi16(i1, 8),
                                                       _mm_srli_epi16(i2, 8)));
        y += sizeof(__m128i);
        _mm_store_si128((__m128i *)y, _mm_packus_epi16(_mm_srli_epi16(i3, 8),
                                                       _mm_srli_epi16(i4, 8)));
        y += sizeof(__m128i);

      }

      for (i = (lineStride % (sizeof(__m128i) * 4)) / 4; i; i--)
      {
        *(y++) = packed->y0;
        *(y++) = packed->y1;
        packed++;
      }
    }
  #endif
}
