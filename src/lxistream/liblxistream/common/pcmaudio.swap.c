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

void LXiStream_Common_PcmAudio_decodeS8S16
 (int16_t * __restrict dst, const int8_t * __restrict src, size_t len)
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

void LXiStream_Common_PcmAudio_decodeU8S16
 (int16_t * __restrict dst, const int8_t * __restrict src, size_t len)
{
  unsigned i, n;

#ifndef __SSE__
  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = ((src[i] - 128) << 8);
#else
  __m128i * const __restrict vdst = (__m128i *)dst;
  const __m128i * const __restrict vsrc = (const __m128i *)src;
  const __m128i v0 = _mm_set1_epi16(0);
  const __m128i v128 = _mm_set1_epi8(128);

  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  for (i=0, n=len/sizeof(*vdst); i<n; i++)
  {
    const __m128i v = _mm_sub_epi8(_mm_load_si128(vsrc + i), v128);

    _mm_store_si128(vdst + (i * 2),     _mm_unpacklo_epi8(v0, v));
    _mm_store_si128(vdst + (i * 2) + 1, _mm_unpackhi_epi8(v0, v));
  }

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
      dst[i] = ((src[i] - 128) << 8);
  }
#endif
}

void LXiStream_Common_PcmAudio_swap16
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

void LXiStream_Common_PcmAudio_decodeU16S16
 (int16_t * __restrict dst, const uint16_t * __restrict src, size_t len)
{
  unsigned i, n;

#ifndef __SSE__
  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = src[i] - ((int16_t)32768);
#else
  __m128i * const __restrict vdst = (__m128i *)dst;
  const __m128i * const __restrict vsrc = (const __m128i *)src;
  const __m128i v32767 = _mm_set1_epi16(32768);

  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  for (i=0, n=len/sizeof(*vdst); i<n; i++)
    _mm_store_si128(vdst + i, _mm_sub_epi16(_mm_load_si128(vsrc + i), v32767));

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
      dst[i] = src[i] - ((int16_t)32768);
  }
#endif
}

void LXiStream_Common_PcmAudio_swapDecodeU16S16
 (int16_t * __restrict dst, const uint16_t * __restrict src, size_t len)
{
  unsigned i, n;

#ifndef __SSE__
  for (i=0, n=len/sizeof(*dst); i<n; i++)
  {
    const int16_t swapped = (src[i] >> 8) | (src[i] << 8);
    dst[i] = swapped - ((int16_t)32768);
  }
#else
  __m128i * const __restrict vdst = (__m128i *)dst;
  const __m128i * const __restrict vsrc = (const __m128i *)src;
  const __m128i v32767 = _mm_set1_epi16(32768);

  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  for (i=0, n=len/sizeof(*vdst); i<n; i++)
  {
    const __m128i v = _mm_load_si128(vsrc + i);
    const __m128i swapped = _mm_or_si128(_mm_slli_epi16(v, 8), _mm_srli_epi16(v, 8));

    _mm_store_si128(vdst + i, _mm_sub_epi16(swapped, v32767));
  }

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
    {
      const int16_t swapped = (src[i] >> 8) | (src[i] << 8);
      dst[i] = swapped - ((int16_t)32768);
    }
  }
#endif
}

void LXiStream_Common_PcmAudio_encodeS16U16
 (uint16_t * __restrict dst, const int16_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = src[i] + 32768;
}

void LXiStream_Common_PcmAudio_encodeSwapS16U16
 (uint16_t * __restrict dst, const int16_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
  {
    const uint16_t val = src[i] + 32768;
    dst[i] = (val >> 8) | (val << 8);
  }
}

void LXiStream_Common_PcmAudio_decodeS32S16
 (int16_t * __restrict dst, const int32_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (src[i] >> 16);
}

void LXiStream_Common_PcmAudio_swapDecodeS32S16
 (int16_t * __restrict dst, const int32_t * __restrict src, size_t len)
{
 unsigned i, n;

 for (i=0, n=len/sizeof(*dst); i<n; i++)
 {
#if defined(__GNUC__)
    dst[i] = (__builtin_bswap32(src[i]) >> 16);
#elif defined(_MSC_VER)
    dst[i] = (_byteswap_ulong(src[i]) >> 16);
#endif
  }
}

void LXiStream_Common_PcmAudio_decodeU32S16
 (int16_t * __restrict dst, const int32_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = ((int16_t)(src[i] >> 16)) - ((int16_t)32768);
}

void LXiStream_Common_PcmAudio_swapDecodeU32S16
 (int16_t * __restrict dst, const uint32_t * __restrict src, size_t len)
{
 unsigned i, n;

 for (i=0, n=len/sizeof(*dst); i<n; i++)
 {
#if defined(__GNUC__)
    dst[i] = ((int16_t)(__builtin_bswap32(src[i]) >> 16)) - ((int16_t)32768);
#elif defined(_MSC_VER)
    dst[i] = ((int16_t)(_byteswap_ulong(src[i]) >> 16)) - ((int16_t)32768);
#endif
  }
}

void LXiStream_Common_PcmAudio_decodeF32S16
 (int16_t * __restrict dst, const float * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (int16_t)(src[i] * 32767.0f);
}

void LXiStream_Common_PcmAudio_swapDecodeF32S16
 (int16_t * __restrict dst, const uint32_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
  {
#if defined(__GNUC__)
    const uint32_t swapped = __builtin_bswap32(src[i]);
#elif defined(_MSC_VER)
    const uint32_t swapped = _byteswap_ulong(src[i]);
#endif

    dst[i] = (int16_t)(*((float *)&swapped) * 32767.0f);
  }
}

void LXiStream_Common_PcmAudio_decodeF64S16
 (int16_t * __restrict dst, const double * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (int16_t)(src[i] * 32767.0);
}

void LXiStream_Common_PcmAudio_swapDecodeF64S16
 (int16_t * __restrict dst, const uint64_t * __restrict src, size_t len)
{
  unsigned i, n;

  for (i=0, n=len/sizeof(*dst); i<n; i++)
  {
#if defined(__GNUC__)
    const uint64_t swapped = __builtin_bswap64(src[i]);
#elif defined(_MSC_VER)
    const uint64_t swapped = _byteswap_uint64(src[i]);
#endif

    dst[i] = (int16_t)(*((double *)&swapped) * 32767.0);
  }
}
