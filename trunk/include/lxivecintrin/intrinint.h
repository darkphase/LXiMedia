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
#ifndef LXIVECINTRIN_INTRININT_H
#define LXIVECINTRIN_INTRININT_H

#include "intrin.h"

namespace lxivec {
namespace _private {

  template <int _count> union Floats;
  template <int _count> union Doubles;

#ifdef _MSC_VER
# pragma pack(1)
#endif

  template <typename _type, int _count> union Ints { };

  template <int _count>
  union lxivec_packed lxivec_align Ints<int8_t, _count>
  {
    int8_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(int8_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Ints<uint8_t, _count>
  {
    uint8_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(uint8_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Ints<int16_t, _count>
  {
    int16_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(int16_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Ints<uint16_t, _count>
  {
    uint16_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(uint16_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Ints<int32_t, _count>
  {
    int32_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(int32_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Ints<uint32_t, _count>
  {
    uint32_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(uint32_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Ints<int64_t, _count>
  {
    int64_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(int64_t))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Ints<uint64_t, _count>
  {
    uint64_t val[_count];
#if defined(__SSE2__)
    __m128i vec[_count / (sizeof(__m128i) / sizeof(uint64_t))];
#endif
  };

#ifdef _MSC_VER
# pragma pack()
#endif

#if defined(__SSE2__)
  typedef struct { int64_t a, b; } int64pair;

  lxivec_always_inline int64pair get_int64(__m128i a)
  {
    int64pair result;

# ifdef __x86_64__
    result.a = _mm_cvtsi128_si64(a);
    result.b = _mm_cvtsi128_si64(_mm_srli_si128(a, 8));
# else
    result.a =
        int64_t(_mm_cvtsi128_si32(a)) |
        (int64_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 4))) << 32);
    result.b =
        int64_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 8))) |
        (int64_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 12))) << 32);
# endif

    return result;
  }

  typedef struct { uint64_t a, b; } uint64pair;

  lxivec_always_inline uint64pair get_uint64(__m128i a)
  {
    uint64pair result;

# ifdef __x86_64__
    result.a = uint64_t(_mm_cvtsi128_si64(a));
    result.b = uint64_t(_mm_cvtsi128_si64(_mm_srli_si128(a, 8)));
# else
    result.a =
        uint64_t(_mm_cvtsi128_si32(a)) |
        (uint64_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 4))) << 32);
    result.b =
        uint64_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 8))) |
        (uint64_t(_mm_cvtsi128_si32(_mm_srli_si128(a, 12))) << 32);
# endif

    return result;
  }
#endif

  template <typename _type, int _count>
  lxivec_always_inline void load(Ints<_type, _count> &dst, const _type *p, int max = _count)
  {
    max = max >= _count ? _count : max;
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; (vi < int(sizeof(dst.vec) / sizeof(dst.vec[0]))) && (vi <= int(max / (sizeof(dst.vec[0]) / sizeof(*p)))); vi++)
      dst.vec[vi] = _mm_load_si128(reinterpret_cast<const __m128i *>(p) + vi);

    i += sizeof(dst.vec) / sizeof(*p);
#endif

    for (; i<max; i++)
      dst.val[i] = p[i];
  }

  template <typename _type, int _count>
  lxivec_always_inline void store(_type *p, const Ints<_type, _count> &src, int max = _count)
  {
    max = max >= _count ? _count : max;
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; (vi < int(sizeof(src.vec) / sizeof(src.vec[0]))) && (vi <= int(max / (sizeof(src.vec[0]) / sizeof(*p)))); vi++)
      _mm_store_si128(reinterpret_cast<__m128i *>(p) + vi, src.vec[vi]);

    i += sizeof(src.vec) / sizeof(*p);
#endif

    for (; i<max; i++)
      p[i] = src.val[i];
  }

  template <typename _type, int _count>
  lxivec_always_inline void loadu(Ints<_type, _count> &dst, const _type *p, int max = _count)
  {
    max = max >= _count ? _count : max;
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; (vi < int(sizeof(dst.vec) / sizeof(dst.vec[0]))) && (vi <= int(max / (sizeof(dst.vec[0]) / sizeof(*p)))); vi++)
      dst.vec[vi] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p) + vi);

    i += sizeof(dst.vec) / sizeof(*p);
#endif

    for (; i<max; i++)
      dst.val[i] = p[i];
  }

  template <typename _type, int _count>
  lxivec_always_inline void storeu(_type *p, const Ints<_type, _count> &src, int max = _count)
  {
    max = max >= _count ? _count : max;
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; (vi < int(sizeof(src.vec) / sizeof(src.vec[0]))) && (vi <= int(max / (sizeof(src.vec[0]) / sizeof(*p)))); vi++)
      _mm_storeu_si128(reinterpret_cast<__m128i *>(p) + vi, src.vec[vi]);

    i += sizeof(src.vec) / sizeof(*p);
#endif

    for (; i<max; i++)
      p[i] = src.val[i];
  }

  template <typename _type, int _count>
  lxivec_always_inline void copy(Ints<_type, _count> &dst, const Ints<_type, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = src.vec[vi];

    i += sizeof(dst.vec) / sizeof(_type);
#endif

    for (; i<_count; i++)
      dst.val[i] = src.val[i];
  }
  
  template <typename _type> lxivec_always_inline _type max_val();
  template <> lxivec_always_inline int8_t max_val<int8_t>() { return int8_t(0x7F); }
  template <> lxivec_always_inline uint8_t max_val<uint8_t>() { return uint8_t(0xFF); }
  template <> lxivec_always_inline int16_t max_val<int16_t>() { return int16_t(0x7FFF); }
  template <> lxivec_always_inline uint16_t max_val<uint16_t>() { return uint16_t(0xFFFF); }
  template <> lxivec_always_inline int32_t max_val<int32_t>() { return int32_t(0x7FFFFFFF); }
  template <> lxivec_always_inline uint32_t max_val<uint32_t>() { return uint32_t(0xFFFFFFFFu); }
  template <> lxivec_always_inline int64_t max_val<int64_t>() { return int64_t(0x7FFFFFFFFFFFFFFFll); }
  template <> lxivec_always_inline uint64_t max_val<uint64_t>() { return uint64_t(0xFFFFFFFFFFFFFFFFull); }
  
  template <typename _type> lxivec_always_inline _type min_val();
  template <> lxivec_always_inline int8_t min_val<int8_t>() { return int8_t(0x80); }
  template <> lxivec_always_inline uint8_t min_val<uint8_t>() { return 0; }
  template <> lxivec_always_inline int16_t min_val<int16_t>() { return int16_t(0x8000); }
  template <> lxivec_always_inline uint16_t min_val<uint16_t>() { return 0; }
  template <> lxivec_always_inline int32_t min_val<int32_t>() { return int32_t(0x80000000); }
  template <> lxivec_always_inline uint32_t min_val<uint32_t>() { return 0; }
  template <> lxivec_always_inline int64_t min_val<int64_t>() { return int64_t(0x8000000000000000ll); }
  template <> lxivec_always_inline uint64_t min_val<uint64_t>() { return 0; }
  
  template <typename _totype, typename _fromtype>
  lxivec_always_inline _totype saturate(_fromtype v)
  {
    if (v < _fromtype(min_val<_totype>()))
      return min_val<_totype>();
    else if (v > _fromtype(max_val<_totype>()))
      return max_val<_totype>();
    else
      return _totype(v);
  }

  template <>
  lxivec_always_inline int8_t saturate<int8_t, uint16_t>(uint16_t v)
  {
    return (v <= uint16_t(max_val<int8_t>())) ? int8_t(v) : max_val<int8_t>();
  }

  template <>
  lxivec_always_inline int8_t saturate<int8_t, uint32_t>(uint32_t v)
  {
    return (v <= uint32_t(max_val<int8_t>())) ? int8_t(v) : max_val<int8_t>();
  }

  template <>
  lxivec_always_inline int8_t saturate<int8_t, uint64_t>(uint64_t v)
  {
    return (v <= uint64_t(max_val<int8_t>())) ? int8_t(v) : max_val<int8_t>();
  }

  template <>
  lxivec_always_inline uint8_t saturate<uint8_t, uint16_t>(uint16_t v)
  {
    return (v <= uint16_t(max_val<uint8_t>())) ? uint8_t(v) : max_val<uint8_t>();
  }

  template <>
  lxivec_always_inline uint8_t saturate<uint8_t, uint32_t>(uint32_t v)
  {
    return (v <= uint32_t(max_val<uint8_t>())) ? uint8_t(v) : max_val<uint8_t>();
  }

  template <>
  lxivec_always_inline uint8_t saturate<uint8_t, uint64_t>(uint64_t v)
  {
    return (v <= uint64_t(max_val<uint8_t>())) ? uint8_t(v) : max_val<uint8_t>();
  }

  template <>
  lxivec_always_inline int16_t saturate<int16_t, uint32_t>(uint32_t v)
  {
    return (v <= uint32_t(max_val<int16_t>())) ? int16_t(v) : max_val<int16_t>();
  }

  template <>
  lxivec_always_inline int16_t saturate<int16_t, uint64_t>(uint64_t v)
  {
    return (v <= uint64_t(max_val<int16_t>())) ? int16_t(v) : max_val<int16_t>();
  }

  template <>
  lxivec_always_inline uint16_t saturate<uint16_t, uint32_t>(uint32_t v)
  {
    return (v <= uint32_t(max_val<uint16_t>())) ? uint16_t(v) : max_val<uint16_t>();
  }

  template <>
  lxivec_always_inline uint16_t saturate<uint16_t, uint64_t>(uint64_t v)
  {
    return (v <= uint64_t(max_val<uint16_t>())) ? uint16_t(v) : max_val<uint16_t>();
  }

  template <>
  lxivec_always_inline int32_t saturate<int32_t, uint64_t>(uint64_t v)
  {
    return (v <= uint64_t(max_val<int32_t>())) ? int32_t(v) : max_val<int32_t>();
  }

  template <>
  lxivec_always_inline uint32_t saturate<uint32_t, uint64_t>(uint64_t v)
  {
    return (v <= uint64_t(max_val<uint32_t>())) ? int32_t(v) : max_val<uint32_t>();
  }
  
} } // End of namespaces

#include "intrinint_compare.h"
#include "intrinint_logic.h"
#include "intrinint_math.h"
#include "intrinint_repack8.h"
#include "intrinint_repack16.h"
#include "intrinint_repack32.h"
#include "intrinint_repack64.h"
#include "intrinint_repackfloat.h"
#include "intrinint_set.h"

#endif
