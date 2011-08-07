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

#include "svideoformat.h"
#include "spixels.h"

namespace LXiStream {

SVideoFormat::SVideoFormat(void)
{
  Q_ASSERT(sizeof(*this) == sizeof(d));

  d.format = Format_Invalid;
  d.size = SSize();
  d.frameRate = SInterval();
  d.fieldMode = FieldMode_Invalid;
}

/*! Initializes this codec with the specified format and field mode.

    \sa setFormat()
 */
SVideoFormat::SVideoFormat(Format format, SSize size, SInterval frameRate, FieldMode fieldMode)
{
  Q_ASSERT(sizeof(*this) == sizeof(d));

  setFormat(format, size, frameRate, fieldMode);
}

bool SVideoFormat::operator==(const SVideoFormat &comp) const
{
  return (d.format == comp.d.format) && (d.size == comp.d.size) &&
      qFuzzyCompare(d.frameRate.toFrequency(), comp.d.frameRate.toFrequency()) &&
      (d.fieldMode == comp.d.fieldMode);
}

/*! Sets this codec to the specified video format.
 */
void SVideoFormat::setFormat(Format format, SSize size, SInterval frameRate, FieldMode fieldMode)
{
  d.format = format;
  d.size = size;
  d.frameRate = frameRate;
  d.fieldMode = fieldMode;
}

/*! Returns the size of one sample in bytes of the format, or 0 if not
    applicable. For example for Format_RGB32 this method returns 4.
 */
int SVideoFormat::sampleSize(Format format)
{
  switch (format)
  {
  // Video formats
  case Format_GRAY8:
    return 1;

  case Format_GRAY16BE:
  case Format_GRAY16LE:
  case Format_RGB555:
  case Format_BGR555:
  case Format_RGB565:
  case Format_BGR565:
    return 2;

  case Format_RGB24:
  case Format_BGR24:
    return 3;

  case Format_RGB32:
  case Format_BGR32:
    return 4;

  case Format_YUYV422:
  case Format_UYVY422:
    return 2;

  case Format_BGGR8:
  case Format_GBRG8:
  case Format_GRBG8:
  case Format_RGGB8:
    return 1;

  case Format_BGGR10:
  case Format_GBRG10:
  case Format_GRBG10:
  case Format_RGGB10:
  case Format_BGGR16:
  case Format_GBRG16:
  case Format_GRBG16:
  case Format_RGGB16:
    return 2;

  default:
    return 1;
  }
}

int SVideoFormat::numPlanes(Format format)
{
  switch (format)
  {
  case Format_RGB555:
  case Format_BGR555:
  case Format_RGB565:
  case Format_BGR565:
  case Format_RGB24:
  case Format_BGR24:
  case Format_RGB32:
  case Format_BGR32:
  case Format_GRAY8:
  case Format_GRAY16BE:
  case Format_GRAY16LE:
  case Format_YUYV422:
  case Format_UYVY422:
    return 1;

  case Format_YUV410P:
  case Format_YUV411P:
  case Format_YUV420P:
  case Format_YUV422P:
  case Format_YUV444P:
    return 3;

  case Format_BGGR8:
  case Format_GBRG8:
  case Format_GRBG8:
  case Format_RGGB8:
  case Format_BGGR10:
  case Format_GBRG10:
  case Format_GRBG10:
  case Format_RGGB10:
  case Format_BGGR16:
  case Format_GBRG16:
  case Format_GRBG16:
  case Format_RGGB16:
    return 1;

  default:
    return 0;
  }
}

/*! Sets w and h to the width and height Cr/Cb sample ratios of the format or 0
    if not applicable. For example for Format_YUV422P w = 2 and h = 1.

    \returns True if the values w and h are set to a non zero value, false
             otherwise.
 */
bool SVideoFormat::planarYUVRatio(Format format, int &w, int &h)
{
  switch (format)
  {
  case Format_YUV410P:        w = 4; h = 4; return true;
  case Format_YUV411P:        w = 4; h = 1; return true;
  case Format_YUV420P:        w = 2; h = 2; return true;
  case Format_YUV422P:        w = 2; h = 1; return true;
  case Format_YUV444P:        w = 1; h = 1; return true;
  default:                    w = 0; h = 0; return false;
  }
}

quint32 SVideoFormat::nullPixelValue(Format format)
{
  switch (format)
  {
  case Format_RGB32:        return SPixels::RGBAPixel_Black;
  case Format_BGR32:        return SPixels::BGRAPixel_Black;
  case Format_RGB24:        return 0;//SPixels::RGBPixel_Black;
  case Format_BGR24:        return 0;//SPixels::BGRPixel_Black;
  case Format_RGB555:       return (quint32(SPixels::RGB555Pixel_Black) << 16) | quint32(SPixels::RGB555Pixel_Black);
  case Format_BGR555:       return (quint32(SPixels::BGR555Pixel_Black) << 16) | quint32(SPixels::BGR555Pixel_Black);
  case Format_RGB565:       return (quint32(SPixels::RGB565Pixel_Black) << 16) | quint32(SPixels::RGB565Pixel_Black);
  case Format_BGR565:       return (quint32(SPixels::BGR565Pixel_Black) << 16) | quint32(SPixels::BGR565Pixel_Black);

  case Format_GRAY8:        return 0;
  case Format_GRAY16BE:     return 0;
  case Format_GRAY16LE:     return 0;

  case Format_YUYV422:      return SPixels::YUYVPixel_Black;
  case Format_UYVY422:      return SPixels::UYVYPixel_Black;

  case Format_BGGR8:
  case Format_GBRG8:
  case Format_GRBG8:
  case Format_RGGB8:
  case Format_BGGR10:
  case Format_GBRG10:
  case Format_GRBG10:
  case Format_RGGB10:
  case Format_BGGR16:
  case Format_GBRG16:
  case Format_GRBG16:
  case Format_RGGB16:
  default:
    return 0;
  }
}

/*! Returns the name of the format, e.g. "RGB/555"
 */
const char * SVideoFormat::formatName(Format format)
{
  switch (format)
  {
  case Format_RGB555:                 return "RGB/555";
  case Format_BGR555:                 return "BGR/555";
  case Format_RGB565:                 return "RGB/565";
  case Format_BGR565:                 return "BGR/565";
  case Format_RGB24:                  return "RGB/24";
  case Format_BGR24:                  return "BGR/24";
  case Format_RGB32:                  return "RGB/32";
  case Format_BGR32:                  return "BGR/32";
  case Format_GRAY8:                  return "GRAY/8";
  case Format_GRAY16BE:               return "GRAY/16be";
  case Format_GRAY16LE:               return "GRAY/16le";
  case Format_YUYV422:                return "YUYV/422";
  case Format_UYVY422:                return "UYVY/422";
  case Format_YUV410P:                return "YUV/410 planar";
  case Format_YUV411P:                return "YUV/411 planar";
  case Format_YUV420P:                return "YUV/420 planar";
  case Format_YUV422P:                return "YUV/422 planar";
  case Format_YUV444P:                return "YUV/444 planar";
  case Format_BGGR8:                  return "Bayer/BGGR8";
  case Format_GBRG8:                  return "Bayer/GBRG8";
  case Format_GRBG8:                  return "Bayer/GRBG8";
  case Format_RGGB8:                  return "Bayer/RGGB8";
  case Format_BGGR10:                 return "Bayer/BGGR10";
  case Format_GBRG10:                 return "Bayer/GBRG10";
  case Format_GRBG10:                 return "Bayer/GRBG10";
  case Format_RGGB10:                 return "Bayer/RGGB10";
  case Format_BGGR16:                 return "Bayer/BGGR16";
  case Format_GBRG16:                 return "Bayer/GBRG16";
  case Format_GRBG16:                 return "Bayer/GRBG16";
  case Format_RGGB16:                 return "Bayer/RGGB16";

  default:                            return "";
  }
}

} // End of namespace
