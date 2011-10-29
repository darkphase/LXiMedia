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

  template <int _count>
  lxivec_always_inline void interleave(Ints<uint64_t, _count * 2> &dst, const Ints<uint64_t, _count> &a, const Ints<uint64_t, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(a.vec) / sizeof(a.vec[0])); vi++)
    {
      dst.vec[vi * 2    ] = _mm_unpacklo_epi64(a.vec[vi], b.vec[vi]);
      dst.vec[vi * 2 + 1] = _mm_unpackhi_epi64(a.vec[vi], b.vec[vi]);
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
  lxivec_always_inline void interleave(Ints<int64_t, _count * 2> &dst, const Ints<int64_t, _count> &a, const Ints<int64_t, _count> &b)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(a.vec) / sizeof(a.vec[0])); vi++)
    {
      dst.vec[vi * 2    ] = _mm_unpacklo_epi64(a.vec[vi], b.vec[vi]);
      dst.vec[vi * 2 + 1] = _mm_unpackhi_epi64(a.vec[vi], b.vec[vi]);
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
  lxivec_always_inline void repack(Ints<int64_t, _count> &dst, const Ints<uint64_t, _count> &src)
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
  lxivec_always_inline void repack(Ints<uint64_t, _count> &dst, const Ints<int64_t, _count> &src)
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
  lxivec_always_inline void repack(Ints<int8_t, _count> &dst, const Ints<int64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_int8(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint8_t, _count> &dst, const Ints<int64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_uint8(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int8_t, _count> &dst, const Ints<uint64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_int8(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint8_t, _count> &dst, const Ints<uint64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_uint8(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int16_t, _count> &dst, const Ints<int64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_int16(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint16_t, _count> &dst, const Ints<int64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_uint16(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int16_t, _count> &dst, const Ints<uint64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_int16u(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint16_t, _count> &dst, const Ints<uint64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_uint16(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int32_t, _count> &dst, const Ints<int64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_int32(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint32_t, _count> &dst, const Ints<int64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_uint32(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<int32_t, _count> &dst, const Ints<uint64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_int32(src.val[i]);
  }

  template <int _count>
  lxivec_always_inline void repack(Ints<uint32_t, _count> &dst, const Ints<uint64_t, _count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = saturate_uint32(src.val[i]);
  }

} } // End of namespaces
