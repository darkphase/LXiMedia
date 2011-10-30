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
  lxivec_always_inline __m128i cmpeq(__m128i a, __m128i b, int8_t)   { return _mm_cmpeq_epi8(a, b); }
  lxivec_always_inline __m128i cmpeq(__m128i a, __m128i b, uint8_t)  { return _mm_cmpeq_epi8(a, b); }
  lxivec_always_inline __m128i cmpeq(__m128i a, __m128i b, int16_t)  { return _mm_cmpeq_epi16(a, b); }
  lxivec_always_inline __m128i cmpeq(__m128i a, __m128i b, uint16_t) { return _mm_cmpeq_epi16(a, b); }
  lxivec_always_inline __m128i cmpeq(__m128i a, __m128i b, int32_t)  { return _mm_cmpeq_epi32(a, b); }
  lxivec_always_inline __m128i cmpeq(__m128i a, __m128i b, uint32_t) { return _mm_cmpeq_epi32(a, b); }
  lxivec_always_inline __m128i cmpeq(__m128i a, __m128i b, int64_t)
  {
    const __m128i m = _mm_cmpeq_epi32(a, b);
    return _mm_and_si128(
        _mm_or_si128(_mm_slli_epi64(m, 32), _mm_srli_epi64(m, 32)),
        m);
  }

  lxivec_always_inline __m128i cmpeq(__m128i a, __m128i b, uint64_t)
  {
    const __m128i m = _mm_cmpeq_epi32(a, b);
    return _mm_and_si128(
        _mm_or_si128(_mm_slli_epi64(m, 32), _mm_srli_epi64(m, 32)),
        m);
  }

  lxivec_always_inline __m128i cmpgt(__m128i a, __m128i b, int8_t)   { return _mm_cmpgt_epi8(a, b); }
  lxivec_always_inline __m128i cmpgt(__m128i a, __m128i b, uint8_t)
  {
    return _mm_cmpgt_epi8(
        _mm_sub_epi8(a, _mm_set1_epi8(128u)),
        _mm_sub_epi8(b, _mm_set1_epi8(128u)));
  }

  lxivec_always_inline __m128i cmpgt(__m128i a, __m128i b, int16_t)  { return _mm_cmpgt_epi16(a, b); }
  lxivec_always_inline __m128i cmpgt(__m128i a, __m128i b, uint16_t)
  {
    return _mm_cmpgt_epi16(
        _mm_sub_epi16(a, _mm_set1_epi16(32768u)),
        _mm_sub_epi16(b, _mm_set1_epi16(32768u)));
  }

  lxivec_always_inline __m128i cmpgt(__m128i a, __m128i b, int32_t)  { return _mm_cmpgt_epi32(a, b); }
  lxivec_always_inline __m128i cmpgt(__m128i a, __m128i b, uint32_t)
  {
    return _mm_cmpgt_epi32(
        _mm_sub_epi32(a, _mm_set1_epi32(2147483648u)),
        _mm_sub_epi32(b, _mm_set1_epi32(2147483648u)));
  }

  lxivec_always_inline __m128i cmpgt(__m128i a, __m128i b, int64_t)
  {
    const int64pair sa = get_int64(a);
    const int64pair sb = get_int64(b);
    return _mm_set_epi64x((sa.b > sb.b) ? int64_t(-1) : int64_t(0), (sa.a > sb.a) ? int64_t(-1) : int64_t(0));
  }
  lxivec_always_inline __m128i cmpgt(__m128i a, __m128i b, uint64_t)
  {
    const uint64pair sa = get_uint64(a);
    const uint64pair sb = get_uint64(b);
    return _mm_set_epi64x((sa.b > sb.b) ? int64_t(-1) : int64_t(0), (sa.a > sb.a) ? int64_t(-1) : int64_t(0));
  }

  lxivec_always_inline __m128i cmplt(__m128i a, __m128i b, int8_t)   { return _mm_cmplt_epi8(a, b); }
  lxivec_always_inline __m128i cmplt(__m128i a, __m128i b, uint8_t)
  {
    return _mm_cmplt_epi8(
        _mm_sub_epi8(a, _mm_set1_epi8(128u)),
        _mm_sub_epi8(b, _mm_set1_epi8(128u)));
  }

  lxivec_always_inline __m128i cmplt(__m128i a, __m128i b, int16_t)  { return _mm_cmplt_epi16(a, b); }
  lxivec_always_inline __m128i cmplt(__m128i a, __m128i b, uint16_t)
  {
    return _mm_cmplt_epi16(
        _mm_sub_epi16(a, _mm_set1_epi16(32768u)),
        _mm_sub_epi16(b, _mm_set1_epi16(32768u)));
  }

  lxivec_always_inline __m128i cmplt(__m128i a, __m128i b, int32_t)  { return _mm_cmplt_epi32(a, b); }
  lxivec_always_inline __m128i cmplt(__m128i a, __m128i b, uint32_t)
  {
    return _mm_cmplt_epi32(
        _mm_sub_epi32(a, _mm_set1_epi32(2147483648u)),
        _mm_sub_epi32(b, _mm_set1_epi32(2147483648u)));
  }

  lxivec_always_inline __m128i cmplt(__m128i a, __m128i b, int64_t)
  {
    const int64pair sa = get_int64(a);
    const int64pair sb = get_int64(b);
    return _mm_set_epi64x((sa.b < sb.b) ? int64_t(-1) : int64_t(0), (sa.a < sb.a) ? int64_t(-1) : int64_t(0));
  }
  lxivec_always_inline __m128i cmplt(__m128i a, __m128i b, uint64_t)
  {
    const uint64pair sa = get_uint64(a);
    const uint64pair sb = get_uint64(b);
    return _mm_set_epi64x((sa.b < sb.b) ? int64_t(-1) : int64_t(0), (sa.a < sb.a) ? int64_t(-1) : int64_t(0));
  }
#endif

  template <typename _type, int _count>
  lxivec_always_inline void cmpeq(Bits<sizeof(_type), _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = cmpeq(a.vec[vi], b.vec[vi], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = (a.val[i] == b.val[i]) ? _type(-1) : _type(0);
  }

  template <typename _type, int _count>
  lxivec_always_inline void cmpgt(Bits<sizeof(_type), _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = cmpgt(a.vec[vi], b.vec[vi], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = (a.val[i] > b.val[i]) ? _type(-1) : _type(0);
  }

  template <typename _type, int _count>
  lxivec_always_inline void cmplt(Bits<sizeof(_type), _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = cmplt(a.vec[vi], b.vec[vi], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = (a.val[i] < b.val[i]) ? _type(-1) : _type(0);
  }

#if defined(__SSE4A__)
  lxivec_always_inline __m128i max(__m128i a, __m128i b, int8_t)   { return _mm_min_epi8(a, b); }
  lxivec_always_inline __m128i max(__m128i a, __m128i b, uint8_t)  { return _mm_min_epu8(a, b); }
  lxivec_always_inline __m128i max(__m128i a, __m128i b, int16_t)  { return _mm_min_epi16(a, b); }
  lxivec_always_inline __m128i max(__m128i a, __m128i b, uint16_t) { return _mm_min_epu16(a, b); }
  lxivec_always_inline __m128i max(__m128i a, __m128i b, int32_t)  { return _mm_min_epi32(a, b); }
  lxivec_always_inline __m128i max(__m128i a, __m128i b, uint32_t) { return _mm_min_epu32(a, b); }

  lxivec_always_inline __m128i min(__m128i a, __m128i b, int8_t)   { return _mm_min_epi8(a, b); }
  lxivec_always_inline __m128i min(__m128i a, __m128i b, uint8_t)  { return _mm_min_epu8(a, b); }
  lxivec_always_inline __m128i min(__m128i a, __m128i b, int16_t)  { return _mm_min_epi16(a, b); }
  lxivec_always_inline __m128i min(__m128i a, __m128i b, uint16_t) { return _mm_min_epu16(a, b); }
  lxivec_always_inline __m128i min(__m128i a, __m128i b, int32_t)  { return _mm_min_epi32(a, b); }
  lxivec_always_inline __m128i min(__m128i a, __m128i b, uint32_t) { return _mm_min_epu32(a, b); }
#elif defined(__SSE2__)
  lxivec_always_inline __m128i max(__m128i a, __m128i b, int8_t)
  {
    const __m128i m = _mm_cmpgt_epi8(a, b);
    return _mm_or_si128(_mm_and_si128(m, a), _mm_andnot_si128(m, b));
  }

  lxivec_always_inline __m128i max(__m128i a, __m128i b, uint8_t)  { return _mm_max_epu8(a, b); }
  lxivec_always_inline __m128i max(__m128i a, __m128i b, int16_t)  { return _mm_max_epi16(a, b); }

  lxivec_always_inline __m128i max(__m128i a, __m128i b, uint16_t)
  {
    const __m128i m = _mm_cmpgt_epi16(
        _mm_sub_epi16(a, _mm_set1_epi16(32768)),
        _mm_sub_epi16(b, _mm_set1_epi16(32768)));
    return _mm_or_si128(_mm_and_si128(m, a), _mm_andnot_si128(m, b));
  }

  lxivec_always_inline __m128i max(__m128i a, __m128i b, int32_t)
  {
    const __m128i m = _mm_cmpgt_epi32(a, b);
    return _mm_or_si128(_mm_and_si128(m, a), _mm_andnot_si128(m, b));
  }

  lxivec_always_inline __m128i max(__m128i a, __m128i b, uint32_t)
  {
    const __m128i m = _mm_cmpgt_epi32(
        _mm_sub_epi32(a, _mm_set1_epi32(2147483648u)),
        _mm_sub_epi32(b, _mm_set1_epi32(2147483648u)));
    return _mm_or_si128(_mm_and_si128(m, a), _mm_andnot_si128(m, b));
  }

  lxivec_always_inline __m128i min(__m128i a, __m128i b, int8_t)
  {
    const __m128i m = _mm_cmplt_epi8(a, b);
    return _mm_or_si128(_mm_and_si128(m, a), _mm_andnot_si128(m, b));
  }

  lxivec_always_inline __m128i min(__m128i a, __m128i b, uint8_t)  { return _mm_min_epu8(a, b); }
  lxivec_always_inline __m128i min(__m128i a, __m128i b, int16_t)  { return _mm_min_epi16(a, b); }

  lxivec_always_inline __m128i min(__m128i a, __m128i b, uint16_t)
  {
    const __m128i m = _mm_cmplt_epi16(
        _mm_sub_epi16(a, _mm_set1_epi16(32768)),
        _mm_sub_epi16(b, _mm_set1_epi16(32768)));
    return _mm_or_si128(_mm_and_si128(m, a), _mm_andnot_si128(m, b));
  }

  lxivec_always_inline __m128i min(__m128i a, __m128i b, int32_t)
  {
    const __m128i m = _mm_cmplt_epi32(a, b);
    return _mm_or_si128(_mm_and_si128(m, a), _mm_andnot_si128(m, b));
  }

  lxivec_always_inline __m128i min(__m128i a, __m128i b, uint32_t)
  {
    const __m128i m = _mm_cmplt_epi32(
        _mm_sub_epi32(a, _mm_set1_epi32(2147483648u)),
        _mm_sub_epi32(b, _mm_set1_epi32(2147483648u)));
    return _mm_or_si128(_mm_and_si128(m, a), _mm_andnot_si128(m, b));
  }
#endif

#if defined(__SSE2__)
  lxivec_always_inline __m128i max(__m128i a, __m128i b, int64_t)
  {
    const int64pair sa = get_int64(a);
    const int64pair sb = get_int64(b);
    return _mm_set_epi64x((sa.b >= sb.b) ? sa.b : sb.b, (sa.a >= sb.a) ? sa.a : sb.a);
  }

  lxivec_always_inline __m128i max(__m128i a, __m128i b, uint64_t)
  {
    const uint64pair sa = get_uint64(a);
    const uint64pair sb = get_uint64(b);
    return _mm_set_epi64x((sa.b >= sb.b) ? sa.b : sb.b, (sa.a >= sb.a) ? sa.a : sb.a);
  }

  lxivec_always_inline __m128i min(__m128i a, __m128i b, int64_t)
  {
    const int64pair sa = get_int64(a);
    const int64pair sb = get_int64(b);
    return _mm_set_epi64x((sa.b <= sb.b) ? sa.b : sb.b, (sa.a <= sb.a) ? sa.a : sb.a);
  }

  lxivec_always_inline __m128i min(__m128i a, __m128i b, uint64_t)
  {
    const uint64pair sa = get_uint64(a);
    const uint64pair sb = get_uint64(b);
    return _mm_set_epi64x((sa.b <= sb.b) ? sa.b : sb.b, (sa.a <= sb.a) ? sa.a : sb.a);
  }
#endif

  template <typename _type, int _count>
  lxivec_always_inline void min(Ints<_type, _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = min(a.vec[vi], b.vec[vi], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = lxivec::min(a.val[i], b.val[i]);
  }

  template <typename _type, int _count>
  lxivec_always_inline void max(Ints<_type, _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = max(a.vec[vi], b.vec[vi], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = lxivec::max(a.val[i], b.val[i]);
  }

  template <typename _type, int _count>
  lxivec_always_inline void select(Ints<_type, _count> &dst, const Bits<sizeof(_type), _count> &m, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_or_si128(
          _mm_and_si128(m.vec[vi], a.vec[vi]),
          _mm_andnot_si128(m.vec[vi], b.vec[vi]));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = lxivec::select((m.val[i] != 0), a.val[i], b.val[i]);
  }

} } // End of namespaces
