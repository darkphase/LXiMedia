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
#ifndef LXIVECINTRIN_PLATFORM_H
#define LXIVECINTRIN_PLATFORM_H

#include <sys/types.h>
#include <stdint.h>

#ifdef __SSE__
#include <xmmintrin.h>
#endif
#ifdef __SSE2__
#include <emmintrin.h>
#endif
#ifdef __SSE3__
#include <pmmintrin.h>
#endif
#ifdef __SSSE3__
#include <tmmintrin.h>
#endif
#ifdef __SSE4A__
#include <ammintrin.h>
#endif

#if defined(_MSC_VER) && defined(__SSE2__) && !(_M_AMD64)
__forceinline __m128i _mm_set_epi64x(__int64 i1, __int64 i2) 
{ 
  return _mm_set_epi32(
    __int32(i1 >> 32), __int32(i1), 
    __int32(i2 >> 32), __int32(i2)); 
}

__forceinline __m128i _mm_set1_epi64x(__int64 i) 
{ 
  return _mm_set_epi32(
    __int32(i >> 32), __int32(i), 
    __int32(i >> 32), __int32(i)); 
}
#endif

namespace lxivec {

#if defined(__AVX__)
  static const int vecsize = 32;
#elif defined(__SSE__)
  static const int vecsize = 16;
#else
  static const int vecsize = 8;
#endif

#undef lxivec_always_inline
#undef lxivec_packed
#undef lxivec_align
#undef lxivec_salloc

#if defined(__unix__) && defined(__GNUC__)
# define lxivec_always_inline     inline __attribute__((always_inline))
# define lxivec_packed            __attribute__((packed))
# if defined(__SSE__)
#  define lxivec_align            __attribute__((aligned(16)))
# else
#  define lxivec_align            __attribute__((aligned(4)))
# endif
# define lxivec_salloc(dyn, stat) (dyn)

#elif defined(__APPLE__) && defined(__GNUC__)
# define lxivec_always_inline     inline __attribute__((always_inline))
# define lxivec_packed            __attribute__((packed))
# if defined(__SSE__)
#  define lxivec_align            __attribute__((aligned(16)))
# else
#  define lxivec_align            __attribute__((aligned(4)))
# endif
# define lxivec_salloc(dyn, stat) (dyn)

#elif defined(WIN32) && defined(__GNUC__)
# define lxivec_always_inline     inline __attribute__((always_inline))
# define lxivec_packed            __attribute__((packed))
# if defined(__SSE__)
#  define lxivec_align            __attribute__((aligned(16)))
# else
#  define lxivec_align            __attribute__((aligned(4)))
# endif
# define lxivec_salloc(dyn, stat) (stat) // dyn causes internal compiler error.

#elif defined(WIN32) && defined(_MSC_VER)
# define lxivec_always_inline     __forceinline
# define lxivec_packed
# if defined(__SSE__)
#  define lxivec_align            __declspec(align(16))
# else
#  define lxivec_align            __declspec(align(4))
# endif
# define lxivec_salloc(dyn, stat) (stat)

#else
# error Platform not supported
#endif

}

#endif
