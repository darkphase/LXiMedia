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

#include "intrinint.h"

namespace lxivec {
namespace _private {

  template <int _count>
  lxivec_always_inline void set(Ints<int8_t, _count> &dst, int8_t v)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set1_epi8(v);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = v;
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int8_t, _count> &dst, int8_t v1, int8_t v2)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi8(v2, v1, v2, v1, v2, v1, v2, v1, v2, v1, v2, v1, v2, v1, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+2<=_count; i+=2)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int8_t, _count> &dst, int8_t v1, int8_t v2, int8_t v3, int8_t v4)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi8(v4, v3, v2, v1, v4, v3, v2, v1, v4, v3, v2, v1, v4, v3, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+4<=_count; i+=4)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
      dst.val[i + 2] = v3;
      dst.val[i + 3] = v4;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint8_t, _count> &dst, uint8_t v)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set1_epi8(v);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = v;
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint8_t, _count> &dst, uint8_t v1, uint8_t v2)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi8(v2, v1, v2, v1, v2, v1, v2, v1, v2, v1, v2, v1, v2, v1, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+2<=_count; i+=2)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint8_t, _count> &dst, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi8(v4, v3, v2, v1, v4, v3, v2, v1, v4, v3, v2, v1, v4, v3, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+4<=_count; i+=4)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
      dst.val[i + 2] = v3;
      dst.val[i + 3] = v4;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int16_t, _count> &dst, int16_t v)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set1_epi16(v);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = v;
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int16_t, _count> &dst, int16_t v1, int16_t v2)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi16(v2, v1, v2, v1, v2, v1, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+2<=_count; i+=2)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int16_t, _count> &dst, int16_t v1, int16_t v2, int16_t v3, int16_t v4)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi16(v4, v3, v2, v1, v4, v3, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+4<=_count; i+=4)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
      dst.val[i + 2] = v3;
      dst.val[i + 3] = v4;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint16_t, _count> &dst, uint16_t v)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set1_epi16(v);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = v;
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint16_t, _count> &dst, uint16_t v1, uint16_t v2)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi16(v2, v1, v2, v1, v2, v1, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+2<=_count; i+=2)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint16_t, _count> &dst, uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi16(v4, v3, v2, v1, v4, v3, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+4<=_count; i+=4)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
      dst.val[i + 2] = v3;
      dst.val[i + 3] = v4;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int32_t, _count> &dst, int32_t v)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set1_epi32(v);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = v;
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int32_t, _count> &dst, int32_t v1, int32_t v2)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi32(v2, v1, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+2<=_count; i+=2)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int32_t, _count> &dst, int32_t v1, int32_t v2, int32_t v3, int32_t v4)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi32(v4, v3, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+4<=_count; i+=4)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
      dst.val[i + 2] = v3;
      dst.val[i + 3] = v4;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint32_t, _count> &dst, uint32_t v)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set1_epi32(v);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = v;
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint32_t, _count> &dst, uint32_t v1, uint32_t v2)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi32(v2, v1, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+2<=_count; i+=2)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint32_t, _count> &dst, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi32(v4, v3, v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+4<=_count; i+=4)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
      dst.val[i + 2] = v3;
      dst.val[i + 3] = v4;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int64_t, _count> &dst, int64_t v)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set1_epi64x(v);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = v;
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int64_t, _count> &dst, int64_t v1, int64_t v2)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi64x(v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+2<=_count; i+=2)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<int64_t, _count> &dst, int64_t v1, int64_t v2, int64_t v3, int64_t v4)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi + 2 <= int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi += 2)
    {
      dst.vec[vi    ] = _mm_set_epi64x(v2, v1);
      dst.vec[vi + 1] = _mm_set_epi64x(v4, v3);

      i += (sizeof(dst.vec[0]) / sizeof(dst.val[0])) * 2;
    }
#endif

    for (; i+4<=_count; i+=4)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
      dst.val[i + 2] = v3;
      dst.val[i + 3] = v4;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint64_t, _count> &dst, uint64_t v)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set1_epi64x(v);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = v;
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint64_t, _count> &dst, uint64_t v1, uint64_t v2)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_set_epi64x(v2, v1);

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i+2<=_count; i+=2)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
    }
  }

  template <int _count>
  lxivec_always_inline void set(Ints<uint64_t, _count> &dst, uint64_t v1, uint64_t v2, uint64_t v3, uint64_t v4)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi + 2 <= int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi += 2)
    {
      dst.vec[vi    ] = _mm_set_epi64x(v2, v1);
      dst.vec[vi + 1] = _mm_set_epi64x(v4, v3);

      i += (sizeof(dst.vec[0]) / sizeof(dst.val[0])) * 2;
    }
#endif

    for (; i+4<=_count; i+=4)
    {
      dst.val[i    ] = v1;
      dst.val[i + 1] = v2;
      dst.val[i + 2] = v3;
      dst.val[i + 3] = v4;
    }
  }

} } // End of namespaces
