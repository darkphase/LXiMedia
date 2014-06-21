/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#ifndef LXIVECINTRIN_VECTYPES
#error Please include <lxivecintrin/vectypes>
#endif
#ifndef LXIVECINTRIN_INTRINBOOL_H
#define LXIVECINTRIN_INTRINBOOL_H

#include "intrin.h"

namespace lxivec {
namespace _private {

#ifdef _MSC_VER
# pragma pack(1)
#endif

  template <int _size, int _count> union Bits { };

  template <int _count>
  union lxivec_packed lxivec_align Bits<1, _count>
  {
    uint8_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(uint8_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Bits<2, _count>
  {
    uint16_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(uint16_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Bits<4, _count>
  {
    uint32_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(uint32_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Bits<8, _count>
  {
    uint64_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(uint64_t))];
#endif
  };

#ifdef _MSC_VER
# pragma pack()
#endif

  template <int _size, int _count>
  lxivec_always_inline void copy(Bits<_size, _count> &dst, const Bits<_size, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = src.vec[vi];

    i += sizeof(dst.vec) / _size;
#endif

    for (; i<_count; i++)
      dst.val[i] = src.val[i];
  }

  template <int _size, int _count>
  lxivec_always_inline void bor(Bits<_size, _count> &dst, const Bits<_size, _count> &a, const Bits<_size, _count> &b)
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

  template <int _size, int _count>
  lxivec_always_inline void band(Bits<_size, _count> &dst, const Bits<_size, _count> &a, const Bits<_size, _count> &b)
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

  template <int _size, int _count>
  lxivec_always_inline void bnot(Bits<_size, _count> &dst, const Bits<_size, _count> &a)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_andnot_si128(a.vec[vi], _mm_cmpeq_epi32(a.vec[vi], a.vec[vi]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = ~a.val[i];
  }

  template <int _size, int _count>
  lxivec_always_inline void cmpeq(Bits<_size, _count> &dst, const Bits<_size, _count> &a, const Bits<_size, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_andnot_si128(_mm_xor_si128(a.vec[vi], b.vec[vi]), _mm_cmpeq_epi32(a.vec[vi], a.vec[vi]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = ~(a.val[i] ^ b.val[i]);
  }

  template <int _size, int _count>
  lxivec_always_inline void cmpne(Bits<_size, _count> &dst, const Bits<_size, _count> &a, const Bits<_size, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_xor_si128(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] ^ b.val[i];
  }

} } // End of namespaces

#endif
