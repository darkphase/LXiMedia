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
  lxivec_always_inline __m128i add(__m128i a, __m128i b, int8_t)   { return _mm_add_epi8(a, b); }
  lxivec_always_inline __m128i add(__m128i a, __m128i b, uint8_t)  { return _mm_add_epi8(a, b); }
  lxivec_always_inline __m128i add(__m128i a, __m128i b, int16_t)  { return _mm_add_epi16(a, b); }
  lxivec_always_inline __m128i add(__m128i a, __m128i b, uint16_t) { return _mm_add_epi16(a, b); }
  lxivec_always_inline __m128i add(__m128i a, __m128i b, int32_t)  { return _mm_add_epi32(a, b); }
  lxivec_always_inline __m128i add(__m128i a, __m128i b, uint32_t) { return _mm_add_epi32(a, b); }
  lxivec_always_inline __m128i add(__m128i a, __m128i b, int64_t)  { return _mm_add_epi64(a, b); }
  lxivec_always_inline __m128i add(__m128i a, __m128i b, uint64_t) { return _mm_add_epi64(a, b); }

  lxivec_always_inline __m128i add(__m128i a, int8_t b)   { return _mm_add_epi8(a, _mm_set1_epi8(b)); }
  lxivec_always_inline __m128i add(__m128i a, uint8_t b)  { return _mm_add_epi8(a, _mm_set1_epi8(b)); }
  lxivec_always_inline __m128i add(__m128i a, int16_t b)  { return _mm_add_epi16(a, _mm_set1_epi16(b)); }
  lxivec_always_inline __m128i add(__m128i a, uint16_t b) { return _mm_add_epi16(a, _mm_set1_epi16(b)); }
  lxivec_always_inline __m128i add(__m128i a, int32_t b)  { return _mm_add_epi32(a, _mm_set1_epi32(b)); }
  lxivec_always_inline __m128i add(__m128i a, uint32_t b) { return _mm_add_epi32(a, _mm_set1_epi32(b)); }
  lxivec_always_inline __m128i add(__m128i a, int64_t b)  { return _mm_add_epi64(a, _mm_set1_epi64x(b)); }
  lxivec_always_inline __m128i add(__m128i a, uint64_t b) { return _mm_add_epi64(a, _mm_set1_epi64x(b)); }

  lxivec_always_inline __m128i sub(__m128i a, __m128i b, int8_t)   { return _mm_sub_epi8(a, b); }
  lxivec_always_inline __m128i sub(__m128i a, __m128i b, uint8_t)  { return _mm_sub_epi8(a, b); }
  lxivec_always_inline __m128i sub(__m128i a, __m128i b, int16_t)  { return _mm_sub_epi16(a, b); }
  lxivec_always_inline __m128i sub(__m128i a, __m128i b, uint16_t) { return _mm_sub_epi16(a, b); }
  lxivec_always_inline __m128i sub(__m128i a, __m128i b, int32_t)  { return _mm_sub_epi32(a, b); }
  lxivec_always_inline __m128i sub(__m128i a, __m128i b, uint32_t) { return _mm_sub_epi32(a, b); }
  lxivec_always_inline __m128i sub(__m128i a, __m128i b, int64_t)  { return _mm_sub_epi64(a, b); }
  lxivec_always_inline __m128i sub(__m128i a, __m128i b, uint64_t) { return _mm_sub_epi64(a, b); }

  lxivec_always_inline __m128i sub(__m128i a, int8_t b)   { return _mm_sub_epi8(a, _mm_set1_epi8(b)); }
  lxivec_always_inline __m128i sub(__m128i a, uint8_t b)  { return _mm_sub_epi8(a, _mm_set1_epi8(b)); }
  lxivec_always_inline __m128i sub(__m128i a, int16_t b)  { return _mm_sub_epi16(a, _mm_set1_epi16(b)); }
  lxivec_always_inline __m128i sub(__m128i a, uint16_t b) { return _mm_sub_epi16(a, _mm_set1_epi16(b)); }
  lxivec_always_inline __m128i sub(__m128i a, int32_t b)  { return _mm_sub_epi32(a, _mm_set1_epi32(b)); }
  lxivec_always_inline __m128i sub(__m128i a, uint32_t b) { return _mm_sub_epi32(a, _mm_set1_epi32(b)); }
  lxivec_always_inline __m128i sub(__m128i a, int64_t b)  { return _mm_sub_epi64(a, _mm_set1_epi64x(b)); }
  lxivec_always_inline __m128i sub(__m128i a, uint64_t b) { return _mm_sub_epi64(a, _mm_set1_epi64x(b)); }
#endif

  template <typename _type, int _count>
  lxivec_always_inline void add(Ints<_type, _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = add(a.vec[vi], b.vec[vi], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] + b.val[i];
  }

  template <typename _type, int _count>
  lxivec_always_inline void sub(Ints<_type, _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = sub(a.vec[vi], b.vec[vi], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] - b.val[i];
  }

  template <typename _type, int _count>
  lxivec_always_inline void add(Ints<_type, _count> &dst, const Ints<_type, _count> &a, _type b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = add(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] + b;
  }

  template <typename _type, int _count>
  lxivec_always_inline void sub(Ints<_type, _count> &dst, const Ints<_type, _count> &a, _type b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = sub(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] - b;
  }

#if defined(__SSE2__)
  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, int8_t)   
  { 
    return _mm_packs_epi16(
        _mm_add_epi16(
            _mm_and_si128(a, _mm_set1_epi16(0x00FF)),
            _mm_srli_epi16(a, 8)),
        _mm_add_epi16(
            _mm_and_si128(b, _mm_set1_epi16(0x00FF)),
            _mm_srli_epi16(b, 8)));
  }
  
  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, uint8_t)  
  { 
    return _mm_packus_epi16(
        _mm_add_epi16(
            _mm_and_si128(a, _mm_set1_epi16(0x00FF)),
            _mm_srli_epi16(a, 8)),
        _mm_add_epi16(
            _mm_and_si128(b, _mm_set1_epi16(0x00FF)),
            _mm_srli_epi16(b, 8)));
  }
  
#if defined(__SSSE3__)
  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, int16_t)  { return _mm_hadds_epi16(a, b); }
  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, uint16_t) { return _mm_hadd_epi16(a, b); }
  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, int32_t)  { return _mm_hadd_epi32(a, b); }
  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, uint32_t) { return _mm_hadd_epi32(a, b); }
#else
  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, int16_t)  
  { 
    return _mm_packs_epi32(
        _mm_add_epi32(
            _mm_and_si128(a, _mm_set1_epi32(0x0000FFFF)),
            _mm_srli_epi32(a, 16)),
        _mm_add_epi32(
            _mm_and_si128(b, _mm_set1_epi32(0x0000FFFF)),
            _mm_srli_epi32(b, 16)));
  }

  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, uint16_t)
  {
    return _mm_add_epi16(
        _mm_packs_epi32(
            _mm_sub_epi32(
                _mm_add_epi32(
                    _mm_and_si128(a, _mm_set1_epi32(0x0000FFFF)),
                    _mm_srli_epi32(a, 16)),
                _mm_set1_epi32(32768)),
            _mm_sub_epi32(
                _mm_add_epi32(
                    _mm_and_si128(b, _mm_set1_epi32(0x0000FFFF)),
                    _mm_srli_epi32(b, 16)),
                _mm_set1_epi32(32768))),
        _mm_set1_epi16(32768));
  }

  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, int32_t)
  {
    return _mm_set_epi32(
        int32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 8)))  + int32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 12))),
        int32_t(_mm_cvtsi128_si32(b))                     + int32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 4))),
        int32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 8)))  + int32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 12))),
        int32_t(_mm_cvtsi128_si32(a))                     + int32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 4))));
  }

  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, uint32_t)
  {
    return _mm_set_epi32(
        uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 8))) + uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 12))),
        uint32_t(_mm_cvtsi128_si32(b))                    + uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 4))),
        uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 8))) + uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 12))),
        uint32_t(_mm_cvtsi128_si32(a))                    + uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 4))));
  }
#endif

  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, int64_t)
  {
    const int64pair sa = get_int64(a);
    const int64pair sb = get_int64(b);
    return _mm_set_epi64x(sb.a + sb.b, sa.a + sa.b);
  }

  lxivec_always_inline __m128i hadd(__m128i a, __m128i b, uint64_t)
  {
    const uint64pair sa = get_uint64(a);
    const uint64pair sb = get_uint64(b);
    return _mm_set_epi64x(sb.a + sb.b, sa.a + sa.b);
  }
#endif

  template <typename _type, int _count>
  lxivec_always_inline void hadd(Ints<_type, _count / 2> &dst, const Ints<_type, _count> &a)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = hadd(a.vec[vi * 2], a.vec[vi * 2 + 1], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count/2; i++)
      dst.val[i] = a.val[i * 2] + a.val[i * 2 + 1];
  }

  template <typename _type, int _count>
  lxivec_always_inline _type hsum(const Ints<_type, _count> &a)
  {
    int i = 0;

    _type result = 0;
    for (; i<_count; i++)
      result += a.val[i];

    return result;
  }

#if defined(__SSE2__)
  lxivec_always_inline __m128i mul(__m128i a, __m128i b, int8_t)
  {
    const __m128i signa = _mm_cmplt_epi8(a, _mm_setzero_si128());
    const __m128i signb = _mm_cmplt_epi8(b, _mm_setzero_si128());

    return _mm_packs_epi16(
        _mm_srai_epi16(
            _mm_slli_epi16(
                _mm_mullo_epi16(
                    _mm_unpacklo_epi8(a, signa),
                    _mm_unpacklo_epi8(b, signb)),
                8),
            8),
        _mm_srai_epi16(
            _mm_slli_epi16(
                _mm_mullo_epi16(
                    _mm_unpackhi_epi8(a, signa),
                    _mm_unpackhi_epi8(b, signb)),
                8),
            8));
  }

  lxivec_always_inline __m128i mul(__m128i a, __m128i b, uint8_t)
  {
    return _mm_packus_epi16(
        _mm_srli_epi16(
            _mm_slli_epi16(
                _mm_mullo_epi16(
                    _mm_unpacklo_epi8(a, _mm_setzero_si128()),
                    _mm_unpacklo_epi8(b, _mm_setzero_si128())),
                8),
            8),
        _mm_srli_epi16(
            _mm_slli_epi16(
                _mm_mullo_epi16(
                    _mm_unpackhi_epi8(a, _mm_setzero_si128()),
                    _mm_unpackhi_epi8(b, _mm_setzero_si128())),
                8),
            8));
  }

  lxivec_always_inline __m128i mul(__m128i a, __m128i b, int16_t)  { return _mm_mullo_epi16(a, b); }
  lxivec_always_inline __m128i mul(__m128i a, __m128i b, uint16_t) { return _mm_mullo_epi16(a, b); }

#if defined(__SSE4A__)
  lxivec_always_inline __m128i mul(__m128i a, __m128i b, int32_t)  { return _mm_mullo_epi32(a, b); }
  lxivec_always_inline __m128i mul(__m128i a, __m128i b, uint32_t) { return _mm_mullo_epi32(a, b); }
#else
  lxivec_always_inline __m128i mul(__m128i a, __m128i b, int32_t)
  {
    return _mm_set_epi32(
        int32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 12))) * int32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 12))),
        int32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 8)))  * int32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 8))),
        int32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 4)))  * int32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 4))),
        int32_t(_mm_cvtsi128_si32(a))                     * int32_t(_mm_cvtsi128_si32(b)));
  }

  lxivec_always_inline __m128i mul(__m128i a, __m128i b, uint32_t)
  {
    return _mm_set_epi32(
        uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 12))) * uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 12))),
        uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 8)))  * uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 8))),
        uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 4)))  * uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(b, 4))),
        uint32_t(_mm_cvtsi128_si32(a))                     * uint32_t(_mm_cvtsi128_si32(b)));
  }
#endif

  lxivec_always_inline __m128i mul(__m128i a, __m128i b, int64_t)
  {
    const int64pair sa = get_int64(a);
    const int64pair sb = get_int64(b);
    return _mm_set_epi64x(sa.b * sb.b, sa.a * sb.a);
  }

  lxivec_always_inline __m128i mul(__m128i a, __m128i b, uint64_t)
  {
    const uint64pair sa = get_uint64(a);
    const uint64pair sb = get_uint64(b);
    return _mm_set_epi64x(sa.b * sb.b, sa.a * sb.a);
  }

  lxivec_always_inline __m128i mul(__m128i a, int8_t b)
  {
    const __m128i signa = _mm_cmplt_epi8(a, _mm_setzero_si128());

    return _mm_packs_epi16(
        _mm_srai_epi16(
            _mm_slli_epi16(
                _mm_mullo_epi16(
                    _mm_unpacklo_epi8(a, signa),
                    _mm_set1_epi16(b)),
                8),
            8),
        _mm_srai_epi16(
            _mm_slli_epi16(
                _mm_mullo_epi16(
                    _mm_unpackhi_epi8(a, signa),
                    _mm_set1_epi16(b)),
                8),
            8));
  }

  lxivec_always_inline __m128i mul(__m128i a, uint8_t b)
  {
    return _mm_packus_epi16(
        _mm_srli_epi16(
            _mm_slli_epi16(
                _mm_mullo_epi16(
                    _mm_unpacklo_epi8(a, _mm_setzero_si128()),
                    _mm_set1_epi16(b)),
                8),
            8),
        _mm_srli_epi16(
            _mm_slli_epi16(
                _mm_mullo_epi16(
                    _mm_unpackhi_epi8(a, _mm_setzero_si128()),
                    _mm_set1_epi16(b)),
                8),
            8));
  }

  lxivec_always_inline __m128i mul(__m128i a, int16_t b)  { return _mm_mullo_epi16(a, _mm_set1_epi16(b)); }
  lxivec_always_inline __m128i mul(__m128i a, uint16_t b) { return _mm_mullo_epi16(a, _mm_set1_epi16(b)); }

  lxivec_always_inline __m128i mul(__m128i a, int32_t b)
  {
    return _mm_set_epi32(
        int32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 12))) * b,
        int32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 8)))  * b,
        int32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 4)))  * b,
        int32_t(_mm_cvtsi128_si32(a))                     * b);
  }

  lxivec_always_inline __m128i mul(__m128i a, uint32_t b)
  {
    return _mm_set_epi32(
        uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 12))) * b,
        uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 8)))  * b,
        uint32_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 4)))  * b,
        uint32_t(_mm_cvtsi128_si32(a))                     * b);
  }

  lxivec_always_inline __m128i mul(__m128i a, int64_t b)
  {
    const int64pair sa = get_int64(a);
    return _mm_set_epi64x(sa.b * b, sa.a * b);
  }

  lxivec_always_inline __m128i mul(__m128i a, uint64_t b)
  {
    const uint64pair sa = get_uint64(a);
    return _mm_set_epi64x(sa.b * b, sa.a * b);
  }
#endif

  template <typename _type, int _count>
  lxivec_always_inline void mul(Ints<_type, _count> &dst, const Ints<_type, _count> &a, const Ints<_type, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = mul(a.vec[vi], b.vec[vi], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] * b.val[i];
  }

  template <typename _type, int _count>
  lxivec_always_inline void mul(Ints<_type, _count> &dst, const Ints<_type, _count> &a, _type b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = mul(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] * b;
  }

#if defined(__SSSE3__)
  lxivec_always_inline __m128i abs(__m128i a, int8_t)   { return _mm_abs_epi8(a); }
  lxivec_always_inline __m128i abs(__m128i a, int16_t)  { return _mm_abs_epi16(a); }
  lxivec_always_inline __m128i abs(__m128i a, int32_t)  { return _mm_abs_epi32(a); }

  lxivec_always_inline __m128i abs(__m128i a, int64_t)
  {
    const int64pair sa = get_int64(a);
    return _mm_set_epi64x((sa.b >= -sa.b) ? sa.b : -sa.b, (sa.a >= -sa.a) ? sa.a : -sa.a);
  }
#elif defined(__SSE2__)
  lxivec_always_inline __m128i abs(__m128i a, int8_t)
  {
    const __m128i ai = _mm_sub_epi8(_mm_setzero_si128(), a);
    const __m128i m = _mm_cmpgt_epi8(a, ai);
    return _mm_or_si128(_mm_and_si128(a, m), _mm_andnot_si128(m, ai));
  }

  lxivec_always_inline __m128i abs(__m128i a, int16_t)
  {
    return _mm_max_epi16(a, _mm_sub_epi16(_mm_setzero_si128(), a));
  }

  lxivec_always_inline __m128i abs(__m128i a, int32_t)
  {
    const __m128i ai = _mm_sub_epi32(_mm_setzero_si128(), a);
    const __m128i m = _mm_cmpgt_epi32(a, ai);
    return _mm_or_si128(_mm_and_si128(a, m), _mm_andnot_si128(m, ai));
  }

  lxivec_always_inline __m128i abs(__m128i a, int64_t)
  {
    const int64pair sa = get_int64(a);
    return _mm_set_epi64x((sa.b >= -sa.b) ? sa.b : -sa.b, (sa.a >= -sa.a) ? sa.a : -sa.a);
  }
#endif

#if defined(__SSE2__)
  lxivec_always_inline __m128i abs(__m128i a, uint8_t)  { return a; }
  lxivec_always_inline __m128i abs(__m128i a, uint16_t) { return a; }
  lxivec_always_inline __m128i abs(__m128i a, uint32_t) { return a; }
  lxivec_always_inline __m128i abs(__m128i a, uint64_t) { return a; }
#endif

  template <typename _type, int _count>
  lxivec_always_inline void abs(Ints<_type, _count> &dst, const Ints<_type, _count> &a)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = abs(a.vec[vi], dst.val[0]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = lxivec::abs(a.val[i]);
  }

} } // End of namespaces
