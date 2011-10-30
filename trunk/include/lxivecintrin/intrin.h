/***************************************************************************
 *   Copyright (C) 2011 by A.J. Admiraal                                   *
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
#ifndef LXIVECINTRIN_INTRIN_H
#define LXIVECINTRIN_INTRIN_H

#include "platform.h"

namespace lxivec {
namespace _private {

#ifdef _MSC_VER
# pragma pack(1)
#endif

  struct lxivec_packed lxivec_align VecObject
  {
#if defined(__SSE__)
    lxivec_always_inline void * operator new(size_t size)
    {
      return _mm_malloc(size, vecsize);
    }

    lxivec_always_inline void operator delete(void *ptr)
    {
      _mm_free(ptr);
    }
#endif

  protected:
    lxivec_always_inline VecObject(void)
    {
    }

    lxivec_always_inline ~VecObject()
    {
    }
  };

#ifdef _MSC_VER
# pragma pack()
#endif
} // End of namespace

  lxivec_always_inline int8_t   abs(int8_t a)    { return (a >= 0) ? a : -a; }
  lxivec_always_inline uint8_t  abs(uint8_t a)   { return a; }
  lxivec_always_inline int16_t  abs(int16_t a)   { return (a >= 0) ? a : -a; }
  lxivec_always_inline uint16_t abs(uint16_t a)  { return a; }
  lxivec_always_inline int32_t  abs(int32_t a)   { return (a >= 0) ? a : -a; }
  lxivec_always_inline uint32_t abs(uint32_t a)  { return a; }
  lxivec_always_inline int64_t  abs(int64_t a)   { return (a >= 0) ? a : -a; }
  lxivec_always_inline uint64_t abs(uint64_t a)  { return a; }
  lxivec_always_inline float    abs(float a)     { return (a >= 0.0f) ? a : -a; }
  lxivec_always_inline double   abs(double a)    { return (a >= 0.0) ? a : -a; }

#ifndef __LP64__
  lxivec_always_inline long abs(long a) { return abs(int32_t(a)); }
  lxivec_always_inline unsigned long abs(unsigned long a) { return abs(uint32_t(a)); }
#endif
  lxivec_always_inline long long abs(long long a) { return abs(int64_t(a)); }
  lxivec_always_inline unsigned long long abs(unsigned long long a) { return abs(uint64_t(a)); }

  template <typename _type>
  lxivec_always_inline _type max(_type a, _type b)
  {
    return (a >= b) ? a : b;
  }

  template <typename _type>
  lxivec_always_inline _type min(_type a, _type b)
  {
    return (a <= b) ? a : b;
  }

  template <typename _type>
  lxivec_always_inline _type bound(_type min, _type a, _type max)
  {
    return (a < min) ? min : ((a > max) ? max : a);
  }

  template <typename _type>
  lxivec_always_inline _type select(bool s, _type a, _type b)
  {
    return s ? a : b;
  }

} // End of namespace

#endif
