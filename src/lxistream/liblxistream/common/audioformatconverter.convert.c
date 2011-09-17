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

#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#ifdef __SSE__
  #include <emmintrin.h>
#endif
#include <stdint.h>

__inline float fbound(float a, float b, float c) { return b < a ? a : (b > c ? c : b); }
__inline double dbound(double a, double b, double c) { return b < a ? a : (b > c ? c : b); }

void LXiStream_Common_AudioFormatConverter_swap16
 (uint16_t * __restrict dst, const uint16_t * __restrict src, size_t len)
{
  unsigned i, n;

#ifndef __SSE__
  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (src[i] >> 8) | (src[i] << 8);
#else
  __m128i * const __restrict vdst = (__m128i *)dst;
  const __m128i * const __restrict vsrc = (const __m128i *)src;

  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  for (i=0, n=len/sizeof(*vdst); i<n; i++)
  {
    const __m128i v = _mm_load_si128(vsrc + i);

    _mm_store_si128(vdst + i, _mm_or_si128(_mm_slli_epi16(v, 8), _mm_srli_epi16(v, 8)));
  }

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
      dst[i] = (src[i] >> 8) | (src[i] << 8);
  }
#endif
}

void LXiStream_Common_AudioFormatConverter_swap32
 (uint32_t * __restrict dst, const uint32_t * __restrict src, size_t len)
{
 unsigned i, n;

 for (i=0, n=len/sizeof(*dst); i<n; i++)
 {
#if defined(__GNUC__)
    dst[i] = __builtin_bswap32(src[i]);
#elif defined(_MSC_VER)
    dst[i] = _byteswap_ulong(src[i]);
#else
    dst[i] = (src[i] << 24) |
             ((src[i] & 0x0000FF00) << 8) |
             ((src[i] & 0x00FF0000) >> 8) |
             (src[i] >> 24);
#endif
  }
}

void LXiStream_Common_AudioFormatConverter_swap64
 (uint64_t * __restrict dst, const uint64_t * __restrict src, size_t len)
{
 unsigned i, n;

 for (i=0, n=len/sizeof(*dst); i<n; i++)
 {
#if defined(__GNUC__)
    dst[i] = __builtin_bswap64(src[i]);
#elif defined(_MSC_VER)
    dst[i] = _byteswap_uint64(src[i]);
#else
    dst[i] = (src[i]  << 56) |
             ((src[i] << 40) & 0x00FF000000000000ull) |
             ((src[i] << 24) & 0x0000FF0000000000ull) |
             ((src[i] << 8)  & 0x000000FF00000000ull) |
             ((src[i] >> 8)  & 0x00000000FF000000ull) |
             ((src[i] >> 24) & 0x0000000000FF0000ull) |
             ((src[i] >> 40) & 0x000000000000FF00ull) |
             (src[i]  >> 56);
#endif
  }
}

void LXiStream_Common_AudioFormatConverter_convertU8U16
 (uint16_t * __restrict dst, const uint8_t * __restrict src, size_t len)
{
  unsigned i, n;

#ifndef __SSE__
  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (src[i] << 8);
#else
  __m128i * const __restrict vdst = (__m128i *)dst;
  const __m128i * const __restrict vsrc = (const __m128i *)src;
  const __m128i v0 = _mm_set1_epi16(0);

  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  for (i=0, n=len/sizeof(*vdst); i<n; i++)
  {
    const __m128i v = _mm_load_si128(vsrc + i);

    _mm_store_si128(vdst + (i * 2),     _mm_unpacklo_epi8(v0, v));
    _mm_store_si128(vdst + (i * 2) + 1, _mm_unpackhi_epi8(v0, v));
  }

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
      dst[i] = (src[i] << 8);
  }
#endif
}

void LXiStream_Common_AudioFormatConverter_convertU16U8
 (uint8_t * __restrict dst, const uint16_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (src[i] >> 8);
}

void LXiStream_Common_AudioFormatConverter_convertU16U32
 (uint32_t * __restrict dst, const uint16_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (src[i] << 16);
}

void LXiStream_Common_AudioFormatConverter_convertU32U16
 (uint16_t * __restrict dst, const uint32_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (src[i] >> 16);
}

void LXiStream_Common_AudioFormatConverter_convertS16F32
 (float * __restrict dst, const int16_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = ((float)src[i]) / 32768.0f;
}

void LXiStream_Common_AudioFormatConverter_convertF32S16
 (int16_t * __restrict dst, const float * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (uint16_t)fbound(-32768.0f, src[i] * 32768.0f, 32767.0f);
}

void LXiStream_Common_AudioFormatConverter_convertS32F32
 (float * __restrict dst, const int32_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = ((float)src[i]) / 2147483648.0f;
}

void LXiStream_Common_AudioFormatConverter_convertF32S32
 (int32_t * __restrict dst, const float * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (int32_t)fbound(-2147483648.0f, src[i] * 2147483648.0f, 2147483647.0f);
}

void LXiStream_Common_AudioFormatConverter_convertS16F64
 (double * __restrict dst, const int16_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = ((double)src[i]) / 32768.0;
}

void LXiStream_Common_AudioFormatConverter_convertF64S16
 (int16_t * __restrict dst, const double * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (uint16_t)dbound(-32768.0, src[i] * 32768.0, 32767.0);
}

void LXiStream_Common_AudioFormatConverter_convertS32F64
 (double * __restrict dst, const int32_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = ((double)src[i]) / 2147483648.0;
}

void LXiStream_Common_AudioFormatConverter_convertF64S32
 (int32_t * __restrict dst, const double * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (int32_t)dbound(-2147483648.0, src[i] * 2147483648.0, 2147483647.0);
}

void LXiStream_Common_AudioFormatConverter_convertU16S16
 (int16_t * __restrict dst, const uint16_t * __restrict src, size_t len)
{
  unsigned i, n;

#ifndef __SSE__
  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = src[i] - ((int16_t)32768);
#else
  __m128i * const __restrict vdst = (__m128i *)dst;
  const __m128i * const __restrict vsrc = (const __m128i *)src;
  const __m128i v32768 = _mm_set1_epi16(32768);

  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  for (i=0, n=len/sizeof(*vdst); i<n; i++)
    _mm_store_si128(vdst + i, _mm_sub_epi16(_mm_load_si128(vsrc + i), v32768));

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
      dst[i] = src[i] - ((int16_t)32768);
  }
#endif
}

void LXiStream_Common_AudioFormatConverter_convertS16U16
 (uint16_t * __restrict dst, const int16_t * __restrict src, size_t len)
{
  unsigned i, n;

#ifndef __SSE__
  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = src[i] + ((int16_t)32768);
#else
  __m128i * const __restrict vdst = (__m128i *)dst;
  const __m128i * const __restrict vsrc = (const __m128i *)src;
  const __m128i v32768 = _mm_set1_epi16(32768);

  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  for (i=0, n=len/sizeof(*vdst); i<n; i++)
    _mm_store_si128(vdst + i, _mm_add_epi16(_mm_load_si128(vsrc + i), v32768));

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
      dst[i] = src[i] + ((int16_t)32768);
  }
#endif
}

void LXiStream_Common_AudioFormatConverter_convertU32S32
 (int32_t * __restrict dst, const uint32_t * __restrict src, size_t len)
{
  unsigned i, n;

#ifndef __SSE__
  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = src[i] - ((int32_t)2147483648);
#else
  __m128i * const __restrict vdst = (__m128i *)dst;
  const __m128i * const __restrict vsrc = (const __m128i *)src;
  const __m128i v2147483648 = _mm_set1_epi32(2147483648);

  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  for (i=0, n=len/sizeof(*vdst); i<n; i++)
    _mm_store_si128(vdst + i, _mm_sub_epi32(_mm_load_si128(vsrc + i), v2147483648));

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
      dst[i] = src[i] - ((int32_t)2147483648);
  }
#endif
}

void LXiStream_Common_AudioFormatConverter_convertS32U32
 (uint32_t * __restrict dst, const int32_t * __restrict src, size_t len)
{
  unsigned i, n;

#ifndef __SSE__
  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = src[i] + ((int32_t)2147483648);
#else
  __m128i * const __restrict vdst = (__m128i *)dst;
  const __m128i * const __restrict vsrc = (const __m128i *)src;
  const __m128i v2147483648 = _mm_set1_epi32(2147483648);

  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  for (i=0, n=len/sizeof(*vdst); i<n; i++)
    _mm_store_si128(vdst + i, _mm_add_epi32(_mm_load_si128(vsrc + i), v2147483648));

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
      dst[i] = src[i] + ((int32_t)2147483648);
  }
#endif
}
