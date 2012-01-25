/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#ifdef __SSE__
  #include <xmmintrin.h>
#endif

void LXiStream_AnalogVideoFilter_mixLine
 (uint8_t * restrict dst, const uint8_t * restrict src, uint32_t rmask, unsigned numBytes)
{
  static const uint8_t rm0 = 0, rm1 = 2, rm2 = 4;

  // Check alignment
  assert(((size_t)src & (size_t)15) == 0);
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)numBytes & (size_t)15) == 0);

#ifndef __SSE__
  const uint8_t rm[4] = { rmask & 0xFF, (rmask & 0xFF00) >> 8, (rmask & 0xFF0000) >> 16, (rmask & 0xFF000000) >> 24 };
  const uint8_t crm0[4] = { rm0 + rm[0], rm0 + rm[1], rm0 + rm[2], rm0 + rm[3] };
  const uint8_t crm1[4] = { rm1 + rm[0], rm1 + rm[1], rm1 + rm[2], rm1 + rm[3] };
  const uint8_t crm2[4] = { rm2 + rm[0], rm2 + rm[1], rm2 + rm[2], rm2 + rm[3] };

  for (unsigned i=0; i<numBytes; i+=4)
  for (unsigned j=0; j<4; j++)
  {
    const unsigned p = i + j;
    const uint8_t as = src[p] >> 1, bs = dst[p] >> 1;

    int8_t dAB = ((int8_t)as) - ((int8_t)bs);
    dAB = dAB >= 0 ? dAB : (-dAB);

    if (dAB <= crm0[j])
      dst[p] = ((as >> 1) + (bs + (bs >> 1)));
    else if (dAB <= crm1[j])
      dst[p] = as + bs;
    else if (dAB <= crm2[j])
      dst[p] = (as + (as >> 1)) + (bs >> 1);
    else
      dst[p] = src[p];
  }
#else
  const __m128i m = _mm_set1_epi8(0xFE);
  const __m128i rm = _mm_set1_epi32(rmask);
  const __m128i crm0 = _mm_adds_epu8(_mm_set1_epi8(rm0), rm);
  const __m128i crm1 = _mm_adds_epu8(_mm_set1_epi8(rm1), rm);
  const __m128i crm2 = _mm_adds_epu8(_mm_set1_epi8(rm2), rm);

  for (unsigned i=0; i<numBytes; i+=(sizeof(__m128i)/sizeof(uint8_t)))
  {
    __m128i a  = _mm_load_si128((__m128i *)(src + i));
    __m128i as = _mm_srli_epi32(_mm_and_si128(a, m), 1);
    __m128i bs = _mm_srli_epi32(_mm_and_si128(_mm_load_si128((__m128i *)(dst + i)), m), 1);

    __m128i dAB = _mm_sub_epi8(as, bs);
    dAB = _mm_min_epu8(dAB, _mm_sub_epi8(_mm_setzero_si128(), dAB));

    __m128i ass = _mm_srli_epi32(_mm_and_si128(as, m), 1);
    __m128i bss = _mm_srli_epi32(_mm_and_si128(bs, m), 1);

    __m128i result = _mm_adds_epu8(ass, _mm_adds_epu8(bs, bss));
    __m128i blend0 = _mm_cmpgt_epi8(dAB, crm0);
    __m128i result0 = _mm_or_si128(_mm_andnot_si128(blend0, result), _mm_and_si128(_mm_adds_epu8(as, bs), blend0));
    __m128i blend1 = _mm_cmpgt_epi8(dAB, crm1);
    __m128i result1 = _mm_or_si128(_mm_andnot_si128(blend1, result0), _mm_and_si128(_mm_adds_epu8(_mm_adds_epu8(as, ass), bss), blend1));
    __m128i blend2 = _mm_cmpgt_epi8(dAB, crm2);
    __m128i result2 = _mm_or_si128(_mm_andnot_si128(blend2, result1), _mm_and_si128(a, blend2));

    _mm_store_si128((__m128i *)(dst + i), result2);
  }
#endif
}

#ifndef __SSE__
inline uint16_t conbrg_c(uint16_t t, uint16_t mul)
{
  t = t < 256 ? t : 0;
  t = (t * mul) >> 7;
  t = t < 256 ? t : 255;
  //return t;

  uint16_t ta = 255 - t;
  ta = 255 - ((ta * ta) >> 8);

  return (ta + t) >> 1;
}
#else
inline __m128i conbrg_sse(__m128i am, __m128i cmin, __m128i cmul, __m128i c0, __m128i c255)
{
  __m128i t = _mm_sub_epi16(am, cmin);
  t = _mm_max_epi16(t, c0);
  t = _mm_srli_epi16(_mm_mullo_epi16(t, cmul), 7);
  t = _mm_min_epi16(t, c255);
  //return t;

  __m128i ta = _mm_sub_epi16(c255, t);
  ta = _mm_sub_epi16(c255, _mm_srli_epi16(_mm_mullo_epi16(ta, ta), 8));

  return _mm_srli_epi16(_mm_add_epi16(ta, t), 1);
}
#endif

void LXiStream_AnalogVideoFilter_conbrg
 (uint8_t * restrict dst, const uint8_t * restrict src, uint8_t min, uint8_t max, unsigned numBytes)
{
  // Check alignment
  assert(((size_t)src & (size_t)15) == 0);
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)numBytes & (size_t)15) == 0);

#ifndef __SSE__
  const uint16_t mul = 32767 / (max - min);
  for (unsigned i=0; i<numBytes; i++)
    dst[i] = conbrg_c(((uint16_t)src[i]) - ((uint16_t)min), mul);
#else
  const __m128i cmin = _mm_set1_epi16(min);
  const __m128i cmul = _mm_set1_epi16(32767 / (max - min));
  const __m128i c0 = _mm_setzero_si128();
  const __m128i c255 = _mm_set1_epi16(255);

  for (unsigned i=0; i<numBytes; i+=(sizeof(__m128i)/sizeof(uint8_t)))
  {
    __m128i a  = _mm_load_si128((__m128i *)(src + i));
    __m128i a0 = conbrg_sse(_mm_unpacklo_epi8(a, c0), cmin, cmul, c0, c255);
    __m128i a1 = conbrg_sse(_mm_unpackhi_epi8(a, c0), cmin, cmul, c0, c255);

    _mm_store_si128((__m128i *)(dst + i), _mm_packus_epi16(a0, a1));
  }
#endif
}

void LXiStream_AnalogVideoFilter_conbrg_YUYV
 (uint8_t * restrict dst, const uint8_t * restrict src, uint8_t min, uint8_t max, unsigned numBytes)
{
  // Check alignment
  assert(((size_t)src & (size_t)15) == 0);
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)numBytes & (size_t)15) == 0);

#ifndef __SSE__
  const uint16_t mul = 32767 / max;
  for (unsigned i=0; i<numBytes; i+=2)
  {
    dst[i] = conbrg_c(((uint16_t)src[i]) - ((uint16_t)min), mul);
    dst[i+1] = src[i+1];
  }
#else
  const __m128i cmin = _mm_set1_epi16(min);
  const __m128i cmul = _mm_set1_epi16(32767 / max);
  const __m128i c0 = _mm_setzero_si128();
  const __m128i c255 = _mm_set1_epi16(255);

  for (unsigned i=0; i<numBytes; i+=(sizeof(__m128i)/sizeof(uint8_t)))
  {
    __m128i a  = _mm_load_si128((__m128i *)(src + i));
    __m128i am = _mm_and_si128(a, c255);

    __m128i r = conbrg_sse(am, cmin, cmul, c0, c255);
    r = _mm_or_si128(r, _mm_andnot_si128(c255, a));

    _mm_store_si128((__m128i *)(dst + i), r);
  }
#endif
}

void LXiStream_AnalogVideoFilter_conbrg_UYVY
 (uint8_t * restrict dst, const uint8_t * restrict src, uint8_t min, uint8_t max, unsigned numBytes)
{
  // Check alignment
  assert(((size_t)src & (size_t)15) == 0);
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)numBytes & (size_t)15) == 0);

#ifndef __SSE__
  const uint16_t mul = 32767 / max;
  for (unsigned i=0; i<numBytes; i+=2)
  {
    dst[i] = conbrg_c(((uint16_t)src[i]) - ((uint16_t)min), mul);
    dst[i+1] = src[i+1];
  }
#else
  const __m128i cmin = _mm_set1_epi16(min);
  const __m128i cmul = _mm_set1_epi16(32767 / max);
  const __m128i c0 = _mm_setzero_si128();
  const __m128i c255 = _mm_set1_epi16(255);

  for (unsigned i=0; i<numBytes; i+=(sizeof(__m128i)/sizeof(uint8_t)))
  {
    __m128i a  = _mm_load_si128((__m128i *)(src + i));
    __m128i am = _mm_srli_epi16(a, 8);

    __m128i r = conbrg_sse(am, cmin, cmul, c0, c255);
    r = _mm_or_si128(_mm_slli_epi16(r, 8), _mm_and_si128(a, c255));

    _mm_store_si128((__m128i *)(dst + i), r);
  }
#endif
}
