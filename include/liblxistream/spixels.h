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
namespace LXiStream {

class SVideoBuffer;

namespace SPixels {

#define OP16 uint16_t pack; inline operator uint16_t() const { return pack; }
#define OP32 uint32_t pack; inline operator uint32_t() const { return pack; }
#define EXTC extern "C"
#else
#define OP16 uint16_t pack;
#define OP32 uint32_t pack;
#define EXTC
#endif

#define PACK __attribute__((packed))

typedef union { struct { uint8_t b, g, r, a; } PACK; OP32 } PACK RGBAPixel;
  
static const RGBAPixel RGBAPixel_Black     = { { 0x00, 0x00, 0x00, 0xFF } };
static const RGBAPixel RGBAPixel_White     = { { 0xFF, 0xFF, 0xFF, 0xFF } };
static const RGBAPixel RGBAPixel_Red       = { { 0x00, 0x00, 0xFF, 0xFF } };
static const RGBAPixel RGBAPixel_Green     = { { 0x00, 0xFF, 0x00, 0xFF } };
static const RGBAPixel RGBAPixel_Blue      = { { 0xFF, 0x00, 0x00, 0xFF } };


typedef union { struct { uint8_t r, g, b, a; } PACK; OP32 } PACK BGRAPixel;

static const BGRAPixel BGRAPixel_Black     = { { 0x00, 0x00, 0x00, 0xFF } };
static const BGRAPixel BGRAPixel_White     = { { 0xFF, 0xFF, 0xFF, 0xFF } };
static const BGRAPixel BGRAPixel_Red       = { { 0xFF, 0x00, 0x00, 0xFF } };
static const BGRAPixel BGRAPixel_Green     = { { 0x00, 0xFF, 0x00, 0xFF } };
static const BGRAPixel BGRAPixel_Blue      = { { 0x00, 0x00, 0xFF, 0xFF } };


typedef union { struct { unsigned b:5, g:5, r:5, res:1; } PACK; OP16 } PACK RGB555Pixel;

static const RGB555Pixel RGB555Pixel_Black = { { 0x00, 0x00, 0x00, 0 } };
static const RGB555Pixel RGB555Pixel_White = { { 0x1F, 0x1F, 0x1F, 0 } };


typedef union { struct { unsigned r:5, g:5, b:5, res:1; } PACK; OP16 } PACK BGR555Pixel;

static const BGR555Pixel BGR555Pixel_Black = { { 0x00, 0x00, 0x00, 0 } };
static const BGR555Pixel BGR555Pixel_White = { { 0x1F, 0x1F, 0x1F, 0 } };


typedef union { struct { unsigned b:5, g:6, r:5; } PACK; OP16 } PACK RGB565Pixel;

static const RGB565Pixel RGB565Pixel_Black = { { 0x00, 0x00, 0x00 } };
static const RGB565Pixel RGB565Pixel_White = { { 0x1F, 0x3F, 0x1F } };


typedef union { struct { unsigned r:5, g:6, b:5; } PACK; OP16 } PACK BGR565Pixel;

static const BGR565Pixel BGR565Pixel_Black = { { 0x00, 0x00, 0x00 } };
static const BGR565Pixel BGR565Pixel_White = { { 0x1F, 0x3F, 0x1F } };


typedef union { struct { uint8_t y0, u, y1, v; } PACK; OP32 } PACK YUYVPixel;

static const YUYVPixel YUYVPixel_Black     = { { 0x00, 0x7F, 0x00, 0x7F } };
static const YUYVPixel YUYVPixel_White     = { { 0xFF, 0x7F, 0xFF, 0x7F } };


typedef union { struct { uint8_t u, y0, v, y1; } PACK; OP32 } PACK UYVYPixel;

static const UYVYPixel UYVYPixel_Black     = { { 0x7F, 0x00, 0x7F, 0x00 } };
static const UYVYPixel UYVYPixel_White     = { { 0x7F, 0xFF, 0x7F, 0xFF } };


typedef union { struct { uint8_t y, u, v, a; } PACK; OP32 } PACK YUVAPixel;

static const YUVAPixel YUVAPixel_Black     = { { 0x00, 0x7F, 0x7F, 0xFF } };
static const YUVAPixel YUVAPixel_White     = { { 0xFF, 0x7F, 0x7F, 0xFF } };

EXTC YUVAPixel RGBA2YUVA(RGBAPixel p);

struct YUVConstData
{
#ifdef __cplusplus
  YUVConstData(const SVideoBuffer &buffer);
#endif

  const uint8_t * y, * u, * v;
  unsigned yStride, uStride, vStride;
  int wr, hr;
};

struct YUVData
{
#ifdef __cplusplus
  YUVData(SVideoBuffer &buffer);
#endif

  uint8_t * y, * u, * v;
  unsigned yStride, uStride, vStride;
  int wr, hr;
};

#undef PACK

#ifdef __cplusplus
} } // End of namespaces
#endif

#endif
