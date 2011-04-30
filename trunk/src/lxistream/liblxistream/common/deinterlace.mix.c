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
#include <assert.h>
#ifdef __SSE__
  #include <emmintrin.h>
#endif
#include <stdint.h>

void LXiStream_Common_Deinterlace_mixFields
 (const uint8_t * __restrict lineA, uint8_t * __restrict lineB, const uint8_t * __restrict lineC, unsigned numSamples)
{
  unsigned i;

#ifndef __SSE__
  for (i=0; i<numSamples; i++)
  {
    const uint8_t as = lineA[i] >> 1, bs = lineB[i] >> 1, cs = lineC[i] >> 1;

    int8_t dAB = ((int8_t)as) - ((int8_t)bs);
    int8_t dAC = ((int8_t)as) - ((int8_t)cs);

    dAB = dAB >= 0 ? dAB : (-dAB);
    dAC = dAC >= 0 ? dAC : (-dAC);

    if (dAB > (dAC + 8))
      lineB[i] = as + cs;
  }
#else
  const __m128i m = _mm_set1_epi8(0xFE);
  const __m128i c8 = _mm_set1_epi8(8);

  // Check alignment
  assert(((size_t)lineA & (size_t)15) == 0);
  assert(((size_t)lineB & (size_t)15) == 0);
  assert(((size_t)lineC & (size_t)15) == 0);
  assert(((size_t)numSamples & (size_t)15) == 0);

  for (i=0; i<numSamples; i+=(sizeof(__m128i)/sizeof(uint8_t)))
  {
    __m128i as = _mm_srli_epi32(_mm_and_si128(_mm_load_si128((__m128i *)(lineA + i)), m), 1);
    __m128i b  = _mm_load_si128((__m128i *)(lineB + i));
    __m128i bs = _mm_srli_epi32(_mm_and_si128(b, m), 1);
    __m128i cs = _mm_srli_epi32(_mm_and_si128(_mm_load_si128((__m128i *)(lineC + i)), m), 1);

    __m128i dAB = _mm_sub_epi8(as, bs);
    __m128i dAC = _mm_sub_epi8(as, cs);

    dAB = _mm_min_epu8(dAB, _mm_sub_epi8(_mm_setzero_si128(), dAB));
    dAC = _mm_min_epu8(dAC, _mm_sub_epi8(_mm_setzero_si128(), dAC));

    {
      __m128i blend = _mm_cmpgt_epi8(dAB, _mm_adds_epi8(dAC, c8));
      __m128i result = _mm_and_si128(_mm_adds_epu8(as, cs), blend);
      _mm_store_si128((__m128i *)(lineB + i), _mm_or_si128(result, _mm_andnot_si128(blend, b)));
    }
  }
#endif
}
