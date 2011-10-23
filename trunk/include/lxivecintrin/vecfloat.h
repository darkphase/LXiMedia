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
  {
  }

  lxivec_always_inline FloatVector(const FloatVector<_count> &from)
  {
    copy(data, from.data);
  }

  lxivec_always_inline FloatVector(const DoubleVector<_count> &from)
  {
    repack(data, from.data);
  }

  template <typename _fromtype>
  lxivec_always_inline FloatVector(const IntVector<_fromtype, _count> &from)
  {
    repack(data, from.data);
  }

  lxivec_always_inline FloatVector<_count> & operator=(const FloatVector<_count> &from)
  {
    copy(data, from.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> & operator=(const DoubleVector<_count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  template <typename _fromtype>
  lxivec_always_inline FloatVector<_count> & operator=(const IntVector<_fromtype, _count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  template <typename _fromtype>
  lxivec_always_inline FloatVector<_count> & operator=(const DoubleVector<_count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  static lxivec_always_inline FloatVector<_count> load(const float *p)
  {
    FloatVector<_count> r;
    _private::load(r.data, p);
    return r;
  }

  lxivec_always_inline void store(float *p) const
  {
    _private::store(p, data);
  }

  lxivec_always_inline FloatVector<_count> operator+(const FloatVector<_count> &b) const
  {
    FloatVector<_count> r;
    add(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline FloatVector<_count> & operator+=(const FloatVector<_count> &b)
  {
    add(data, data, b.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator+(float b) const
  {
    FloatVector<_count> r;
    add(r.data, data, b);
    return r;
  }

  lxivec_always_inline FloatVector<_count> & operator+=(float b)
  {
    add(data, data, b);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator-(const FloatVector<_count> &b) const
  {
    FloatVector<_count> r;
    sub(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline FloatVector<_count> & operator-=(const FloatVector<_count> &b)
  {
    sub(data, data, b.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator-(float b) const
  {
    FloatVector<_count> r;
    sub(r.data, data, b);
    return r;
  }

  lxivec_always_inline FloatVector<_count> & operator-=(float b)
  {
    sub(data, data, b);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator*(const FloatVector<_count> &b) const
  {
    FloatVector<_count> r;
    mul(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline FloatVector<_count> & operator*=(const FloatVector<_count> &b)
  {
    mul(data, data, b.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator*(float b) const
  {
    FloatVector<_count> r;
    mul(r.data, data, b);
    return r;
  }

  lxivec_always_inline FloatVector<_count> & operator*=(float b)
  {
    mul(data, data, b);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator/(const FloatVector<_count> &b) const
  {
    FloatVector<_count> r;
    div(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline FloatVector<_count> & operator/=(const FloatVector<_count> &b)
  {
    div(data, data, b.data);
    return *this;
  }

  lxivec_always_inline FloatVector<_count> operator/(float b) const
  {
    FloatVector<_count> r;
    div(r.data, data, b);
    return r;
  }

  lxivec_always_inline FloatVector<_count> & operator/=(float b)
  {
    div(data, data, b);
    return *this;
  }

  lxivec_always_inline BoolVector<4, _count> operator==(const FloatVector<_count> &b) const
  {
    BoolVector<4, _count> r;
    cmpeq(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<4, _count> operator>(const FloatVector<_count> &b) const
  {
    BoolVector<4, _count> r;
    cmpgt(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<4, _count> operator<(const FloatVector<_count> &b) const
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
struct DoubleVector
{
  lxivec_always_inline DoubleVector(void)
  {
  }

  lxivec_always_inline DoubleVector(const DoubleVector<_count> &from)
  {
    copy(data, from.data);
  }

  lxivec_always_inline DoubleVector(const FloatVector<_count> &from)
  {
    repack(data, from.data);
  }

  template <typename _fromtype>
  lxivec_always_inline DoubleVector(const IntVector<_fromtype, _count> &from)
  {
    repack(data, from.data);
  }

  lxivec_always_inline DoubleVector<_count> & operator=(const DoubleVector<_count> &from)
  {
    copy(data, from.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> & operator=(const FloatVector<_count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  template <typename _fromtype>
  lxivec_always_inline DoubleVector<_count> & operator=(const IntVector<_fromtype, _count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  template <typename _fromtype>
  lxivec_always_inline DoubleVector<_count> & operator=(const DoubleVector<_count> &from)
  {
    repack(data, from.data);
    return *this;
  }

  static lxivec_always_inline DoubleVector<_count> load(const double *p)
  {
    DoubleVector<_count> r;
    _private::load(r.data, p);
    return r;
  }

  lxivec_always_inline void store(double *p) const
  {
    _private::store(p, data);
  }

  lxivec_always_inline DoubleVector<_count> operator+(const DoubleVector<_count> &b) const
  {
    DoubleVector<_count> r;
    add(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline DoubleVector<_count> & operator+=(const DoubleVector<_count> &b)
  {
    add(data, data, b.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator+(float b) const
  {
    DoubleVector<_count> r;
    add(r.data, data, b);
    return r;
  }

  lxivec_always_inline DoubleVector<_count> & operator+=(float b)
  {
    add(data, data, b);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator-(const DoubleVector<_count> &b) const
  {
    DoubleVector<_count> r;
    sub(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline DoubleVector<_count> & operator-=(const DoubleVector<_count> &b)
  {
    sub(data, data, b.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator-(float b) const
  {
    DoubleVector<_count> r;
    sub(r.data, data, b);
    return r;
  }

  lxivec_always_inline DoubleVector<_count> & operator-=(float b)
  {
    sub(data, data, b);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator*(const DoubleVector<_count> &b) const
  {
    DoubleVector<_count> r;
    mul(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline DoubleVector<_count> & operator*=(const DoubleVector<_count> &b)
  {
    mul(data, data, b.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator*(float b) const
  {
    DoubleVector<_count> r;
    mul(r.data, data, b);
    return r;
  }

  lxivec_always_inline DoubleVector<_count> & operator*=(float b)
  {
    mul(data, data, b);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator/(const DoubleVector<_count> &b) const
  {
    DoubleVector<_count> r;
    div(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline DoubleVector<_count> & operator/=(const DoubleVector<_count> &b)
  {
    div(data, data, b.data);
    return *this;
  }

  lxivec_always_inline DoubleVector<_count> operator/(float b) const
  {
    DoubleVector<_count> r;
    div(r.data, data, b);
    return r;
  }

  lxivec_always_inline DoubleVector<_count> & operator/=(float b)
  {
    div(data, data, b);
    return *this;
  }

  lxivec_always_inline BoolVector<8, _count> operator==(const DoubleVector<_count> &b) const
  {
    BoolVector<8, _count> r;
    cmpeq(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<8, _count> operator>(const DoubleVector<_count> &b) const
  {
    BoolVector<8, _count> r;
    cmpgt(r.data, data, b.data);
    return r;
  }

  lxivec_always_inline BoolVector<8, _count> operator<(const DoubleVector<_count> &b) const
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

template <int _count>
lxivec_always_inline _private::FloatVector<_count> abs(
    const _private::FloatVector<_count> &x)
{
  _private::FloatVector<_count> r;
  _private::abs(r.data, x.data);
  return r;
}

template <int _count>
lxivec_always_inline _private::DoubleVector<_count> abs(
    const _private::DoubleVector<_count> &x)
{
  _private::DoubleVector<_count> r;
  _private::abs(r.data, x.data);
  return r;
}

template <int _count>
lxivec_always_inline _private::FloatVector<_count> min(
    const _private::FloatVector<_count> &a,
    const _private::FloatVector<_count> &b)
{
  _private::FloatVector<_count> r;
  _private::min(r.data, a.data, b.data);
  return r;
}

template <int _count>
lxivec_always_inline _private::DoubleVector<_count> min(
    const _private::DoubleVector<_count> &a,
    const _private::DoubleVector<_count> &b)
{
  _private::DoubleVector<_count> r;
  _private::min(r.data, a.data, b.data);
  return r;
}

template <int _count>
lxivec_always_inline _private::FloatVector<_count> max(
    const _private::FloatVector<_count> &a,
    const _private::FloatVector<_count> &b)
{
  _private::FloatVector<_count> r;
  _private::max(r.data, a.data, b.data);
  return r;
}

template <int _count>
lxivec_always_inline _private::DoubleVector<_count> max(
    const _private::DoubleVector<_count> &a,
    const _private::DoubleVector<_count> &b)
{
  _private::DoubleVector<_count> r;
  _private::max(r.data, a.data, b.data);
  return r;
}

template <int _count>
lxivec_always_inline _private::FloatVector<_count> select(
    const _private::BoolVector<4, _count> &m,
    const _private::FloatVector<_count> &a,
    const _private::FloatVector<_count> &b)
{
  _private::FloatVector<_count> r;
  _private::select(r.data, m.data, a.data, b.data);
  return r;
}

template <int _count>
lxivec_always_inline _private::DoubleVector<_count> select(
    const _private::BoolVector<8, _count> &m,
    const _private::DoubleVector<_count> &a,
    const _private::DoubleVector<_count> &b)
{
  _private::DoubleVector<_count> r;
  _private::select(r.data, m.data, a.data, b.data);
  return r;
}

} // End of namespace

#endif
