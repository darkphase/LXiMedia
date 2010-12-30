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
#ifdef __SSE__
  #include <xmmintrin.h>
#endif

void LXiStream_STextureBuffer_convertRGBtoBGR
 (uint32_t * restrict dst, const uint32_t * restrict src, unsigned numPixels)
{
  unsigned i;

#ifdef __SSE__
  // Check alignment
  if ((((size_t)dst & (size_t)15) != 0) || (((size_t)src & (size_t)15) != 0) ||
      ((numPixels % (sizeof(__m128i) / sizeof(uint32_t))) != 0))
  {
#endif
    for (i=0; i<numPixels; i++)
    {
      *dst++ =  (*src & 0xFF00FF00) |
               ((*src & 0x000000FF) << 16) |
               ((*src & 0x00FF0000) >> 16);
      src++;
    }
#ifdef __SSE__
  }
  else
  {
    __m128i mask1 = _mm_set_epi32(0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00);
    __m128i mask2 = _mm_set_epi32(0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF);
    __m128i mask3 = _mm_set_epi32(0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000);

    for (i=0; i<numPixels; i+=sizeof(__m128i)/sizeof(uint32_t))
    {
      __m128i px = _mm_load_si128((__m128i *)src);
      src += sizeof(__m128i) / sizeof(*src);

      __m128i rs = _mm_and_si128(px, mask1);
      rs = _mm_or_si128(rs, _mm_slli_epi32(_mm_and_si128(px, mask2), 16));
      rs = _mm_or_si128(rs, _mm_srli_epi32(_mm_and_si128(px, mask3), 16));

      _mm_store_si128((__m128i *)dst, rs);
      dst += sizeof(__m128i) / sizeof(*dst);
    }
  }
#endif
}
