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
#ifndef LXIVECINTRIN_VECINT_H
#define LXIVECINTRIN_VECINT_H

#include "intrinint.h"

namespace lxivec {
namespace _private {

template <int _size, int _count> struct BoolVector;
template <int _count> struct FloatVector;
template <int _count> struct DoubleVector;

template <typename _type, int _count>
struct IntVector : VecObject
{
  lxivec_always_inline IntVector(void)
    : VecObject()
  {
  }

  lxivec_always_inline IntVector(
      const IntVector<_type, _count> &from)
    : VecObject(from)
  {
    copy(data, from.data);
  }

  template <typename _fromtype>
  lxivec_always_inline IntVector(
      const IntVector<_fromtype, _count> &from)
    : VecObject()
  {
    repack(data, from.data);
  }

  lxivec_always_inline IntVector(
      const FloatVector<_count> &from)
    : VecObject()
  {
    repack(data, from.data);
  }

  lxivec_always_inline IntVector(
      const DoubleVector<_count> &from)
    : VecObject()
  {
    repack(data, from.data);
  }

  static lxivec_always_inline IntVector<_type, _count> load(
      const _type *p,
      int max = _count)
  {
    IntVector<_type, _count> r;
    _private::load(r.data, p, max);
    return r;
  }

  friend lxivec_always_inline void store(
      _type *p,
      const IntVector<_type, _count> &a,
      int max = _count)
  {
    _private::store(p, a.data, max);
  }

  static lxivec_always_inline IntVector<_type, _count> loadu(
      const _type *p,
      int max = _count)
  {
    IntVector<_type, _count> r;
    _private::loadu(r.data, p, max);
    return r;
  }

  friend lxivec_always_inline void storeu(
      _type *p,
      const IntVector<_type, _count> &a,
      int max = _count)
  {
    _private::storeu(p, a.data, max);
  }

  static lxivec_always_inline IntVector<_type, _count> set(
      _type v)
  {
    IntVector<_type, _count> r;
    _private::set(r.data, v);
    return r;
  }

  static lxivec_always_inline IntVector<_type, _count> set(
      _type v1, _type v2)
  {
    IntVector<_type, _count> r;
    _private::set(r.data, v1, v2);
    return r;
  }

  static lxivec_always_inline IntVector<_type, _count> set(
      _type v1, _type v2, _type v3, _type v4)
  {
    IntVector<_type, _count> r;
    _private::set(r.data, v1, v2, v3, v4);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count * 2> interleave(
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &b)
  {
    IntVector<_type, _count * 2> r;
    interleave(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> add(
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &b)
  {
    IntVector<_type, _count> r;
    add(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> add(
      const IntVector<_type, _count> &a,
      _type b)
  {
    IntVector<_type, _count> r;
    add(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> adds(
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &b)
  {
    IntVector<_type, _count> r;
    adds(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> adds(
      const IntVector<_type, _count> &a,
      _type b)
  {
    IntVector<_type, _count> r;
    adds(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count / 2> hadd(
      const IntVector<_type, _count> &a)
  {
    IntVector<_type, _count / 2> r;
    hadd(r.data, a.data);
    return r;
  }

  friend lxivec_always_inline _type hsum(
      const IntVector<_type, _count> &a)
  {
    return hsum(a.data);
  }

  friend lxivec_always_inline IntVector<_type, _count> sub(
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &b)
  {
    IntVector<_type, _count> r;
    sub(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> sub(
      const IntVector<_type, _count> &a,
      _type b)
  {
    IntVector<_type, _count> r;
    sub(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> subs(
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &b)
  {
    IntVector<_type, _count> r;
    subs(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> subs(
      const IntVector<_type, _count> &a,
      _type b)
  {
    IntVector<_type, _count> r;
    subs(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> mul(
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &b)
  {
    IntVector<_type, _count> r;
    mul(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> mul(
      const IntVector<_type, _count> &a,
      _type b)
  {
    IntVector<_type, _count> r;
    mul(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> abs(
      const IntVector<_type, _count> &a)
  {
    IntVector<_type, _count> r;
    abs(r.data, a.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> min(
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &b)
  {
    IntVector<_type, _count> r;
    min(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> max(
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &b)
  {
    IntVector<_type, _count> r;
    max(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> bound(
      const IntVector<_type, _count> &low,
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &high)
  {
    IntVector<_type, _count> r;
    max(r.data, low.data, a.data);
    min(r.data, high.data, r.data);
    return r;
  }

  friend lxivec_always_inline IntVector<_type, _count> select(
      const BoolVector<sizeof(_type), _count> &m,
      const IntVector<_type, _count> &a,
      const IntVector<_type, _count> &b)
  {
    IntVector<_type, _count> r;
    select(r.data, m.data, a.data, b.data);
    return r;
  }

  template <typename _astype>
  lxivec_always_inline IntVector<typename _astype::single, _count * sizeof(_type) / sizeof(typename _astype::single)> & as(void)
  {
    return *reinterpret_cast<IntVector<typename _astype::single, _count * sizeof(_type) / sizeof(typename _astype::single)> *>(this);
  }

  template <typename _astype>
  lxivec_always_inline const IntVector<typename _astype::single, _count * sizeof(_type) / sizeof(typename _astype::single)> & as(void) const
  {
    return *reinterpret_cast<const IntVector<typename _astype::single, _count * sizeof(_type) / sizeof(typename _astype::single)> *>(this);
  }

  lxivec_always_inline IntVector<_type, _count> & operator=(
      const IntVector<_type, _count> &from)
  {
    copy(data, from.data);
    return *this;
  }

  template <typename _fromtype>
  lxivec_always_inline IntVector<_type, _count> & operator=(
      const IntVector<_fromtype, _count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator+(
      const IntVector<_type, _count> &b) const
  {
    return add(*this, b);
  }

  lxivec_always_inline IntVector<_type, _count> & operator+=(
      const IntVector<_type, _count> &b)
  {
    add(data, data, b.data);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator+(
      _type b) const
  {
    return add(*this, b);
  }

  lxivec_always_inline IntVector<_type, _count> & operator+=(
      _type b)
  {
    add(data, data, b);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator-(
      const IntVector<_type, _count> &b) const
  {
    return sub(*this, b);
  }

  lxivec_always_inline IntVector<_type, _count> & operator-=(
      const IntVector<_type, _count> &b)
  {
    sub(data, data, b.data);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator-(
      _type b) const
  {
    return sub(*this, b);
  }

  lxivec_always_inline IntVector<_type, _count> & operator-=(
      _type b)
  {
    sub(data, data, b);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator*(
      const IntVector<_type, _count> &b) const
  {
    return mul(*this, b);
  }

  lxivec_always_inline IntVector<_type, _count> & operator*=(
      const IntVector<_type, _count> &b)
  {
    mul(data, data, b.data);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator*(
      _type b) const
  {
    return mul(*this, b);
  }

  lxivec_always_inline IntVector<_type, _count> & operator*=(
      _type b)
  {
    mul(data, data, b);
    return *this;
  }

  lxivec_always_inline BoolVector<sizeof(_type), _count> operator==(
      const IntVector<_type, _count> &b) const
  {
    BoolVector<sizeof(_type), _count> r;
    cmpeq(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<sizeof(_type), _count> operator>(
      const IntVector<_type, _count> &b) const
  {
    BoolVector<sizeof(_type), _count> r;
    cmpgt(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<sizeof(_type), _count> operator<(
      const IntVector<_type, _count> &b) const
  {
    BoolVector<sizeof(_type), _count> r;
    cmplt(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline IntVector<_type, _count> operator|(
      const IntVector<_type, _count> &b) const
  {
    IntVector<_type, _count> r;
    bor(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline IntVector<_type, _count> & operator|=(
      const IntVector<_type, _count> &b)
  {
    bor(data, data, b.data);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator|(
      _type b) const
  {
    IntVector<_type, _count> r;
    bor(r.data, data, b);
    return r;
  }

  lxivec_always_inline IntVector<_type, _count> & operator|=(
      _type b)
  {
    bor(data, data, b);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator&(
      const IntVector<_type, _count> &b) const
  {
    IntVector<_type, _count> r;
    band(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline IntVector<_type, _count> & operator&=(
      const IntVector<_type, _count> &b)
  {
    band(data, data, b.data);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator&(
      _type b) const
  {
    IntVector<_type, _count> r;
    band(r.data, data, b);
    return r;
  }

  lxivec_always_inline IntVector<_type, _count> & operator&=(
      _type b)
  {
    band(data, data, b);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator>>(
      int b) const
  {
    IntVector<_type, _count> r;
    shr(r.data, data, b);
    return r;
  }

  lxivec_always_inline IntVector<_type, _count> & operator>>=(
      int b)
  {
    shr(data, data, b);
    return *this;
  }

  lxivec_always_inline IntVector<_type, _count> operator<<(
      int b) const
  {
    IntVector<_type, _count> r;
    shl(r.data, data, b);
    return r;
  }

  lxivec_always_inline IntVector<_type, _count> & operator<<=(
      int b)
  {
    shl(data, data, b);
    return *this;
  }

  typedef _type single;
  static const int count = _count;
  Ints<_type, _count> data;
};

} // End of namespace

typedef _private::IntVector<uint8_t,  1 * vecsize / sizeof(uint8_t)>    uint8_v;
typedef _private::IntVector<int8_t,   1 * vecsize / sizeof(int8_t)>     int8_v;
typedef _private::IntVector<uint16_t, 1 * vecsize / sizeof(uint16_t)>   uint16_v;
typedef _private::IntVector<int16_t,  1 * vecsize / sizeof(int16_t)>    int16_v;
typedef _private::IntVector<uint32_t, 1 * vecsize / sizeof(uint32_t)>   uint32_v;
typedef _private::IntVector<int32_t,  1 * vecsize / sizeof(int32_t)>    int32_v;
typedef _private::IntVector<uint64_t, 1 * vecsize / sizeof(uint64_t)>   uint64_v;
typedef _private::IntVector<int64_t,  1 * vecsize / sizeof(int64_t)>    int64_v;

typedef _private::IntVector<uint8_t,  2 * vecsize / sizeof(uint8_t)>    uint8_dv;
typedef _private::IntVector<int8_t,   2 * vecsize / sizeof(int8_t)>     int8_dv;
typedef _private::IntVector<uint16_t, 2 * vecsize / sizeof(uint16_t)>   uint16_dv;
typedef _private::IntVector<int16_t,  2 * vecsize / sizeof(int16_t)>    int16_dv;
typedef _private::IntVector<uint32_t, 2 * vecsize / sizeof(uint32_t)>   uint32_dv;
typedef _private::IntVector<int32_t,  2 * vecsize / sizeof(int32_t)>    int32_dv;
typedef _private::IntVector<uint64_t, 2 * vecsize / sizeof(uint64_t)>   uint64_dv;
typedef _private::IntVector<int64_t,  2 * vecsize / sizeof(int64_t)>    int64_dv;

typedef _private::IntVector<uint8_t,  4 * vecsize / sizeof(uint8_t)>    uint8_qv;
typedef _private::IntVector<int8_t,   4 * vecsize / sizeof(int8_t)>     int8_qv;
typedef _private::IntVector<uint16_t, 4 * vecsize / sizeof(uint16_t)>   uint16_qv;
typedef _private::IntVector<int16_t,  4 * vecsize / sizeof(int16_t)>    int16_qv;
typedef _private::IntVector<uint32_t, 4 * vecsize / sizeof(uint32_t)>   uint32_qv;
typedef _private::IntVector<int32_t,  4 * vecsize / sizeof(int32_t)>    int32_qv;
typedef _private::IntVector<uint64_t, 4 * vecsize / sizeof(uint64_t)>   uint64_qv;
typedef _private::IntVector<int64_t,  4 * vecsize / sizeof(int64_t)>    int64_qv;

typedef _private::IntVector<uint8_t,  8 * vecsize / sizeof(uint8_t)>    uint8_ov;
typedef _private::IntVector<int8_t,   8 * vecsize / sizeof(int8_t)>     int8_ov;
typedef _private::IntVector<uint16_t, 8 * vecsize / sizeof(uint16_t)>   uint16_ov;
typedef _private::IntVector<int16_t,  8 * vecsize / sizeof(int16_t)>    int16_ov;
typedef _private::IntVector<uint32_t, 8 * vecsize / sizeof(uint32_t)>   uint32_ov;
typedef _private::IntVector<int32_t,  8 * vecsize / sizeof(int32_t)>    int32_ov;
typedef _private::IntVector<uint64_t, 8 * vecsize / sizeof(uint64_t)>   uint64_ov;
typedef _private::IntVector<int64_t,  8 * vecsize / sizeof(int64_t)>    int64_ov;

typedef _private::IntVector<uint8_t,  16 * vecsize / sizeof(uint8_t)>   uint8_hv;
typedef _private::IntVector<int8_t,   16 * vecsize / sizeof(int8_t)>    int8_hv;
typedef _private::IntVector<uint16_t, 16 * vecsize / sizeof(uint16_t)>  uint16_hv;
typedef _private::IntVector<int16_t,  16 * vecsize / sizeof(int16_t)>   int16_hv;
typedef _private::IntVector<uint32_t, 16 * vecsize / sizeof(uint32_t)>  uint32_hv;
typedef _private::IntVector<int32_t,  16 * vecsize / sizeof(int32_t)>   int32_hv;
typedef _private::IntVector<uint64_t, 16 * vecsize / sizeof(uint64_t)>  uint64_hv;
typedef _private::IntVector<int64_t,  16 * vecsize / sizeof(int64_t)>   int64_hv;

} // End of namespace

#endif
