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
  static lxivec_always_inline void repack(Ints<uint8_t, _count> &dst, const Floats<_count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_packus_epi16(
          _mm_packs_epi32(_mm_cvtps_epi32(src.vec[vi * 4    ]), _mm_cvtps_epi32(src.vec[vi * 4 + 1])),
          _mm_packs_epi32(_mm_cvtps_epi32(src.vec[vi * 4 + 2]), _mm_cvtps_epi32(src.vec[vi * 4 + 3])));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<uint8_t, int32_t>(int32_t(src.val[i]));
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<int8_t, _count> &dst, const Floats<_count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_packs_epi16(
          _mm_packs_epi32(_mm_cvtps_epi32(src.vec[vi * 4    ]), _mm_cvtps_epi32(src.vec[vi * 4 + 1])),
          _mm_packs_epi32(_mm_cvtps_epi32(src.vec[vi * 4 + 2]), _mm_cvtps_epi32(src.vec[vi * 4 + 3])));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<int8_t, int32_t>(int32_t(src.val[i]));
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<uint16_t, _count> &dst, const Floats<_count> &src)
  {
    int i = 0;

#if defined(__SSE4A__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_packus_epi32(_mm_cvtps_epi32(src.vec[vi * 2]), _mm_cvtps_epi32(src.vec[vi * 2 + 1]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#elif defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_add_epi16(
        _mm_packs_epi32(
            _mm_cvtps_epi32(_mm_sub_ps(src.vec[vi * 2    ], _mm_set1_ps(32768.0f))),
            _mm_cvtps_epi32(_mm_sub_ps(src.vec[vi * 2 + 1], _mm_set1_ps(32768.0f)))),
        _mm_set1_epi16(32768));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<uint16_t, int32_t>(int32_t(src.val[i]));
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<int16_t, _count> &dst, const Floats<_count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
      dst.vec[vi] = _mm_packs_epi32(_mm_cvtps_epi32(src.vec[vi * 2]), _mm_cvtps_epi32(src.vec[vi * 2 + 1]));

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<int16_t, int32_t>(int32_t(src.val[i]));
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<uint32_t, _count> &dst, const Floats<_count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = uint32_t(src.val[i]);
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<int32_t, _count> &dst, const Floats<_count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(src.vec) / sizeof(src.vec[0])); vi++)
      dst.vec[vi] = _mm_cvtps_epi32(src.vec[vi]);

    i += sizeof(src.vec) / sizeof(src.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = int32_t(src.val[i]);
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<uint64_t, _count> &dst, const Floats<_count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = uint64_t(src.val[i]);
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<int64_t, _count> &dst, const Floats<_count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = int64_t(src.val[i]);
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<uint8_t, _count> &dst, const Doubles<_count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_packus_epi16(
          _mm_packs_epi32(
              _mm_or_si128(
                  _mm_cvtpd_epi32(src.vec[vi * 8    ]),
                  _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 8 + 1]), 8)),
              _mm_or_si128(
                  _mm_cvtpd_epi32(src.vec[vi * 8 + 2]),
                  _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 8 + 3]), 8))),
          _mm_packs_epi32(
              _mm_or_si128(
                  _mm_cvtpd_epi32(src.vec[vi * 8 + 4]),
                  _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 8 + 5]), 8)),
              _mm_or_si128(
                  _mm_cvtpd_epi32(src.vec[vi * 8 + 6]),
                  _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 8 + 7]), 8))));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<uint8_t, int32_t>(int32_t(src.val[i]));
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<int8_t, _count> &dst, const Doubles<_count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_packs_epi16(
          _mm_packs_epi32(
              _mm_or_si128(
                  _mm_cvtpd_epi32(src.vec[vi * 8    ]),
                  _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 8 + 1]), 8)),
              _mm_or_si128(
                  _mm_cvtpd_epi32(src.vec[vi * 8 + 2]),
                  _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 8 + 3]), 8))),
          _mm_packs_epi32(
              _mm_or_si128(
                  _mm_cvtpd_epi32(src.vec[vi * 8 + 4]),
                  _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 8 + 5]), 8)),
              _mm_or_si128(
                  _mm_cvtpd_epi32(src.vec[vi * 8 + 6]),
                  _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 8 + 7]), 8))));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<int8_t, int32_t>(int32_t(src.val[i]));
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<uint16_t, _count> &dst, const Doubles<_count> &src)
  {
    int i = 0;

//#if defined(__SSE4A__)
//    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
//    {
//      dst.vec[vi] = _mm_packus_epi32(
//          _mm_or_si128(
//              _mm_cvtpd_epi32(src.vec[vi * 4]),
//              _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 4 + 1]), 8)),
//          _mm_or_si128(
//              _mm_cvtpd_epi32(src.vec[vi * 4 + 2]),
//              _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 4 + 3]), 8)));
//    }
//
//    i += sizeof(dst.vec) / sizeof(dst.val[0]);
//#elif defined(__SSE2__)
//    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
//    {
//      dst.vec[vi] = _mm_add_epi16(
//        _mm_packs_epi32(
//            _mm_or_si128(
//                _mm_cvtpd_epi32(_mm_sub_pd(src.vec[vi * 4], _mm_set1_pd(32768.0))),
//                _mm_slli_si128(_mm_cvtpd_epi32(_mm_sub_pd(src.vec[vi * 4 + 1], _mm_set1_pd(32768.0))), 8)),
//            _mm_or_si128(
//                _mm_cvtpd_epi32(_mm_sub_pd(src.vec[vi * 4 + 2], _mm_set1_pd(32768.0))),
//                _mm_slli_si128(_mm_cvtpd_epi32(_mm_sub_pd(src.vec[vi * 4 + 3], _mm_set1_pd(32768.0))), 8))),
//        _mm_set1_epi16(32768));
//    }
//
//    i += sizeof(dst.vec) / sizeof(dst.val[0]);
//#endif

    for (; i<_count; i++)
      dst.val[i] = uint16_t(src.val[i]);
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<int16_t, _count> &dst, const Doubles<_count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_packs_epi32(
          _mm_or_si128(
              _mm_cvtpd_epi32(src.vec[vi * 4]),
              _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 4 + 1]), 8)),
          _mm_or_si128(
              _mm_cvtpd_epi32(src.vec[vi * 4 + 2]),
              _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 4 + 3]), 8)));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = saturate<int16_t, int32_t>(int32_t(src.val[i]));
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<uint32_t, _count> &dst, const Doubles<_count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = uint32_t(src.val[i]);
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<int32_t, _count> &dst, const Doubles<_count> &src)
  {
    int i = 0;

#if defined(__SSE2__)
    for (int vi = 0; vi < int(sizeof(dst.vec) / sizeof(dst.vec[0])); vi++)
    {
      dst.vec[vi] = _mm_or_si128(
          _mm_cvtpd_epi32(src.vec[vi * 2]),
          _mm_slli_si128(_mm_cvtpd_epi32(src.vec[vi * 2 + 1]), 8));
    }

    i += sizeof(dst.vec) / sizeof(dst.val[0]);
#endif

    for (; i<_count; i++)
      dst.val[i] = int32_t(src.val[i]);
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<uint64_t, _count> &dst, const Doubles<_count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = uint64_t(src.val[i]);
  }

  template <int _count>
  static lxivec_always_inline void repack(Ints<int64_t, _count> &dst, const Doubles<_count> &src)
  {
    int i = 0;

    for (; i<_count; i++)
      dst.val[i] = int64_t(src.val[i]);
  }

} } // End of namespaces
