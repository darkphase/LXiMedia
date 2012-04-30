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

#include "videoconvert.h"
#include <lxivecintrin/vectypes>

namespace LXiStream {
namespace Algorithms {

using namespace lxivec;

#if defined(__GNUC__)
struct __attribute__((packed)) VideoConvert::RGBAPixel { uint8_t b, g, r, a; };
#elif defined(_MSC_VER)
# pragma pack(1)
struct VideoConvert::RGBAPixel { uint8_t b, g, r, a; };
# pragma pack()
#else
struct VideoConvert::RGBAPixel { uint8_t b, g, r, a; };
#endif

void VideoConvert::YUYVtoYUV2(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint8_t * src, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint8_t * src, int m)
    {
      const uint8_qv yuyv = uint8_qv::load(src, m);
      const uint8_dv y = yuyv.as<int16_qv>() & 0x00FF;
      const uint8_v u = (yuyv.as<int32_qv>() & 0x0000FF00) >> 8;
      const uint8_v v = (yuyv.as<int32_qv>() >> 24) & 0x000000FF;

      store(dsty, y, m / 2);
      store(dstu, u, m / 4);
      store(dstv, v, m / 4);
    }
  };

  n *= 2;

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint8_qv::count <= n; i += uint8_qv::count)
    T::process(dsty + i / 2, dstu + i / 4, dstv + i / 4, src + i, uint8_qv::count);

  if (i < n)
    T::process(dsty + i / 2, dstu + i / 4, dstv + i / 4, src + i, n - i);
}

void VideoConvert::UYVYtoYUV2(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint8_t * src, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint8_t * src, int m)
    {
      const uint8_qv uyvy = uint8_qv::load(src, m);
      const uint8_dv y = (uyvy.as<int16_qv>() >> 8) & 0x00FF;
      const uint8_v u = uyvy.as<int32_qv>() & 0x000000FF;
      const uint8_v v = (uyvy.as<int32_qv>() >> 16) & 0x000000FF;

      store(dsty, y, m / 2);
      store(dstu, u, m / 4);
      store(dstv, v, m / 4);
    }
  };

  n *= 2;

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint8_qv::count <= n; i += uint8_qv::count)
    T::process(dsty + i / 2, dstu + i / 4, dstv + i / 4, src + i, uint8_qv::count);

  if (i < n)
    T::process(dsty + i / 2, dstu + i / 4, dstv + i / 4, src + i, n - i);
}

void VideoConvert::mergeUVlines(
    uint8_t *dstu, uint8_t *dstv,
    const uint8_t * srcua, const uint8_t * srcub,
    const uint8_t * srcva, const uint8_t * srcvb,
    int n)
{
  for (int i = 0; i < n; i += uint8_v::count)
  {
    int16_dv a = uint8_v::load(srcua + i);
    int16_dv b = uint8_v::load(srcub + i);

    uint8_v r = (a + b) >> 1;
    store(dstu + i, r);

    a = uint8_v::load(srcva + i);
    b = uint8_v::load(srcvb + i);

    r = (a + b) >> 1;
    store(dstv + i, r);
  }
}

/*! Converts YUYV to RGB using the following formula:
\code
    R = Y + 1.403 * V'
    G = Y - 0.344 * U' - 0.714 * V'
    B = Y + 1.770 * U'
\endcode

    \sa http://www.fourcc.org/fccyvrgb.php
 */
void VideoConvert::YUYVtoRGB(uint32_t *dst, const uint8_t *src, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint32_t *dst, const uint8_t *src, int m)
    {
      const int16_qv mu = int16_qv::set(113, -22, 0, 0);
      const int16_qv mv = int16_qv::set(0, -46, 90, 0);

      const uint8_v yuyv = uint8_v::load(src, m);
  
      // Convert YUYVYUYV... to xYYYxYYYxYYYxYYY...
      uint32_dv y = yuyv.as<uint16_v>() & 0x00FF;
      y = y | (y << 8) | (y << 16) | 0xFF000000;
  
      // Convert YUYVYUYV... to xUUUxUUUxUUUxUUU...
      uint64_dv u = yuyv.as<uint32_v>() & 0x0000FF00;
      u = u | (u << 8) | (u >> 8);
      u |= u << 32;
  
      // Convert YUYVYUYV... to xVVVxVVVxVVVxVVV...
      uint64_dv v = yuyv.as<uint32_v>() >> 24;
      v = v | (v << 8) | (v << 16);
      v |= v << 32;
  
      // Multipy factors and pack into RGBARGBA...
      const uint8_dv r =
          int16_qv(y.as<uint8_dv>()) +
          (((int16_qv(u.as<uint8_dv>()) - 128) * mu) >> 6) +
          (((int16_qv(v.as<uint8_dv>()) - 128) * mv) >> 6);

      store(dst, r.as<uint32_dv>(), m / 2);
    }
  };

  n *= 2;

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint8_v::count <= n; i += uint8_v::count)
    T::process(dst + i / 2, src + i, uint8_v::count);

  if (i < n)
    T::process(dst + i / 2, src + i, n - i);
}

/*! Converts UYVY to RGB using the following formula:
\code
    R = Y + 1.403 * V'
    G = Y - 0.344 * U' - 0.714 * V'
    B = Y + 1.770 * U'
\endcode

    \sa http://www.fourcc.org/fccyvrgb.php
 */
void VideoConvert::UYVYtoRGB(uint32_t *dst, const uint8_t *src, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint32_t *dst, const uint8_t *src, int m)
    {
      const int16_qv mu = int16_qv::set(113, -22, 0, 0);
      const int16_qv mv = int16_qv::set(0, -46, 90, 0);

      const uint8_v uyvy = uint8_v::load(src, m);

      // Convert UYVYUYVY... to xYYYxYYYxYYYxYYY...
      uint32_dv y = uyvy.as<uint16_v>() & 0xFF00;
      y = y | (y << 8) | (y >> 8) | 0xFF000000;

      // Convert UYVYUYVY... to xUUUxUUUxUUUxUUU...
      uint64_dv u = uyvy.as<uint32_v>() & 0x000000FF;
      u = u | (u << 8) | (u << 16);
      u |= u << 32;

      // Convert UYVYUYVY... to xVVVxVVVxVVVxVVV...
      uint64_dv v = uyvy.as<uint32_v>() & 0x00FF0000;
      v = v | (v >> 8) | (v >> 16);
      v |= v << 32;

      // Multipy factors and pack into RGBARGBA...
      const uint8_dv r =
          int16_qv(y.as<uint8_dv>()) +
          (((int16_qv(u.as<uint8_dv>()) - 128) * mu) >> 6) +
          (((int16_qv(v.as<uint8_dv>()) - 128) * mv) >> 6);

      store(dst, r.as<uint32_dv>(), m / 2);
    }
  };

  n *= 2;

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint8_v::count <= n; i += uint8_v::count)
    T::process(dst + i / 2, src + i, uint8_v::count);

  if (i < n)
    T::process(dst + i / 2, src + i, n - i);
}

/*! Converts YUV1 to RGB using the following formula:
\code
    R = Y + 1.403 * V'
    G = Y - 0.344 * U' - 0.714 * V'
    B = Y + 1.770 * U'
\endcode

    \sa http://www.fourcc.org/fccyvrgb.php
 */
void VideoConvert::YUV1toRGB(uint32_t *dst, const uint8_t *srcy, const uint8_t *srcu, const uint8_t *srcv, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint32_t *dst, const uint8_t *srcy, const uint8_t *srcu, const uint8_t *srcv, int m)
    {
      const int16_ov mu = int16_ov::set(113, -22, 0, 0);
      const int16_ov mv = int16_ov::set(0, -46, 90, 0);

      // xYYYxYYYxYYYxYYY...
      uint32_qv y = uint16_dv(uint8_v::load(srcy, m));
      y = y | (y << 8) | (y << 16) | 0xFF000000;

      // xUUUxUUUxUUUxUUU...
      uint32_qv u = uint16_dv(uint8_v::load(srcu, m));
      u = u | (u << 8) | (u << 16);

      // Convert UYVYUYVY... to xVVVxVVVxVVVxVVV...
      uint32_qv v = uint16_dv(uint8_v::load(srcv, m));
      v = v | (v << 8) | (v << 16);

      // Multipy factors and pack into RGBARGBA...
      const uint8_qv r =
          int16_ov(y.as<uint8_qv>()) +
          (((int16_ov(u.as<uint8_qv>()) - 128) * mu) >> 6) +
          (((int16_ov(v.as<uint8_qv>()) - 128) * mv) >> 6);

      store(dst, r.as<uint32_qv>(), m);
    }
  };

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint8_v::count <= n; i += uint8_v::count)
    T::process(dst + i, srcy + i, srcu + i, srcv + i, uint8_v::count);

  if (i < n)
    T::process(dst + i, srcy + i, srcu + i, srcv + i, n - i);
}

/*! Converts YUV2 to RGB using the following formula:
\code
    R = Y + 1.403 * V'
    G = Y - 0.344 * U' - 0.714 * V'
    B = Y + 1.770 * U'
\endcode

    \sa http://www.fourcc.org/fccyvrgb.php
 */
void VideoConvert::YUV2toRGB(uint32_t *dst, const uint8_t *srcy, const uint8_t *srcu, const uint8_t *srcv, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint32_t *dst, const uint8_t *srcy, const uint8_t *srcu, const uint8_t *srcv, int m)
    {
      const int16_hv mu = int16_hv::set(113, -22, 0, 0);
      const int16_hv mv = int16_hv::set(0, -46, 90, 0);

      // xYYYxYYYxYYYxYYY...
      uint32_ov y = uint16_qv(uint8_dv::load(srcy, m));
      y = y | (y << 8) | (y << 16) | 0xFF000000;

      // xUUUxUUUxUUUxUUU...
      uint32_qv u = uint16_dv(uint8_v::load(srcu, m / 2));
      u = u | (u << 8) | (u << 16);

      // Convert UYVYUYVY... to xVVVxVVVxVVVxVVV...
      uint32_qv v = uint16_dv(uint8_v::load(srcv, m / 2));
      v = v | (v << 8) | (v << 16);

      // Multipy factors and pack into RGBARGBA...
      const uint8_ov r =
          int16_hv(y.as<uint8_ov>()) +
          (((int16_hv(interleave(u, u).as<uint8_ov>()) - 128) * mu) >> 6) +
          (((int16_hv(interleave(v, v).as<uint8_ov>()) - 128) * mv) >> 6);

      store(dst, r.as<uint32_ov>(), m);
    }
  };

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint8_dv::count <= n; i += uint8_dv::count)
    T::process(dst + i, srcy + i, srcu + (i / 2), srcv + (i / 2), uint8_dv::count);

  if (i < n)
    T::process(dst + i, srcy + i, srcu + (i / 2), srcv + (i / 2), n - i);
}

/*! Converts RGB to YUYV using the following formula:
\code
    Y = 0.299R + 0.587G + 0.114B
    U'= (B - Y) * 0.565
    V'= (R - Y) * 0.713
\endcode

    \sa http://www.fourcc.org/fccyvrgb.php
 */
void VideoConvert::RGBtoYUYV(uint8_t *dst, const uint32_t *src, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint8_t *dst, const uint32_t *src, int m)
    {
      const int16_qv my = int16_qv::set(7, 38, 19, 0);
      const int16_v muv = int16_v::set(36, 46);

      const uint32_dv rgba = uint32_dv::load(src, m);

      // Convert RGBARGBA... to YY...
      const int16_v y = hadd(hadd(int16_qv(rgba.as<uint8_dv>()) * my)) >> 6;

      // Compute average Y and R,B of two neighburing pixels.
      const int16_v ya = (y + ((y.as<uint32_v>() >> 16) | (y.as<uint32_v>() << 16)).as<int16_v>()) >> 1;
      const int16_v rba = (hadd(rgba & 0x00FF00FF) >> 1).as<int16_v>();

      // Multiply factors and pack into YUYVYUYV...
      const uint8_v r = interleave(y, (((rba - ya) * muv) >> 6) + 128);

      store(dst, r, m * 2);
    }
  };

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint32_dv::count <= n; i += uint32_dv::count)
    T::process(dst + i * 2, src + i, uint32_dv::count);

  if (i < n)
    T::process(dst + i * 2, src + i, n - i);
}

/*! Converts RGB to YUYV using the following formula:
\code
    Y = 0.299R + 0.587G + 0.114B
    U'= (B - Y) * 0.565
    V'= (R - Y) * 0.713
\endcode

    \sa http://www.fourcc.org/fccyvrgb.php
 */
void VideoConvert::RGBtoUYVY(uint8_t *dst, const uint32_t *src, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint8_t *dst, const uint32_t *src, int m)
    {
      const int16_qv my = int16_qv::set(7, 38, 19, 0);
      const int16_v muv = int16_v::set(36, 46);

      const uint32_dv rgba = uint32_dv::load(src, m);
  
      // Convert RGBARGBA... to YY...
      const int16_v y = hadd(hadd(int16_qv(rgba.as<uint8_dv>()) * my)) >> 6;
  
      // Compute average Y and R,B of two neighburing pixels.
      const int16_v ya = (y + ((y.as<uint32_v>() >> 16) | (y.as<uint32_v>() << 16)).as<int16_v>()) >> 1;
      const int16_v rba = (hadd(rgba & 0x00FF00FF) >> 1).as<int16_v>();
  
      // Multiply factors and pack into YUYVYUYV...
      const uint8_v r = interleave((((rba - ya) * muv) >> 6) + 128, y);
  
      store(dst, r, m * 2);
    }
  };

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint32_dv::count <= n; i += uint32_dv::count)
    T::process(dst + i * 2, src + i, uint32_dv::count);

  if (i < n)
    T::process(dst + i * 2, src + i, n - i);
}

/*! Converts RGB to YUV1 using the following formula:
\code
    Y = 0.299R + 0.587G + 0.114B
    U'= (B - Y) * 0.565
    V'= (R - Y) * 0.713
\endcode

    \sa http://www.fourcc.org/fccyvrgb.php
 */
void VideoConvert::RGBtoYUV1(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint32_t *src, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint32_t *src, int m)
    {
      const int16_ov my = int16_ov::set(7, 38, 19, 0);
      const int16_dv muv = int16_dv::set(36, 46);

      const uint32_qv rgba = uint32_qv::load(src, m);

      // Convert RGBARGBA... to YY...
      const uint8_v y = hadd(hadd(int16_ov(rgba.as<uint8_qv>()) * my)) >> 6;
      store(dsty, y, m);

      // Convert RGBARGBA... to UU...
      const uint8_v u = (((int16_dv(rgba & 0x000000FF) - y) * 36) >> 6) + 128;
      store(dstu, u, m);

      // Convert RGBARGBA... to VV...
      const uint8_v v = (((int16_dv((rgba & 0x00FF0000) >> 16) - y) * 46) >> 6) + 128;
      store(dstv, v, m);
    }
  };

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint32_qv::count <= n; i += uint32_qv::count)
    T::process(dsty + i, dstu + i, dstv + i, src + i, uint32_qv::count);

  if (i < n)
    T::process(dsty + i, dstu + i, dstv + i, src + i, n - i);
}

/*! Converts RGB to YUV2 using the following formula:
\code
    Y = 0.299R + 0.587G + 0.114B
    U'= (B - Y) * 0.565
    V'= (R - Y) * 0.713
\endcode

    \sa http://www.fourcc.org/fccyvrgb.php
 */
void VideoConvert::RGBtoYUV2(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint32_t *src, int n)
{
  struct T
  {
    static lxivec_always_inline void process(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint32_t *src, int m)
    {
      const int16_hv my = int16_hv::set(7, 38, 19, 0);
      const int16_dv muv = int16_dv::set(36, 46);

      const uint32_ov rgba = uint32_ov::load(src, m);

      // Convert RGBARGBA... to YY...
      const uint8_dv y = hadd(hadd(int16_hv(rgba.as<uint8_ov>()) * my)) >> 6;
      store(dsty, y, m);

      // Convert RGBARGBA... to UU...
      const uint8_v u = hadd((((int16_qv(rgba & 0x000000FF) - y) * 36) >> 6) + 128) >> 1;
      store(dstu, u, m / 2);

      // Convert RGBARGBA... to VV...
      const uint8_v v = hadd((((int16_qv((rgba & 0x00FF0000) >> 16) - y) * 46) >> 6) + 128) >> 1;
      store(dstv, v, m / 2);
    }
  };

  // Manually unroll the last item of the loop, as GCC will not do that automatically.
  int i = 0;
  for (; i + uint32_ov::count <= n; i += uint32_ov::count)
    T::process(dsty + i, dstu + (i / 2), dstv + (i / 2), src + i, uint32_ov::count);

  if (i < n)
    T::process(dsty + i, dstu + (i / 2), dstv + (i / 2), src + i, n - i);
}

/*! Converts BGR to RGB.
 */
void VideoConvert::BGRtoRGB(uint32_t *dst, const uint32_t *src, int n)
{
  for (int i = 0; i < n; i += uint32_v::count)
  {
    const uint32_v bgra = uint32_v::load(src + i);

    uint32_v rgba = bgra & 0xFF00FF00;
    rgba |= (bgra & 0x00FF0000) >> 16;
    rgba |= (bgra & 0x000000FF) << 16;

    store(dst + i, rgba);
  }
}

void VideoConvert::demosaicGRBG8(
    const uint8_t * src, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    uint8_t * dst, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const __restrict srcLine1 = src + (srcStride * y);
    const uint8_t * const __restrict srcLine2 = srcLine1 + srcStride;
    RGBAPixel * const __restrict dstLine1 = (RGBAPixel *)(dst + (dstStride * y));
    RGBAPixel * const __restrict dstLine2 = (RGBAPixel *)(((uint8_t *)dstLine1) + dstStride);

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine2[x+1].r = srcLine1[x+1];
      dstLine1[x+0].g = srcLine1[x+0];
      dstLine2[x+1].g = srcLine2[x+1];
      dstLine1[x+0].b = dstLine2[x+1].b = srcLine2[x+0];
      dstLine1[x+0].a = dstLine2[x+1].a = 0xFF;
      dstLine1[x+1].a = dstLine2[x+0].a = 0x00;
    }
  }
}

void VideoConvert::demosaicGBRG8(
    const uint8_t * src, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    uint8_t * dst, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const srcLine1 = src + (srcStride * y);
    const uint8_t * const srcLine2 = srcLine1 + srcStride;
    RGBAPixel * const dstLine1 = (RGBAPixel *)(dst + (dstStride * y));
    RGBAPixel * const dstLine2 = (RGBAPixel *)(((uint8_t *)dstLine1) + dstStride);

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine2[x+1].r = srcLine2[x+0];
      dstLine1[x+0].g = srcLine1[x+0];
      dstLine2[x+1].g = srcLine2[x+1];
      dstLine1[x+0].b = dstLine2[x+1].b = srcLine1[x+1];
      dstLine1[x+0].a = dstLine2[x+1].a = 0xFF;
      dstLine1[x+1].a = dstLine2[x+0].a = 0x00;
    }
  }
}

void VideoConvert::demosaicBGGR8(
    const uint8_t * src, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    uint8_t * dst, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const srcLine1 = src + (srcStride * y);
    const uint8_t * const srcLine2 = srcLine1 + srcStride;
    RGBAPixel * const dstLine1 = (RGBAPixel *)(dst + (dstStride * y));
    RGBAPixel * const dstLine2 = (RGBAPixel *)(((uint8_t *)dstLine1) + dstStride);

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine2[x+1].r = srcLine2[x+1];
      dstLine1[x+0].g = srcLine1[x+1];
      dstLine2[x+1].g = srcLine2[x+0];
      dstLine1[x+0].b = dstLine2[x+1].b = srcLine1[x+0];
      dstLine1[x+0].a = dstLine2[x+1].a = 0xFF;
      dstLine1[x+1].a = dstLine2[x+0].a = 0x00;
    }
  }
}

void VideoConvert::demosaicRGGB8(
    const uint8_t * src, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    uint8_t * dst, unsigned dstStride)
{
  if ((srcNumLines > 1) && (srcWidth > 1))
  for (unsigned y=0; y<srcNumLines-1; y+=2)
  {
    const uint8_t * const __restrict srcLine1 = src + (srcStride * y);
    const uint8_t * const __restrict srcLine2 = srcLine1 + srcStride;
    RGBAPixel * const __restrict dstLine1 = (RGBAPixel *)(dst + (dstStride * y));
    RGBAPixel * const __restrict dstLine2 = (RGBAPixel *)(((uint8_t *)dstLine1) + dstStride);

    for (unsigned x=0; x<srcWidth-1; x+=2)
    {
      dstLine1[x+0].r = dstLine2[x+1].r = srcLine1[x+0];
      dstLine1[x+0].g = srcLine1[x+1];
      dstLine2[x+1].g = srcLine2[x+0];
      dstLine1[x+0].b = dstLine2[x+1].b = srcLine2[x+1];
      dstLine1[x+0].a = dstLine2[x+1].a = 0xFF;
      dstLine1[x+1].a = dstLine2[x+0].a = 0x00;
    }
  }
}

void VideoConvert::demosaicPostfilter(
    uint8_t * data, unsigned width, unsigned stride, unsigned numLines)
{
  const int yo = ((const RGBAPixel *)data)->a == 0 ? 1 : 0;

  if ((numLines > 1) && (width > 1))
  {
    for (unsigned y=1; y<numLines-1; y++)
    {
      RGBAPixel * const __restrict line2 = (RGBAPixel *)(data + (stride * y));
      RGBAPixel * const __restrict line1 = (RGBAPixel *)(((uint8_t *)line2) - stride);
      RGBAPixel * const __restrict line3 = (RGBAPixel *)(((uint8_t *)line2) + stride);

      for (unsigned x=1+((y+yo)%2); x<width-1; x+=2)
      {
        const int8_t gw = line2[x-1].g >> 1;
        const int8_t ge = line2[x+1].g >> 1;
        const int8_t gn = line1[x+0].g >> 1;
        const int8_t gs = line3[x+0].g >> 1;

        if (abs(int8_t(gw - ge)) < abs(int8_t(gw - ge)))
        {
          line2[x].r = (line2[x-1].r >> 1) + (line2[x+1].r >> 1);
          line2[x].g = gw + ge;
          line2[x].b = (line2[x-1].b >> 1) + (line2[x+1].b >> 1);
        }
        else
        {
          line2[x].r = (line1[x+0].r >> 1) + (line3[x+0].r >> 1);
          line2[x].g = gn + gs;
          line2[x].b = (line1[x+0].b >> 1) + (line3[x+0].b >> 1);
        }

        line2[x].a = 0xFF;
      }

      // Side pixels
      {
        const int n = (((y+yo)%2) == 0) ? (width - 1) : 0;
        line2[n].r = (line1[n].r >> 1) + (line3[n].r >> 1);
        line2[n].g = (line1[n].g >> 1) + (line3[n].g >> 1);
        line2[n].b = (line1[n].b >> 1) + (line3[n].b >> 1);
        line2[n].a = 0xFF;
      }
    }

    // Top line
    {
      RGBAPixel * const __restrict topLine = (RGBAPixel *)data;
      for (unsigned x=1+(yo%2); x<width-1; x+=2)
      {
        topLine[x].r = (topLine[x-1].r >> 1) + (topLine[x+1].r >> 1);
        topLine[x].g = (topLine[x-1].g >> 1) + (topLine[x+1].g >> 1);
        topLine[x].b = (topLine[x-1].b >> 1) + (topLine[x+1].b >> 1);
        topLine[x].a = 0xFF;
      }

      topLine[0]       = topLine[1];
      topLine[width-1] = topLine[width-2];
    }

    // Bottom line
    {
      RGBAPixel * const __restrict botLine = (RGBAPixel *)(data + (stride * (numLines-1)));
      for (unsigned x=1+(((numLines-1)+yo)%2); x<width-1; x+=2)
      {
        botLine[x].r = (botLine[x-1].r >> 1) + (botLine[x+1].r >> 1);
        botLine[x].g = (botLine[x-1].g >> 1) + (botLine[x+1].g >> 1);
        botLine[x].b = (botLine[x-1].b >> 1) + (botLine[x+1].b >> 1);
        botLine[x].a = 0xFF;
      }

      botLine[0]       = botLine[1];
      botLine[width-1] = botLine[width-2];
    }
  }
}

} } // End of namespaces
