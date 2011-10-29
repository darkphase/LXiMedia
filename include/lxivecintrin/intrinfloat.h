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
#ifndef LXIVECINTRIN_INTRINFLOAT_H
#define LXIVECINTRIN_INTRINFLOAT_H

#include "intrin.h"

namespace lxivec {
namespace _private {

  template <typename _type, int _count> union Ints;

#ifdef _MSC_VER
# pragma pack(1)
#endif

  template <int _count>
  union lxivec_packed lxivec_align Floats
  {
    float val[_count];
#if defined(__SSE__)
    __m128 vec[_count / (sizeof(__m128) / sizeof(float))];
#endif
  };

  template <int _count>
  union lxivec_packed lxivec_align Doubles
  {
    double val[_count];
#if defined(__SSE2__)
    __m128d vec[_count / (sizeof(__m128d) / sizeof(double))];
#endif
  };

  template <int _count>
  lxivec_always_inline void load(Floats<_count> &dst, const float *p, int max = _count)
  {
    max = max >= _count ? _count : max;
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; (vi < int(sizeof(dst.vec) / sizeof(dst.vec[0]))) && (vi <= int(max / (sizeof(dst.vec[0]) / sizeof(*p)))); vi++)
      dst.vec[vi] = _mm_load_ps(p + (vi * (sizeof(dst.vec[0]) / sizeof(*p))));

    i += sizeof(dst.vec) / sizeof(*p);
#endif

    for (; i<max; i++)
      dst.val[i] = p[i];
  }

  template <int _count>
  lxivec_always_inline void load(Doubles<_count> &dst, const double *p, int max = _count)
  {
    max = max >= _count ? _count : max;
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; (vi < int(sizeof(dst.vec) / sizeof(dst.vec[0]))) && (vi <= int(max / (sizeof(dst.vec[0]) / sizeof(*p)))); vi++)
      dst.vec[vi] = _mm_load_pd(p + (vi * (sizeof(dst.vec[0]) / sizeof(*p))));

    i += sizeof(dst.vec) / sizeof(*p);
#endif

    for (; i<max; i++)
      dst.val[i] = p[i];
  }

  template <int _count>
  lxivec_always_inline void store(float *p, const Floats<_count> &src, int max = _count)
  {
    max = max >= _count ? _count : max;
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; (vi < int(sizeof(src.vec) / sizeof(src.vec[0]))) && (vi <= int(max / (sizeof(src.vec[0]) / sizeof(*p)))); vi++)
      _mm_store_ps(p + (vi * (sizeof(src.vec[0]) / sizeof(*p))), src.vec[vi]);

    i += sizeof(src.vec) / sizeof(*p);
#endif

    for (; i<max; i++)
      p[i] = src.val[i];
  }

  template <int _count>
  lxivec_always_inline void store(double *p, const Doubles<_count> &src, int max = _count)
  {
    max = max >= _count ? _count : max;
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; (vi < int(sizeof(src.vec) / sizeof(src.vec[0]))) && (vi <= int(max / (sizeof(src.vec[0]) / sizeof(*p)))); vi++)
      _mm_store_pd(p + (vi * (sizeof(src.vec[0]) / sizeof(*p))), src.vec[vi]);

    i += sizeof(src.vec) / sizeof(*p);
#endif

    for (; i<max; i++)
      p[i] = src.val[i];
  }

  template <int _count>
  lxivec_always_inline void copy(Floats<_count> &dst, const Floats<_count> &src)
  {
    int i = 0;

#if defined(__SSE__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = src.vec[vi];

    i += sizeof(dst.vec) / sizeof(float);
#endif

    for (; i<_count; i++)
      dst.val[i] = src.val[i];
  }

  template <int _count>
  lxivec_always_inline void copy(Doubles<_count> &dst, const Doubles<_count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = src.vec[vi];

    i += sizeof(dst.vec) / sizeof(double);
#endif

    for (; i<_count; i++)
      dst.val[i] = src.val[i];
  }

} } // End of namespaces

#include "intrinfloat_compare.h"
#include "intrinfloat_math.h"
#include "intrinfloat_repack.h"

#endif
