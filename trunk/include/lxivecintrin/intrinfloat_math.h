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

#include "intrinfloat.h"

namespace lxivec {
namespace _private {

#if defined(__SSE__)
  lxivec_always_inline __m128  add(__m128  a, __m128  b)           { return _mm_add_ps(a, b); }
  lxivec_always_inline __m128  add(__m128  a, float b)             { return _mm_add_ps(a, _mm_set1_ps(b)); }
#endif
#if defined(__SSE2__)
  lxivec_always_inline __m128d add(__m128d a, __m128d b)           { return _mm_add_pd(a, b); }
  lxivec_always_inline __m128d add(__m128d a, double b)            { return _mm_add_pd(a, _mm_set1_pd(b)); }
#endif
#if defined(__SSE__)
  lxivec_always_inline __m128  sub(__m128  a, __m128  b)           { return _mm_sub_ps(a, b); }
  lxivec_always_inline __m128  sub(__m128  a, float b)             { return _mm_sub_ps(a, _mm_set1_ps(b)); }
#endif
#if defined(__SSE2__)
  lxivec_always_inline __m128d sub(__m128d a, __m128d b)           { return _mm_sub_pd(a, b); }
  lxivec_always_inline __m128d sub(__m128d a, double b)            { return _mm_sub_pd(a, _mm_set1_pd(b)); }
#endif

  template <int _count>
  lxivec_always_inline void add(Floats<_count> &dst, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = add(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] + b.val[i];
  }

  template <int _count>
  lxivec_always_inline void sub(Floats<_count> &dst, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = sub(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] - b.val[i];
  }

  template <int _count>
  lxivec_always_inline void add(Floats<_count> &dst, const Floats<_count> &a, float b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = add(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] + b;
  }

  template <int _count>
  lxivec_always_inline void sub(Floats<_count> &dst, const Floats<_count> &a, float b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = sub(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] - b;
  }

  template <int _count>
  lxivec_always_inline void add(Doubles<_count> &dst, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = add(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] + b.val[i];
  }

  template <int _count>
  lxivec_always_inline void sub(Doubles<_count> &dst, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = sub(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] - b.val[i];
  }

  template <int _count>
  lxivec_always_inline void add(Doubles<_count> &dst, const Doubles<_count> &a, double b)
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

  template <int _count>
  lxivec_always_inline void sub(Doubles<_count> &dst, const Doubles<_count> &a, double b)
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

#if defined(__SSE__)
  lxivec_always_inline __m128  mul(__m128  a, __m128  b)           { return _mm_mul_ps(a, b); }
  lxivec_always_inline __m128  mul(__m128  a, float b)             { return _mm_mul_ps(a, _mm_set1_ps(b)); }
#endif
#if defined(__SSE2__)
  lxivec_always_inline __m128d mul(__m128d a, __m128d b)           { return _mm_mul_pd(a, b); }
  lxivec_always_inline __m128d mul(__m128d a, double b)            { return _mm_mul_pd(a, _mm_set1_pd(b)); }
#endif
#if defined(__SSE__)
  lxivec_always_inline __m128  div(__m128  a, __m128  b)           { return _mm_div_ps(a, b); }
  lxivec_always_inline __m128  div(__m128  a, float b)             { return _mm_div_ps(a, _mm_set1_ps(b)); }
#endif
#if defined(__SSE2__)
  lxivec_always_inline __m128d div(__m128d a, __m128d b)           { return _mm_div_pd(a, b); }
  lxivec_always_inline __m128d div(__m128d a, double b)            { return _mm_div_pd(a, _mm_set1_pd(b)); }
#endif

  template <int _count>
  lxivec_always_inline void mul(Floats<_count> &dst, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = mul(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] * b.val[i];
  }

  template <int _count>
  lxivec_always_inline void div(Floats<_count> &dst, const Floats<_count> &a, const Floats<_count> &b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = div(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] / b.val[i];
  }

  template <int _count>
  lxivec_always_inline void mul(Floats<_count> &dst, const Floats<_count> &a, float b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = mul(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] * b;
  }

  template <int _count>
  lxivec_always_inline void div(Floats<_count> &dst, const Floats<_count> &a, float b)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = div(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] / b;
  }

  template <int _count>
  lxivec_always_inline void mul(Doubles<_count> &dst, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = mul(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] * b.val[i];
  }

  template <int _count>
  lxivec_always_inline void div(Doubles<_count> &dst, const Doubles<_count> &a, const Doubles<_count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = div(a.vec[vi], b.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] / b.val[i];
  }

  template <int _count>
  lxivec_always_inline void mul(Doubles<_count> &dst, const Doubles<_count> &a, double b)
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

  template <int _count>
  lxivec_always_inline void div(Doubles<_count> &dst, const Doubles<_count> &a, double b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = div(a.vec[vi], b);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = a.val[i] / b;
  }

#if defined(__SSE__)
  lxivec_always_inline __m128 abs(__m128 a)
  {
    const __m128 ai = _mm_sub_ps(_mm_setzero_ps(), a);
    const __m128 m = _mm_cmpgt_ps(a, ai);
    return _mm_or_ps(_mm_and_ps(a, m), _mm_andnot_ps(m, ai));
  }
#endif
#if defined(__SSE2__)
  lxivec_always_inline __m128d abs(__m128d a)
  {
    const __m128d ai = _mm_sub_pd(_mm_setzero_pd(), a);
    const __m128d m = _mm_cmpgt_pd(a, ai);
    return _mm_or_pd(_mm_and_pd(a, m), _mm_andnot_pd(m, ai));
  }
#endif

  template <int _count>
  lxivec_always_inline void abs(Floats<_count> &dst, const Floats<_count> &a)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = abs(a.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = lxivec::abs(a.val[i]);
  }

  template <int _count>
  lxivec_always_inline void abs(Doubles<_count> &dst, const Doubles<_count> &a)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = abs(a.vec[vi]);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = lxivec::abs(a.val[i]);
  }

} } // End of namespaces
