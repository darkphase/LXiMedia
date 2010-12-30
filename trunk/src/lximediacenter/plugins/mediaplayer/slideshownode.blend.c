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

void LXiMediaCenter_SlideShowNode_blendImages
 (uint8_t * restrict dstData, const uint8_t * restrict srcDataA, const uint8_t * restrict srcDataB, unsigned numPixels, int factor)
{
  #ifndef __SSE__
    const uint16_t f = factor, af = 256 - factor;

    for (unsigned i=0; i<numPixels; i++)
      dstData[i] = (uint8_t)(((((uint16_t)srcDataA[i]) * af) + (((uint16_t)srcDataB[i]) * f)) >> 8);
  #else
    // Check alignment
    assert(((size_t)dstData & (size_t)15) == 0);
    assert(((size_t)srcDataA & (size_t)15) == 0);
    assert(((size_t)srcDataB & (size_t)15) == 0);
    assert(((size_t)numPixels & (size_t)15) == 0);

    const __m128i z  = _mm_setzero_si128();
    const __m128i f  = _mm_set1_epi16(factor);
    const __m128i af = _mm_set1_epi16(256 - factor);

    for (unsigned i=0; i<numPixels; i+=(sizeof(__m128i)/sizeof(uint8_t)))
    {
      __m128i a = _mm_load_si128((__m128i *)(srcDataA + i));
      __m128i b = _mm_load_si128((__m128i *)(srcDataB + i));

      __m128i rl = _mm_add_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(a, z), af),
                                 _mm_mullo_epi16(_mm_unpacklo_epi8(b, z), f));
      __m128i rh = _mm_add_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(a, z), af),
                                 _mm_mullo_epi16(_mm_unpackhi_epi8(b, z), f));

      _mm_store_si128((__m128i *)(dstData + i),
                      _mm_packus_epi16(_mm_srli_epi16(rl, 8), _mm_srli_epi16(rh, 8)));
    }
  #endif
}
