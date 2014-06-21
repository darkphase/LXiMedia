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
#ifndef LXIVECINTRIN_VECBOOL_H
#define LXIVECINTRIN_VECBOOL_H

#include "intrinbool.h"

namespace lxivec {
namespace _private {

template <int _size, int _count>
struct BoolVector : VecObject
{
  lxivec_always_inline BoolVector(void)
    : VecObject()
  {
  }

  lxivec_always_inline BoolVector(
      const BoolVector<_size, _count> &from)
    : VecObject(from)
  {
    copy(data, from.data);
  }

  lxivec_always_inline BoolVector<_size, _count> & operator=(
      const BoolVector<_size, _count> &from)
  {
    copy(data, from.data);
    return *this;
  }

  lxivec_always_inline BoolVector<_size, _count> operator||(
      const BoolVector<_size, _count> &b) const
  {
    BoolVector<_size, _count> r;
    bor(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<_size, _count> operator&&(
      const BoolVector<_size, _count> &b) const
  {
    BoolVector<_size, _count> r;
    band(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<_size, _count> operator~(void) const
  {
    BoolVector<_size, _count> r;
    bnot(r.data, data);
    return r;
  }

  lxivec_always_inline BoolVector<_size, _count> operator==(
      const BoolVector<_size, _count> &b) const
  {
    BoolVector<_size, _count> r;
    cmpeq(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<_size, _count> operator!=(
      const BoolVector<_size, _count> &b) const
  {
    BoolVector<_size, _count> r;
    cmpne(r.data, data, b.data);
    return r;
  }

  static const int count = _count;
  Bits<_size, _count> data;
};

} // End of namespace

typedef _private::BoolVector<1, 1 * vecsize / 1>  bool8_v;
typedef _private::BoolVector<2, 1 * vecsize / 2>  bool16_v;
typedef _private::BoolVector<4, 1 * vecsize / 4>  bool32_v;
typedef _private::BoolVector<8, 1 * vecsize / 8>  bool64_v;

typedef _private::BoolVector<1, 2 * vecsize / 1>  bool8_dv;
typedef _private::BoolVector<2, 2 * vecsize / 2>  bool16_dv;
typedef _private::BoolVector<4, 2 * vecsize / 4>  bool32_dv;
typedef _private::BoolVector<8, 2 * vecsize / 8>  bool64_dv;

typedef _private::BoolVector<1, 4 * vecsize / 1>  bool8_qv;
typedef _private::BoolVector<2, 4 * vecsize / 2>  bool16_qv;
typedef _private::BoolVector<4, 4 * vecsize / 4>  bool32_qv;
typedef _private::BoolVector<8, 4 * vecsize / 8>  bool64_qv;

typedef _private::BoolVector<1, 8 * vecsize / 1>  bool8_ov;
typedef _private::BoolVector<2, 8 * vecsize / 2>  bool16_ov;
typedef _private::BoolVector<4, 8 * vecsize / 4>  bool32_ov;
typedef _private::BoolVector<8, 8 * vecsize / 8>  bool64_ov;

typedef _private::BoolVector<1, 16 * vecsize / 1> bool8_hv;
typedef _private::BoolVector<2, 16 * vecsize / 2> bool16_hv;
typedef _private::BoolVector<4, 16 * vecsize / 4> bool32_hv;
typedef _private::BoolVector<8, 16 * vecsize / 8> bool64_hv;

} // End of namespace

#endif
