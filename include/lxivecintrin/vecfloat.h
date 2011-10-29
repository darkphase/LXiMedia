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
#ifndef LXIVECINTRIN_VECFLOAT_H
#define LXIVECINTRIN_VECFLOAT_H

#include "intrinfloat.h"

namespace lxivec {
namespace _private {

template <int _size, int _count> struct BoolVector;
template <typename _type, int _count> struct IntVector;
template <int _count> struct DoubleVector;

template <int _count>
struct FloatVector : VecObject
{
  lxivec_always_inline FloatVector(void)
    : VecObject()
  {
  }

  lxivec_always_inline FloatVector(
      const FloatVector<_count> &from)
    : VecObject(from)
  {
    copy(data, from.data);
  }

  lxivec_always_inline FloatVector(
      const DoubleVector<_count> &from)
    : VecObject()
  {
    repack(data, from.data);
  }

  template <typename _fromtype>
  lxivec_always_inline FloatVector(
      const IntVector<_fromtype, _count> &from)
    : VecObject()
  {
    repack(data, from.data);
  }

  static lxivec_always_inline FloatVector<_count> load(
      const float *p)
  {
    FloatVector<_count> r;
    _private::load(r.data, p);
    return r;
  }

  friend lxivec_always_inline void store(
      float *p,
      const FloatVector<_count> &a,
      int max = _count)
  {
    _private::store(p, a.data, max);
  }

  friend lxivec_always_inline FloatVector<_count> add(
      const FloatVector<_count> &a,
      const FloatVector<_count> &b)
  {
    FloatVector<_count> r;
    add(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> add(
      const FloatVector<_count> &a,
      float b)
  {
    FloatVector<_count> r;
    add(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> sub(
      const FloatVector<_count> &a,
      const FloatVector<_count> &b)
  {
    FloatVector<_count> r;
    sub(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> sub(
      const FloatVector<_count> &a,
      float b)
  {
    FloatVector<_count> r;
    sub(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> mul(
      const FloatVector<_count> &a,
      const FloatVector<_count> &b)
  {
    FloatVector<_count> r;
    mul(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> mul(
      const FloatVector<_count> &a,
      float b)
  {
    FloatVector<_count> r;
    mul(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> div(
      const FloatVector<_count> &a,
      const FloatVector<_count> &b)
  {
    FloatVector<_count> r;
    div(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> div(
      const FloatVector<_count> &a,
      float b)
  {
    FloatVector<_count> r;
    div(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> abs(
      const FloatVector<_count> &x)
  {
    FloatVector<_count> r;
    abs(r.data, x.data);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> min(
      const FloatVector<_count> &a,
      const FloatVector<_count> &b)
  {
    FloatVector<_count> r;
    min(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> max(
      const FloatVector<_count> &a,
      const FloatVector<_count> &b)
  {
    FloatVector<_count> r;
    max(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline FloatVector<_count> select(
      const BoolVector<4, _count> &m,
      const FloatVector<_count> &a,
      const FloatVector<_count> &b)
  {
    FloatVector<_count> r;
    select(r.data, m.data, a.data, b.data);
    return r;
  }

  lxivec_always_inline FloatVector<_count> & operator=(
      const FloatVector<_count> &from)
  {
    copy(data, from.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> & operator=(
      const DoubleVector<_count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  template <typename _fromtype>
  lxivec_always_inline FloatVector<_count> & operator=(
      const IntVector<_fromtype, _count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  template <typename _fromtype>
  lxivec_always_inline FloatVector<_count> & operator=(
      const DoubleVector<_count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator+(
      const FloatVector<_count> &b) const
  {
    return add(*this, b);
  }

  lxivec_always_inline FloatVector<_count> & operator+=(
      const FloatVector<_count> &b)
  {
    add(data, data, b.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator+(
      float b) const
  {
    return add(*this, b);
  }

  lxivec_always_inline FloatVector<_count> & operator+=(
      float b)
  {
    add(data, data, b);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator-(
      const FloatVector<_count> &b) const
  {
    return sub(*this, b);
  }

  lxivec_always_inline FloatVector<_count> & operator-=(
      const FloatVector<_count> &b)
  {
    sub(data, data, b.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator-(
      float b) const
  {
    return sub(*this, b);
  }

  lxivec_always_inline FloatVector<_count> & operator-=(
      float b)
  {
    sub(data, data, b);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator*(
      const FloatVector<_count> &b) const
  {
    return mul(*this, b);
  }

  lxivec_always_inline FloatVector<_count> & operator*=(
      const FloatVector<_count> &b)
  {
    mul(data, data, b.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator*(
      float b) const
  {
    return mul(*this, b);
  }

  lxivec_always_inline FloatVector<_count> & operator*=(
      float b)
  {
    mul(data, data, b);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator/(
      const FloatVector<_count> &b) const
  {
    return div(*this, b);
  }

  lxivec_always_inline FloatVector<_count> & operator/=(
      const FloatVector<_count> &b)
  {
    div(data, data, b.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator/(
      float b) const
  {
    return div(*this, b);
  }

  lxivec_always_inline FloatVector<_count> & operator/=(
      float b)
  {
    div(data, data, b);
    return *this;
  }

  lxivec_always_inline BoolVector<4, _count> operator==(
      const FloatVector<_count> &b) const
  {
    BoolVector<4, _count> r;
    cmpeq(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<4, _count> operator>(
      const FloatVector<_count> &b) const
  {
    BoolVector<4, _count> r;
    cmpgt(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<4, _count> operator<(
      const FloatVector<_count> &b) const
  {
    BoolVector<4, _count> r;
    cmplt(r.data, data, b.data);
    return r;
  }

  typedef float single;
  static const int count = _count;
  Floats<_count> data;
};

template <int _count>
struct DoubleVector : VecObject
{
  lxivec_always_inline DoubleVector(void)
    : VecObject()
  {
  }

  lxivec_always_inline DoubleVector(
      const DoubleVector<_count> &from)
    : VecObject(from)
  {
    copy(data, from.data);
  }

  lxivec_always_inline DoubleVector(
      const FloatVector<_count> &from)
    : VecObject()
  {
    repack(data, from.data);
  }

  template <typename _fromtype>
  lxivec_always_inline DoubleVector(
      const IntVector<_fromtype, _count> &from)
    : VecObject()
  {
    repack(data, from.data);
  }

  static lxivec_always_inline DoubleVector<_count> load(
      const double *p)
  {
    DoubleVector<_count> r;
    _private::load(r.data, p);
    return r;
  }

  friend lxivec_always_inline void store(
      double *p,
      const DoubleVector<_count> &a,
      int max = _count)
  {
    _private::store(p, a.data, max);
  }

  friend lxivec_always_inline DoubleVector<_count> add(
      const DoubleVector<_count> &a,
      const DoubleVector<_count> &b)
  {
    DoubleVector<_count> r;
    add(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> add(
      const DoubleVector<_count> &a,
      float b)
  {
    DoubleVector<_count> r;
    add(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> sub(
      const DoubleVector<_count> &a,
      const DoubleVector<_count> &b)
  {
    DoubleVector<_count> r;
    sub(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> sub(
      const DoubleVector<_count> &a,
      float b)
  {
    DoubleVector<_count> r;
    sub(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> mul(
      const DoubleVector<_count> &a,
      const DoubleVector<_count> &b)
  {
    DoubleVector<_count> r;
    mul(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> mul(
      const DoubleVector<_count> &a,
      float b)
  {
    DoubleVector<_count> r;
    mul(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> div(
      const DoubleVector<_count> &a,
      const DoubleVector<_count> &b)
  {
    DoubleVector<_count> r;
    div(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> div(
      const DoubleVector<_count> &a,
      float b)
  {
    DoubleVector<_count> r;
    div(r.data, a.data, b);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> abs(
      const DoubleVector<_count> &x)
  {
    DoubleVector<_count> r;
    abs(r.data, x.data);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> min(
      const DoubleVector<_count> &a,
      const DoubleVector<_count> &b)
  {
    DoubleVector<_count> r;
    min(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> max(
      const DoubleVector<_count> &a,
      const DoubleVector<_count> &b)
  {
    DoubleVector<_count> r;
    max(r.data, a.data, b.data);
    return r;
  }

  friend lxivec_always_inline DoubleVector<_count> select(
      const BoolVector<8, _count> &m,
      const DoubleVector<_count> &a,
      const DoubleVector<_count> &b)
  {
    DoubleVector<_count> r;
    select(r.data, m.data, a.data, b.data);
    return r;
  }

  lxivec_always_inline DoubleVector<_count> & operator=(
      const DoubleVector<_count> &from)
  {
    copy(data, from.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> & operator=(
      const FloatVector<_count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  template <typename _fromtype>
  lxivec_always_inline DoubleVector<_count> & operator=(
      const IntVector<_fromtype, _count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  template <typename _fromtype>
  lxivec_always_inline DoubleVector<_count> & operator=(
      const DoubleVector<_count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator+(
      const DoubleVector<_count> &b) const
  {
    return add(*this, b);
  }

  lxivec_always_inline DoubleVector<_count> & operator+=(
      const DoubleVector<_count> &b)
  {
    add(data, data, b.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator+(
      float b) const
  {
    return add(*this, b);
  }

  lxivec_always_inline DoubleVector<_count> & operator+=(
      float b)
  {
    add(data, data, b);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator-(
      const DoubleVector<_count> &b) const
  {
    return sub(*this, b);
  }

  lxivec_always_inline DoubleVector<_count> & operator-=(
      const DoubleVector<_count> &b)
  {
    sub(data, data, b.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator-(
      float b) const
  {
    return sub(*this, b);
  }

  lxivec_always_inline DoubleVector<_count> & operator-=(
      float b)
  {
    sub(data, data, b);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator*(
      const DoubleVector<_count> &b) const
  {
    return mul(*this, b);
  }

  lxivec_always_inline DoubleVector<_count> & operator*=(
      const DoubleVector<_count> &b)
  {
    mul(data, data, b.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator*(
      float b) const
  {
    return mul(*this, b);
  }

  lxivec_always_inline DoubleVector<_count> & operator*=(
      float b)
  {
    mul(data, data, b);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator/(
      const DoubleVector<_count> &b) const
  {
    return div(*this, b);
  }

  lxivec_always_inline DoubleVector<_count> & operator/=(
      const DoubleVector<_count> &b)
  {
    div(data, data, b.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator/(
      float b) const
  {
    return div(*this, b);
  }

  lxivec_always_inline DoubleVector<_count> & operator/=(
      float b)
  {
    div(data, data, b);
    return *this;
  }

  lxivec_always_inline BoolVector<8, _count> operator==(
      const DoubleVector<_count> &b) const
  {
    BoolVector<8, _count> r;
    cmpeq(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<8, _count> operator>(
      const DoubleVector<_count> &b) const
  {
    BoolVector<8, _count> r;
    cmpgt(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<8, _count> operator<(
      const DoubleVector<_count> &b) const
  {
    BoolVector<8, _count> r;
    cmplt(r.data, data, b.data);
    return r;
  }

  typedef double single;
  static const int count = _count;
  Doubles<_count> data;
};

} // End of namespace

typedef _private::FloatVector <1 * vecsize / sizeof(float)>   float_v;
typedef _private::DoubleVector<1 * vecsize / sizeof(double)>  double_v;

typedef _private::FloatVector <2 * vecsize / sizeof(float)>   float_dv;
typedef _private::DoubleVector<2 * vecsize / sizeof(double)>  double_dv;

typedef _private::FloatVector <4 * vecsize / sizeof(float)>   float_qv;
typedef _private::DoubleVector<4 * vecsize / sizeof(double)>  double_qv;

typedef _private::FloatVector <8 * vecsize / sizeof(float)>   float_ov;
typedef _private::DoubleVector<8 * vecsize / sizeof(double)>  double_ov;

typedef _private::FloatVector <16 * vecsize / sizeof(float)>  float_hv;
typedef _private::DoubleVector<16 * vecsize / sizeof(double)> double_hv;

} // End of namespace

#endif
