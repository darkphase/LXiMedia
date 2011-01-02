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
namespace SPixels {

#define OP16 inline operator uint16_t() const { return *reinterpret_cast<const uint16_t *>(this); }
#define OP24 inline operator uint32_t() const { return *reinterpret_cast<const uint32_t *>(this) | 0xFF000000u; }
#define OP32 inline operator uint32_t() const { return *reinterpret_cast<const uint32_t *>(this); }
#else
#define OP16;
#define OP24;
#define OP32;
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

#undef OP32
#ifdef __cplusplus
} } // End of namespaces
#endif

#endif
