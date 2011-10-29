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

#ifndef LXIVECINTRIN_VECTYPES
#error Please include <lxivecintrin/vectypes>
#endif

#include "intrinint.h"

namespace lxivec {
namespace _private {

#if defined(__SSE2__)
  lxivec_always_inline __m128i bor(__m128i a, int8_t b)   { return _mm_or_si128(a, _mm_set1_epi8(b)); }
  lxivec_always_inline __m128i bor(__m128i a, uint8_t b)  { return _mm_or_si128(a, _mm_set1_epi8(b)); }
  lxivec_always_inline __m128i bor(__m128i a, int16_t b)  { return _mm_or_si128(a, _mm_set1_epi16(b)); }
  lxivec_always_inline __m128i bor(__m128i a, uint16_t b) { return _mm_or_si128(a, _mm_set1_epi16(b)); }
  lxivec_always_inline __m128i bor(__m128i a, int32_t b)  { return _mm_or_si128(a, _mm_set1_epi32(b)); }
  lxivec_always_inline __m128i bor(__m128i a, uint32_t b) { return _mm_or_si128(a, _mm_set1_epi32(b)); }
  lxivec_always_inline __m128i bor(__m128i a, int64_t b)  { return _mm_or_si128(a, _mm_set1_epi64x(b)); }
  lxivec_always_inline __m128i bor(__m128i a, uint64_t b) { return _mm_or_si128(a, _mm_set1_epi64x(b)); }

  lxivec_always_inline __m128i band(__m128i a, int8_t b)   { return _mm_and_si128(a, _mm_set1_epi8(b)); }
  lxivec_always_inline __m128i band(__m128i a, uint8_t b)  { return _mm_and_si128(a, _mm_set1_epi8(b)); }
  lxivec_always_inline __m128i band(__m128i a, int16_t b)  { return _mm_and_si128(a, _mm_set1_epi16(b)); }
  lxivec_always_inline __m128i band(__m128i a, uint16_t b) { return _mm_and_si128(a, _mm_set1_epi16(b)); }
  lxivec_always_inline __m128i band(__m128i a, int32_t b)  { return _mm_and_si128(a, _mm_set1_epi32(b)); }
  lxivec_always_inline __m128i band(__m128i a, uint32_t b) { return _mm_and_si128(a, _mm_set1_epi32(b)); }
  lxivec_always_inline __m128i band(__m128i a, int64_t b)  { return _mm_and_si128(a, _mm_set1_epi64x(b)); }
  lxivec_always_inline __m128i band(__m128i a, uint64_t b) { return _mm_and_si128(a, _mm_set1_epi64x(b)); }
#endif

  template <typename _type, int _count>
  lxivec_always_inline void bor(Ints<_type, _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_or_si128(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] | b.val[i];
  }

  template <typename _type, int _count>
  lxivec_always_inline void band(Ints<_type, _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_and_si128(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] & b.val[i];
  }

  template <typename _type, int _count>
  lxivec_always_inline void bor(Ints<_type, _count> &dst, const Ints<_type, _count> &a, _type b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = bor(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] | b;
  }

  template <typename _type, int _count>
  lxivec_always_inline void band(Ints<_type, _count> &dst, const Ints<_type, _count> &a, _type b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = band(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] & b;
  }

#if defined(__SSE2__)
  lxivec_always_inline __m128i shl(__m128i a, int b, int8_t)
  {
    return _mm_slli_epi16(_mm_andnot_si128(_mm_set1_epi8(((1 << b) - 1) << (8 - b)), a), b);
  }

  lxivec_always_inline __m128i shl(__m128i a, int b, uint8_t)
  {
    return _mm_slli_epi16(_mm_andnot_si128(_mm_set1_epi8(((1 << b) - 1) << (8 - b)), a), b);
  }

  lxivec_always_inline __m128i shl(__m128i a, int b, int16_t)  { return _mm_slli_epi16(a, b); }
  lxivec_always_inline __m128i shl(__m128i a, int b, uint16_t) { return _mm_slli_epi16(a, b); }
  lxivec_always_inline __m128i shl(__m128i a, int b, int32_t)  { return _mm_slli_epi32(a, b); }
  lxivec_always_inline __m128i shl(__m128i a, int b, uint32_t) { return _mm_slli_epi32(a, b); }
  lxivec_always_inline __m128i shl(__m128i a, int b, int64_t)  { return _mm_slli_epi64(a, b); }
  lxivec_always_inline __m128i shl(__m128i a, int b, uint64_t) { return _mm_slli_epi64(a, b); }

  lxivec_always_inline __m128i shr(__m128i a, int b, int8_t)
  {
    return _mm_or_si128(
        _mm_srli_epi16(_mm_andnot_si128(_mm_set1_epi8((1 << b) - 1), a), b),
        _mm_and_si128(_mm_set1_epi8(((1 << b) - 1) << (8 - b)), _mm_cmplt_epi8(a, _mm_setzero_si128())));
  }

  lxivec_always_inline __m128i shr(__m128i a, int b, uint8_t)
  {
    return _mm_srli_epi16(_mm_andnot_si128(_mm_set1_epi8((1 << b) - 1), a), b);
  }

  lxivec_always_inline __m128i shr(__m128i a, int b, int16_t)  { return _mm_srai_epi16(a, b); }
  lxivec_always_inline __m128i shr(__m128i a, int b, uint16_t) { return _mm_srli_epi16(a, b); }
  lxivec_always_inline __m128i shr(__m128i a, int b, int32_t)  { return _mm_srai_epi32(a, b); }
  lxivec_always_inline __m128i shr(__m128i a, int b, uint32_t) { return _mm_srli_epi32(a, b); }
  lxivec_always_inline __m128i shr(__m128i a, int b, int64_t)
  {
    const int64pair sa = get_int64(a);
    return _mm_set_epi64x(sa.b >> b, sa.a >> b);
  }

  lxivec_always_inline __m128i shr(__m128i a, int b, uint64_t) { return _mm_srli_epi64(a, b); }
#endif

  template <typename _type, int _count>
  lxivec_always_inline void shl(Ints<_type, _count> &dst, const Ints<_type, _count> &a, int b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = shl(a.vec[vi], b, dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] << b;
  }

  template <typename _type, int _count>
  lxivec_always_inline void shr(Ints<_type, _count> &dst, const Ints<_type, _count> &a, int b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = shr(a.vec[vi], b, dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] >> b;
  }

} } // End of namespaces
