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

#if defined(__SSE__)
  lxivec_always_inline __m128 cmpeq(__m128 a, __m128 b)   { return _mm_cmpeq_ps(a, b); }
  lxivec_always_inline __m128 cmpgt(__m128 a, __m128 b)   { return _mm_cmpgt_ps(a, b); }
  lxivec_always_inline __m128 cmplt(__m128 a, __m128 b)   { return _mm_cmplt_ps(a, b); }
#endif
#if defined(__SSE2__)
  lxivec_always_inline __m128d cmpeq(__m128d a, __m128d b)   { return _mm_cmpeq_pd(a, b); }
  lxivec_always_inline __m128d cmpgt(__m128d a, __m128d b)   { return _mm_cmpgt_pd(a, b); }
  lxivec_always_inline __m128d cmplt(__m128d a, __m128d b)   { return _mm_cmplt_pd(a, b); }
#endif

  template <int _count>
  lxivec_always_inline void cmpeq(Bits<4, _count> &dst, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_castps_si128(cmpeq(a.vec[vi], b.vec[vi]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#elif defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      const __m128 r = cmpeq(a.vec[vi], b.vec[vi]);
      _mm_storel_pi(dst.vec + (vi * 2    ), r);
      _mm_storeh_pi(dst.vec + (vi * 2 + 1), r);
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] == b.val[i]) ? 0xFFFFFFFFu : 0x00000000u;
  }

  template <int _count>
  lxivec_always_inline void cmpeq(Bits<8, _count> &dst, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_castpd_si128(cmpeq(a.vec[vi], b.vec[vi]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] == b.val[i]) ? 0xFFFFFFFFFFFFFFFFull : 0x0000000000000000ull;
  }

  template <int _count>
  lxivec_always_inline void cmpgt(Bits<4, _count> &dst, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_castps_si128(cmpgt(a.vec[vi], b.vec[vi]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#elif defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      const __m128 r = cmpgt(a.vec[vi], b.vec[vi]);
      _mm_storel_pi(dst.vec + (vi * 2    ), r);
      _mm_storeh_pi(dst.vec + (vi * 2 + 1), r);
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] > b.val[i]) ? 0xFFFFFFFFu : 0x00000000u;
  }

  template <int _count>
  lxivec_always_inline void cmpgt(Bits<8, _count> &dst, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_castpd_si128(cmpgt(a.vec[vi], b.vec[vi]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] > b.val[i]) ? 0xFFFFFFFFFFFFFFFFull : 0x0000000000000000ull;
  }

  template <int _count>
  lxivec_always_inline void cmplt(Bits<4, _count> &dst, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_castps_si128(cmplt(a.vec[vi], b.vec[vi]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#elif defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      const __m128 r = cmplt(a.vec[vi], b.vec[vi]);
      _mm_storel_pi(dst.vec + (vi * 2    ), r);
      _mm_storeh_pi(dst.vec + (vi * 2 + 1), r);
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] < b.val[i]) ? 0xFFFFFFFFu : 0x00000000u;
  }

  template <int _count>
  lxivec_always_inline void cmplt(Bits<8, _count> &dst, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_castpd_si128(cmplt(a.vec[vi], b.vec[vi]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] < b.val[i]) ? 0xFFFFFFFFFFFFFFFFull : 0x0000000000000000ull;
  }

#if defined(__SSE__)
  lxivec_always_inline __m128  min(__m128 a, __m128 b)  { return _mm_min_ps(a, b); }
  lxivec_always_inline __m128  max(__m128 a, __m128 b)  { return _mm_max_ps(a, b); }
#endif
#if defined(__SSE2__)
  lxivec_always_inline __m128d min(__m128d a, __m128d b)  { return _mm_min_pd(a, b); }
  lxivec_always_inline __m128d max(__m128d a, __m128d b)  { return _mm_max_pd(a, b); }
#endif

  template <int _count>
  lxivec_always_inline void min(Floats<_count> &dst, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = min(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] <= b.val[i]) ? a.val[i] : b.val[i];
  }

  template <int _count>
  lxivec_always_inline void max(Floats<_count> &dst, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = max(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] >= b.val[i]) ? a.val[i] : b.val[i];
  }

  template <int _count>
  lxivec_always_inline void min(Doubles<_count> &dst, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = min(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] <= b.val[i]) ? a.val[i] : b.val[i];
  }

  template <int _count>
  lxivec_always_inline void max(Doubles<_count> &dst, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = max(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (a.val[i] >= b.val[i]) ? a.val[i] : b.val[i];
  }

  template <int _count>
  lxivec_always_inline void select(Floats<_count> &dst, const Bits<4, _count> &m, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_or_ps(
          _mm_and_ps(_mm_castsi128_ps(m.vec[vi]), a.vec[vi]),
          _mm_andnot_ps(_mm_castsi128_ps(m.vec[vi]), b.vec[vi]));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#elif defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      __m128 mv;
      mv = _mm_loadl_pi(mv, m.vec + (vi * 2    ));
      mv = _mm_loadh_pi(mv, m.vec + (vi * 2 + 1));
      dst.vec[vi] = _mm_or_ps(_mm_and_ps(mv, a.vec[vi]), _mm_andnot_ps(mv, b.vec[vi]));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (m.val[i] != 0) ? a.val[i] : b.val[i];
  }

  template <int _count>
  lxivec_always_inline void select(Doubles<_count> &dst, const Bits<8, _count> &m, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_or_pd(
          _mm_and_pd(_mm_castsi128_pd(m.vec[vi]), a.vec[vi]),
          _mm_andnot_pd(_mm_castsi128_pd(m.vec[vi]), b.vec[vi]));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    if (i < _count) release();
    for (; i<_count; i++)
      dst.val[i] = (m.val[i] != 0) ? a.val[i] : b.val[i];
  }

} } // End of namespaces
