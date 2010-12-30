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
#include <stdint.h>
#include <assert.h>
#ifdef __SSE__
  #include <xmmintrin.h>
#endif

void LXiStream_Common_PcmAudio_swap16
 (uint16_t * restrict dst, const uint16_t * restrict src, size_t len)
{
#ifndef __SSE__
  for (unsigned i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = (src[i] >> 8) | (src[i] << 8);
#else
  // Check alignment
  assert(((size_t)dst & (size_t)15) == 0);
  assert(((size_t)src & (size_t)15) == 0);

  __m128i * const restrict vdst = (__m128i *)dst;
  const __m128i * const restrict vsrc = (const __m128i *)src;

  for (unsigned i=0, n=len/sizeof(*vdst); i<n; i++)
  {
    const __m128i v = _mm_load_si128(vsrc + i);

    _mm_store_si128(vdst + i, _mm_or_si128(_mm_slli_epi16(v, 8), _mm_srli_epi16(v, 8)));
  }

  if ((len & 15) != 0)
  {
    const size_t pos = (len / sizeof(*vdst)) * sizeof(*vdst);

    for (unsigned i=pos/sizeof(*dst), n=len/sizeof(*dst); i<n; i++)
      dst[i] = (src[i] >> 8) | (src[i] << 8);
  }
#endif
}

void LXiStream_Common_PcmAudio_swap32
 (uint32_t * restrict dst, const uint32_t * restrict src, size_t len)
{
  for (unsigned i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = __builtin_bswap32(src[i]);
}

void LXiStream_Common_PcmAudio_swap64
 (uint64_t * restrict dst, const uint64_t * restrict src, size_t len)
{
  for (unsigned i=0, n=len/sizeof(*dst); i<n; i++)
    dst[i] = __builtin_bswap64(src[i]);
}
