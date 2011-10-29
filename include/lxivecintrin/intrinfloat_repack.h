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

#include "intrinfloat.h"

namespace lxivec {
namespace _private {

  template <int _count>
  lxivec_always_inline void repack(Floats<_count> &dst, const Ints<uint8_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i lo = _mm_unpacklo_epi8(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 4    ] = _mm_cvtepi32_ps(_mm_unpacklo_epi16(lo, _mm_setzero_si128()));
      dst.vec[vi * 4 + 1] = _mm_cvtepi32_ps(_mm_unpackhi_epi16(lo, _mm_setzero_si128()));

      const __m128i hi = _mm_unpackhi_epi8(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 4 + 2] = _mm_cvtepi32_ps(_mm_unpacklo_epi16(hi, _mm_setzero_si128()));
      dst.vec[vi * 4 + 3] = _mm_cvtepi32_ps(_mm_unpackhi_epi16(hi, _mm_setzero_si128()));
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Floats<_count> &dst, const Ints<int8_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i lo = _mm_unpacklo_epi8(_mm_setzero_si128(), src.vec[vi]);
      dst.vec[vi * 4    ] = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), lo), 24));
      dst.vec[vi * 4 + 1] = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), lo), 24));

      const __m128i hi = _mm_unpackhi_epi8(_mm_setzero_si128(), src.vec[vi]);
      dst.vec[vi * 4 + 2] = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), hi), 24));
      dst.vec[vi * 4 + 3] = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), hi), 24));
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Floats<_count> &dst, const Ints<uint16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      dst.vec[vi * 2    ] = _mm_cvtepi32_ps(_mm_unpacklo_epi16(src.vec[vi], _mm_setzero_si128()));
      dst.vec[vi * 2 + 1] = _mm_cvtepi32_ps(_mm_unpackhi_epi16(src.vec[vi], _mm_setzero_si128()));
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Floats<_count> &dst, const Ints<int16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      dst.vec[vi * 2    ] = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), src.vec[vi]), 16));
      dst.vec[vi * 2 + 1] = _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), src.vec[vi]), 16));
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Floats<_count> &dst, const Ints<uint32_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Floats<_count> &dst, const Ints<int32_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
      dst.vec[vi] = _mm_cvtepi32_ps(src.vec[vi]);

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Floats<_count> &dst, const Ints<uint64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Floats<_count> &dst, const Ints<int64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Doubles<_count> &dst, const Ints<uint8_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i lo = _mm_unpacklo_epi8(src.vec[vi], _mm_setzero_si128());
      const __m128i lolo = _mm_unpacklo_epi16(lo, _mm_setzero_si128());
      dst.vec[vi * 8    ] = _mm_cvtepi32_pd(lolo);
      dst.vec[vi * 8 + 1] = _mm_cvtepi32_pd(_mm_srli_si128(lolo, 8));

      const __m128i lohi = _mm_unpackhi_epi16(lo, _mm_setzero_si128());
      dst.vec[vi * 8 + 2] = _mm_cvtepi32_pd(lohi);
      dst.vec[vi * 8 + 3] = _mm_cvtepi32_pd(_mm_srli_si128(lohi, 8));

      const __m128i hi = _mm_unpackhi_epi8(src.vec[vi], _mm_setzero_si128());
      const __m128i hilo = _mm_unpacklo_epi16(hi, _mm_setzero_si128());
      dst.vec[vi * 8 + 4] = _mm_cvtepi32_pd(hilo);
      dst.vec[vi * 8 + 5] = _mm_cvtepi32_pd(_mm_srli_si128(hilo, 8));

      const __m128i hihi = _mm_unpackhi_epi16(hi, _mm_setzero_si128());
      dst.vec[vi * 8 + 6] = _mm_cvtepi32_pd(hihi);
      dst.vec[vi * 8 + 7] = _mm_cvtepi32_pd(_mm_srli_si128(hihi, 8));
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Doubles<_count> &dst, const Ints<int8_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i lo = _mm_unpacklo_epi8(_mm_setzero_si128(), src.vec[vi]);
      const __m128i lolo = _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), lo), 24);
      dst.vec[vi * 8    ] = _mm_cvtepi32_pd(lolo);
      dst.vec[vi * 8 + 1] = _mm_cvtepi32_pd(_mm_srli_si128(lolo, 8));

      const __m128i lohi = _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), lo), 24);
      dst.vec[vi * 8 + 2] = _mm_cvtepi32_pd(lohi);
      dst.vec[vi * 8 + 3] = _mm_cvtepi32_pd(_mm_srli_si128(lohi, 8));

      const __m128i hi = _mm_unpackhi_epi8(_mm_setzero_si128(), src.vec[vi]);
      const __m128i hilo = _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), hi), 24);
      dst.vec[vi * 8 + 4] = _mm_cvtepi32_pd(hilo);
      dst.vec[vi * 8 + 5] = _mm_cvtepi32_pd(_mm_srli_si128(hilo, 8));

      const __m128i hihi = _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), hi), 24);
      dst.vec[vi * 8 + 6] = _mm_cvtepi32_pd(hihi);
      dst.vec[vi * 8 + 7] = _mm_cvtepi32_pd(_mm_srli_si128(hihi, 8));
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Doubles<_count> &dst, const Ints<uint16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i lo = _mm_unpacklo_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 4    ] = _mm_cvtepi32_pd(lo);
      dst.vec[vi * 4 + 1] = _mm_cvtepi32_pd(_mm_srli_si128(lo, 8));

      const __m128i hi = _mm_unpackhi_epi16(src.vec[vi], _mm_setzero_si128());
      dst.vec[vi * 4 + 2] = _mm_cvtepi32_pd(hi);
      dst.vec[vi * 4 + 3] = _mm_cvtepi32_pd(_mm_srli_si128(hi, 8));
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Doubles<_count> &dst, const Ints<int16_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      const __m128i lo = _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), src.vec[vi]), 16);
      dst.vec[vi * 4    ] = _mm_cvtepi32_pd(lo);
      dst.vec[vi * 4 + 1] = _mm_cvtepi32_pd(_mm_srli_si128(lo, 8));

      const __m128i hi = _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), src.vec[vi]), 16);
      dst.vec[vi * 4 + 2] = _mm_cvtepi32_pd(hi);
      dst.vec[vi * 4 + 3] = _mm_cvtepi32_pd(_mm_srli_si128(hi, 8));
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = float(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Doubles<_count> &dst, const Ints<uint32_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = double(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Doubles<_count> &dst, const Ints<int32_t, _count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
    {
      dst.vec[vi * 2    ] = _mm_cvtepi32_pd(src.vec[vi]);
      dst.vec[vi * 2 + 1] = _mm_cvtepi32_pd(_mm_srli_si128(src.vec[vi], 8));
    }

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = double(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Doubles<_count> &dst, const Ints<uint64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = double(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Doubles<_count> &dst, const Ints<int64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = double(src.val[i]);
  }

} } // End of namespaces
