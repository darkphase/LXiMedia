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

#include <sys/types.h>
#include <stdint.h>

namespace LXiStream {
namespace Algorithms {

class VideoConvert
{
public:
  static void YUYVtoYUV2(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint8_t * src, int n);
  static void UYVYtoYUV2(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint8_t * src, int n);
  static void mergeUVlines(uint8_t *dstu, uint8_t *dstv, const uint8_t * srcua, const uint8_t * srcub, const uint8_t * srcva, const uint8_t * srcvb, int n);

  static void YUYVtoRGB(uint32_t *dst, const uint8_t *src, int n);
  static void UYVYtoRGB(uint32_t *dst, const uint8_t *src, int n);
  static void YUV1toRGB(uint32_t *dst, const uint8_t *srcy, const uint8_t *srcu, const uint8_t *srcv, int n);
  static void YUV2toRGB(uint32_t *dst, const uint8_t *srcy, const uint8_t *srcu, const uint8_t *srcv, int n);

  static void RGBtoYUYV(uint8_t *dst, const uint32_t *src, int n);
  static void RGBtoUYVY(uint8_t *dst, const uint32_t *src, int n);
  static void RGBtoYUV1(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint32_t *src, int n);
  static void RGBtoYUV2(uint8_t *dsty, uint8_t *dstu, uint8_t *dstv, const uint32_t *src, int n);

  static void BGRtoRGB(uint32_t *dst, const uint32_t *src, int n);

  static void demosaicGRBG8(
      const uint8_t * src, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
      uint8_t * dst, unsigned dstStride);

  static void demosaicGBRG8(
      const uint8_t * src, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
      uint8_t * dst, unsigned dstStride);

  static void demosaicBGGR8(
      const uint8_t * src, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
      uint8_t * dst, unsigned dstStride);

  static void demosaicRGGB8(
      const uint8_t * src, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
      uint8_t * dst, unsigned dstStride);

  static void demosaicPostfilter(uint8_t * data, unsigned width, unsigned stride, unsigned numLines);

private:
  struct RGBAPixel;
};

} } // End of namespaces
