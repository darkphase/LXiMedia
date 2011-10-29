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

  lxivec_always_inline int8_t saturate_int8(int16_t v)
  {
    return (v >= -128) ? ((v <= 127) ? int8_t(v) : int8_t(127)) : int8_t(-128);
  }

  lxivec_always_inline int8_t saturate_int8(int32_t v)
  {
    return (v >= -128) ? ((v <= 127) ? int8_t(v) : int8_t(127)) : int8_t(-128);
  }

  lxivec_always_inline int8_t saturate_int8(int64_t v)
  {
    return (v >= -128) ? ((v <= 127) ? int8_t(v) : int8_t(127)) : int8_t(-128);
  }

  lxivec_always_inline int8_t saturate_int8(uint16_t v)
  {
    return (v <= 127) ? int8_t(v) : int8_t(127);
  }

  lxivec_always_inline int8_t saturate_int8(uint32_t v)
  {
    return (v <= 127) ? int8_t(v) : int8_t(127);
  }

  lxivec_always_inline int8_t saturate_int8(uint64_t v)
  {
    return (v <= 127) ? int8_t(v) : int8_t(127);
  }

  lxivec_always_inline uint8_t saturate_uint8(int16_t v)
  {
    return (v >= 0) ? ((v <= 255) ? uint8_t(v) : uint8_t(255)) : uint8_t(0);
  }

  lxivec_always_inline uint8_t saturate_uint8(int32_t v)
  {
    return (v >= 0) ? ((v <= 255) ? uint8_t(v) : uint8_t(255)) : uint8_t(0);
  }

  lxivec_always_inline uint8_t saturate_uint8(int64_t v)
  {
    return (v >= 0) ? ((v <= 255) ? uint8_t(v) : uint8_t(255)) : uint8_t(0);
  }

  lxivec_always_inline uint8_t saturate_uint8(uint16_t v)
  {
    return (v <= 255) ? uint8_t(v) : uint8_t(255);
  }

  lxivec_always_inline uint8_t saturate_uint8(uint32_t v)
  {
    return (v <= 255) ? uint8_t(v) : uint8_t(255);
  }

  lxivec_always_inline uint8_t saturate_uint8(uint64_t v)
  {
    return (v <= 255) ? uint8_t(v) : uint8_t(255);
  }

  lxivec_always_inline int16_t saturate_int16(int32_t v)
  {
    return (v >= -32768) ? ((v <= 32767) ? int16_t(v) : int16_t(32767)) : int16_t(-32768);
  }

  lxivec_always_inline int16_t saturate_int16(int64_t v)
  {
    return (v >= -32768) ? ((v <= 32767) ? int16_t(v) : int16_t(32767)) : int16_t(-32768);
  }

  lxivec_always_inline int16_t saturate_int16(uint32_t v)
  {
    return (v <= 32767) ? int16_t(v) : int16_t(32767);
  }

  lxivec_always_inline int16_t saturate_int16u(uint64_t v)
  {
    return (v <= 32767) ? int16_t(v) : int16_t(32767);
  }

  lxivec_always_inline uint16_t saturate_uint16(int32_t v)
  {
    return (v >= 0) ? ((v <= 65535) ? uint16_t(v) : uint16_t(65535)) : uint16_t(0);
  }

  lxivec_always_inline uint16_t saturate_uint16(int64_t v)
  {
    return (v >= 0) ? ((v <= 65535) ? uint16_t(v) : uint16_t(65535)) : uint16_t(0);
  }

  lxivec_always_inline uint16_t saturate_uint16(uint32_t v)
  {
    return (v <= 65535) ? uint16_t(v) : uint16_t(65535);
  }

  lxivec_always_inline uint16_t saturate_uint16(uint64_t v)
  {
    return (v <= 65535) ? uint16_t(v) : uint16_t(65535);
  }

  lxivec_always_inline int32_t saturate_int32(int64_t v)
  {
    return (v >= -2147483648ll) ? ((v <= 2147483647ll) ? int32_t(v) : int32_t(2147483647)) : int32_t(-2147483648);
  }

  lxivec_always_inline int32_t saturate_int32(uint64_t v)
  {
    return (v <= 2147483647ll) ? int32_t(v) : int32_t(2147483647);
  }

  lxivec_always_inline uint32_t saturate_uint32(int64_t v)
  {
    return (v >= 0) ? ((v <= 4294967295ll) ? uint32_t(v) : uint32_t(4294967295u)) : uint32_t(0);
  }

  lxivec_always_inline uint32_t saturate_uint32(uint64_t v)
  {
    return (v <= 4294967295ull) ? uint32_t(v) : uint32_t(4294967295u);
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
