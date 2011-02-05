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

#ifndef LXISTREAM_SPIXELS_H
#define LXISTREAM_SPIXELS_H

#include <stdint.h>

#ifdef __cplusplus
#include "svideobuffer.h"
#endif

#ifdef __cplusplus
namespace LXiStream {
namespace SPixels {

#define OP16 inline operator uint16_t() const { return *reinterpret_cast<const uint16_t *>(this); }
#define OP24 inline operator uint32_t() const { return *reinterpret_cast<const uint32_t *>(this) | 0xFF000000u; }
#define OP32 inline operator uint32_t() const { return *reinterpret_cast<const uint32_t *>(this); }
#else
#define OP16
#define OP24
#define OP32
#endif
  
struct RGBAPixel { uint8_t b,  g, r,  a; OP32 } __attribute__((packed));
  
static const struct RGBAPixel RGBAPixel_Black     = { 0x00, 0x00, 0x00, 0xFF };
static const struct RGBAPixel RGBAPixel_White     = { 0xFF, 0xFF, 0xFF, 0xFF };
static const struct RGBAPixel RGBAPixel_Red       = { 0x00, 0x00, 0xFF, 0xFF };
static const struct RGBAPixel RGBAPixel_Green     = { 0x00, 0xFF, 0x00, 0xFF };
static const struct RGBAPixel RGBAPixel_Blue      = { 0xFF, 0x00, 0x00, 0xFF };


struct BGRAPixel { uint8_t r,  g, b,  a; OP32 } __attribute__((packed));

static const struct BGRAPixel BGRAPixel_Black     = { 0x00, 0x00, 0x00, 0xFF };
static const struct BGRAPixel BGRAPixel_White     = { 0xFF, 0xFF, 0xFF, 0xFF };
static const struct BGRAPixel BGRAPixel_Red       = { 0xFF, 0x00, 0x00, 0xFF };
static const struct BGRAPixel BGRAPixel_Green     = { 0x00, 0xFF, 0x00, 0xFF };
static const struct BGRAPixel BGRAPixel_Blue      = { 0x00, 0x00, 0xFF, 0xFF };


struct RGBPixel { uint8_t b,  g, r; OP24 } __attribute__((packed));

static const struct RGBPixel RGBPixel_Black       = { 0x00, 0x00, 0x00 };
static const struct RGBPixel RGBPixel_White       = { 0xFF, 0xFF, 0xFF };
static const struct RGBPixel RGBPixel_Red         = { 0x00, 0x00, 0xFF };
static const struct RGBPixel RGBPixel_Green       = { 0x00, 0xFF, 0x00 };
static const struct RGBPixel RGBPixel_Blue        = { 0xFF, 0x00, 0x00 };


struct BGRPixel { uint8_t r,  g, b; OP24 } __attribute__((packed));

static const struct BGRPixel BGRPixel_Black       = { 0x00, 0x00, 0x00 };
static const struct BGRPixel BGRPixel_White       = { 0xFF, 0xFF, 0xFF };
static const struct BGRPixel BGRPixel_Red         = { 0xFF, 0x00, 0x00 };
static const struct BGRPixel BGRPixel_Green       = { 0x00, 0xFF, 0x00 };
static const struct BGRPixel BGRPixel_Blue        = { 0x00, 0x00, 0xFF };


struct RGB555Pixel { unsigned b:5, g:5, r:5, res:1; OP16 } __attribute__((packed));

static const struct RGB555Pixel RGB555Pixel_Black = { 0x00, 0x00, 0x00, 0 };
static const struct RGB555Pixel RGB555Pixel_White = { 0x1F, 0x1F, 0x1F, 0 };


struct BGR555Pixel { unsigned r:5, g:5, b:5, res:1; OP16 } __attribute__((packed));

static const struct BGR555Pixel BGR555Pixel_Black = { 0x00, 0x00, 0x00, 0 };
static const struct BGR555Pixel BGR555Pixel_White = { 0x1F, 0x1F, 0x1F, 0 };


struct RGB565Pixel { unsigned b:5, g:6, r:5; OP16 } __attribute__((packed));

static const struct RGB565Pixel RGB565Pixel_Black = { 0x00, 0x00, 0x00 };
static const struct RGB565Pixel RGB565Pixel_White = { 0x1F, 0x3F, 0x1F };


struct BGR565Pixel { unsigned r:5, g:6, b:5; OP16 } __attribute__((packed));

static const struct BGR565Pixel BGR565Pixel_Black = { 0x00, 0x00, 0x00 };
static const struct BGR565Pixel BGR565Pixel_White = { 0x1F, 0x3F, 0x1F };


struct YUYVPixel { uint8_t y0, u, y1, v; OP32 } __attribute__((packed));

static const struct YUYVPixel YUYVPixel_Black     = { 0x00, 0x7F, 0x00, 0x7F };
static const struct YUYVPixel YUYVPixel_White     = { 0xFF, 0x7F, 0xFF, 0x7F };


struct UYVYPixel { uint8_t u, y0, v, y1; OP32 } __attribute__((packed));

static const struct UYVYPixel UYVYPixel_Black     = { 0x7F, 0x00, 0x7F, 0x00 };
static const struct UYVYPixel UYVYPixel_White     = { 0x7F, 0xFF, 0x7F, 0xFF };


struct YUVAPixel { uint8_t y, u, v, a; OP32 } __attribute__((packed));

static const struct YUVAPixel YUVAPixel_Black     = { 0x00, 0x7F, 0x7F, 0xFF };
static const struct YUVAPixel YUVAPixel_White     = { 0xFF, 0x7F, 0x7F, 0xFF };


struct YUVPixel { uint8_t y, u, v; OP24 } __attribute__((packed));

static const struct YUVPixel YUVPixel_Black       = { 0x00, 0x7F, 0x7F };
static const struct YUVPixel YUVPixel_White       = { 0xFF, 0x7F, 0x7F };


#undef OP16
#undef OP24
#undef OP32


inline struct YUVAPixel RGBA2YUVA(struct RGBAPixel p)
{
  struct YUVAPixel result;

  result.y = (uint8_t)(((( 66 * (int)(p.r)) + (129 * (int)(p.g)) + ( 25 * (int)(p.b)) + 128) >> 8) +  16);
  result.u = (uint8_t)((((-38 * (int)(p.r)) - ( 74 * (int)(p.g)) + (112 * (int)(p.b)) + 128) >> 8) + 128);
  result.v = (uint8_t)((((112 * (int)(p.r)) - ( 94 * (int)(p.g)) - ( 18 * (int)(p.b)) + 128) >> 8) + 128);
  result.a = p.a;

  return result;
}

inline struct YUVPixel RGB2YUV(struct RGBPixel p)
{
  struct YUVPixel result;

  result.y = (uint8_t)(((( 66 * (int)(p.r)) + (129 * (int)(p.g)) + ( 25 * (int)(p.b)) + 128) >> 8) +  16);
  result.u = (uint8_t)((((-38 * (int)(p.r)) - ( 74 * (int)(p.g)) + (112 * (int)(p.b)) + 128) >> 8) + 128);
  result.v = (uint8_t)((((112 * (int)(p.r)) - ( 94 * (int)(p.g)) - ( 18 * (int)(p.b)) + 128) >> 8) + 128);

  return result;
}


struct YUVConstData
{
  const uint8_t * y, * u, * v;
  unsigned yStride, uStride, vStride;
  int wr, hr;

#ifdef __cplusplus
  inline static YUVConstData fromVideoBuffer(const SVideoBuffer &buffer)
  {
    YUVConstData result;
    if (buffer.format().isYUV())
    {
      result.y = reinterpret_cast<const uint8_t *>(buffer.scanLine(0, 0));
      result.u = reinterpret_cast<const uint8_t *>(buffer.scanLine(0, 1));
      result.v = reinterpret_cast<const uint8_t *>(buffer.scanLine(0, 2));
      result.yStride = buffer.lineSize(0);
      result.uStride = buffer.lineSize(1);
      result.vStride = buffer.lineSize(2);

      result.wr = result.hr = 1;
      buffer.format().planarYUVRatio(result.wr, result.hr);
    }
    else
    {
      result.y = result.u = result.v = NULL;
      result.yStride = result.uStride = result.vStride = 0;
      result.wr = result.hr = 0;
    }

    return result;
  }
#endif
};

struct YUVData
{
  uint8_t * y, * u, * v;
  unsigned yStride, uStride, vStride;
  int wr, hr;

#ifdef __cplusplus
  inline static YUVData fromVideoBuffer(SVideoBuffer &buffer)
  {
    YUVData result;
    if (buffer.format().isYUV())
    {
      result.y = reinterpret_cast<uint8_t *>(buffer.scanLine(0, 0));
      result.u = reinterpret_cast<uint8_t *>(buffer.scanLine(0, 1));
      result.v = reinterpret_cast<uint8_t *>(buffer.scanLine(0, 2));
      result.yStride = buffer.lineSize(0);
      result.uStride = buffer.lineSize(1);
      result.vStride = buffer.lineSize(2);

      result.wr = result.hr = 1;
      buffer.format().planarYUVRatio(result.wr, result.hr);
    }
    else
    {
      result.y = result.u = result.v = NULL;
      result.yStride = result.uStride = result.vStride = 0;
      result.wr = result.hr = 0;
    }

    return result;
  }
#endif
};

#ifdef __cplusplus
} } // End of namespaces
#endif

#endif
