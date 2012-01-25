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

#ifndef LXIVECINTRIN_VECTYPES
#error Please include <lxivecintrin/vectypes>
#endif

#include "intrinint.h"

namespace lxivec {
namespace _private {

  template <int _count>
  lxivec_always_inline void interleave(Ints<uint16_t, _count * 2> &dst, const Ints<uint16_t, _count> &a, const Ints<uint16_t, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(a.vec) / sizeof(a.vec[0])); vi++)
    {
      dst.vec[vi * 2    ] = _mm_unpacklo_epi16(a.vec[vi], b.vec[vi]);
      dst.vec[vi * 2 + 1] = _mm_unpackhi_epi16(a.vec[vi], b.vec[vi]);
    }

    i += sizeof(a.vec) / sizeof(a.val[0]);
#endif

    for (; i<_count; i++)
    {
      dst.val[i * 2    ] = a.val[i];
      dst.val[i * 2 + 1] = b.val[i];
    }
  }

  template <int _count>
  lxivec_always_inline void interleave(Ints<int16_t, _count * 2> &dst, const Ints<int16_t, _count> &a, const Ints<int16_t, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(a.vec) / sizeof(a.vec[0])); vi++)
    {
      dst.vec[vi * 2    ] = _mm_unpacklo_epi16(a.vec[vi], b.vec[vi]);
      dst.vec[vi * 2 + 1] = _mm_unpackhi_epi16(a.vec[vi], b.vec[vi]);
    }

    i += sizeof(a.vec) / sizeof(a.val[0]);
#endif

    for (; i<_count; i++)
    {
      dst.val[i * 2    ] = a.val[i];
      dst.val[i * 2 + 1] = b.val[i];
    }
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int16_t, _count> &dst, const Ints<uint16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = src.vec[vi];

    i += sizeof(dst.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = src.val[i];
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint16_t, _count> &dst, const Ints<int16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = src.vec[vi];

    i += sizeof(dst.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = src.val[i];
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int8_t, _count> &dst, const Ints<int16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_packs_epi16(src.vec[vi * 2], src.vec[vi * 2 + 1]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<int8_t, int16_t>(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint8_t, _count> &dst, const Ints<int16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_packus_epi16(src.vec[vi * 2], src.vec[vi * 2 + 1]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<uint8_t, int16_t>(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int8_t, _count> &dst, const Ints<uint16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_packs_epi16(
          _mm_packs_epi32(
              _mm_unpacklo_epi16(src.vec[vi * 2    ], _mm_setzero_si128()),
              _mm_unpackhi_epi16(src.vec[vi * 2    ], _mm_setzero_si128())),
          _mm_packs_epi32(
              _mm_unpacklo_epi16(src.vec[vi * 2 + 1], _mm_setzero_si128()),
              _mm_unpackhi_epi16(src.vec[vi * 2 + 1], _mm_setzero_si128())));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<int8_t, uint16_t>(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint8_t, _count> &dst, const Ints<uint16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_packus_epi16(
          _mm_packs_epi32(
              _mm_unpacklo_epi16(src.vec[vi * 2    ], _mm_setzero_si128()),
              _mm_unpackhi_epi16(src.vec[vi * 2    ], _mm_setzero_si128())),
          _mm_packs_epi32(
              _mm_unpacklo_epi16(src.vec[vi * 2 + 1], _mm_setzero_si128()),
              _mm_unpackhi_epi16(src.vec[vi * 2 + 1], _mm_setzero_si128())));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<uint8_t, uint16_t>(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int32_t, _count> &dst, const Ints<int16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i sign = _mm_cmplt_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 2    ] = _mm_unpacklo_epi16(src.vec[vi], sign);
      dst.vec[vi * 2 + 1] = _mm_unpackhi_epi16(src.vec[vi], sign);
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = int32_t(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint32_t, _count> &dst, const Ints<int16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i sign = _mm_cmplt_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 2    ] = _mm_unpacklo_epi16(src.vec[vi], sign);
      dst.vec[vi * 2 + 1] = _mm_unpackhi_epi16(src.vec[vi], sign);
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = uint32_t(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int32_t, _count> &dst, const Ints<uint16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      dst.vec[vi * 2    ] = _mm_unpacklo_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 2 + 1] = _mm_unpackhi_epi16(src.vec[vi], _mm_setzero_si128());
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = int32_t(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint32_t, _count> &dst, const Ints<uint16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      dst.vec[vi * 2    ] = _mm_unpacklo_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 2 + 1] = _mm_unpackhi_epi16(src.vec[vi], _mm_setzero_si128());
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = uint32_t(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int64_t, _count> &dst, const Ints<int16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i sign = _mm_cmplt_epi16(src.vec[vi], _mm_setzero_si128());

      const __m128i lo = _mm_unpacklo_epi16(src.vec[vi], sign);
      const __m128i losign = _mm_unpacklo_epi16(sign, sign);
      dst.vec[vi * 4    ] = _mm_unpacklo_epi32(lo, losign);
      dst.vec[vi * 4 + 1] = _mm_unpackhi_epi32(lo, losign);

      const __m128i hi = _mm_unpackhi_epi16(src.vec[vi], sign);
      const __m128i hisign = _mm_unpackhi_epi16(sign, sign);
      dst.vec[vi * 4 + 2] = _mm_unpacklo_epi32(hi, hisign);
      dst.vec[vi * 4 + 3] = _mm_unpackhi_epi32(hi, hisign);
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = int64_t(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint64_t, _count> &dst, const Ints<int16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i sign = _mm_cmplt_epi16(src.vec[vi], _mm_setzero_si128());

      const __m128i lo = _mm_unpacklo_epi16(src.vec[vi], sign);
      const __m128i losign = _mm_unpacklo_epi16(sign, sign);
      dst.vec[vi * 4    ] = _mm_unpacklo_epi32(lo, losign);
      dst.vec[vi * 4 + 1] = _mm_unpackhi_epi32(lo, losign);

      const __m128i hi = _mm_unpackhi_epi16(src.vec[vi], sign);
      const __m128i hisign = _mm_unpackhi_epi16(sign, sign);
      dst.vec[vi * 4 + 2] = _mm_unpacklo_epi32(hi, hisign);
      dst.vec[vi * 4 + 3] = _mm_unpackhi_epi32(hi, hisign);
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = uint64_t(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int64_t, _count> &dst, const Ints<uint16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i lo = _mm_unpacklo_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 4    ] = _mm_unpacklo_epi32(lo, _mm_setzero_si128());
      dst.vec[vi * 4 + 1] = _mm_unpackhi_epi32(lo, _mm_setzero_si128());

      const __m128i hi = _mm_unpackhi_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 4 + 2] = _mm_unpacklo_epi32(hi, _mm_setzero_si128());
      dst.vec[vi * 4 + 3] = _mm_unpackhi_epi32(hi, _mm_setzero_si128());
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = int64_t(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint64_t, _count> &dst, const Ints<uint16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i lo = _mm_unpacklo_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 4    ] = _mm_unpacklo_epi32(lo, _mm_setzero_si128());
      dst.vec[vi * 4 + 1] = _mm_unpackhi_epi32(lo, _mm_setzero_si128());

      const __m128i hi = _mm_unpackhi_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 4 + 2] = _mm_unpacklo_epi32(hi, _mm_setzero_si128());
      dst.vec[vi * 4 + 3] = _mm_unpackhi_epi32(hi, _mm_setzero_si128());
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = uint64_t(src.val[i]);
  }

} } // End of namespaces
