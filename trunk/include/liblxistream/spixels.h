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
#endif
  
struct RGBAPixel { uint8_t b,  g, r,  a; } __attribute__((packed));
  
static const struct RGBAPixel RGBAPixel_Black     = { 0x00, 0x00, 0x00, 0xFF };
static const struct RGBAPixel RGBAPixel_White     = { 0xFF, 0xFF, 0xFF, 0xFF };
static const struct RGBAPixel RGBAPixel_Red       = { 0x00, 0x00, 0xFF, 0xFF };
static const struct RGBAPixel RGBAPixel_Green     = { 0x00, 0xFF, 0x00, 0xFF };
static const struct RGBAPixel RGBAPixel_Blue      = { 0xFF, 0x00, 0x00, 0xFF };

struct YUYVPixel { uint8_t y0, u, y1, v; } __attribute__((packed));
struct UYVYPixel { uint8_t u, y0, v, y1; } __attribute__((packed));

#ifdef __cplusplus
} } // End of namespaces
#endif

#endif
