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

#include "videoformatconverter.h"

// Implemented in videoformatconverter.convert.c
extern "C" void LXiStream_Common_VideoFormatConverter_convertYUYVtoRGB(
    void *rgb, const void *yuv, unsigned numPixels);
extern "C" void LXiStream_Common_VideoFormatConverter_convertUYVYtoRGB(
    void *rgb, const void *yuv, unsigned numPixels);
extern "C" void LXiStream_Common_VideoFormatConverter_convertBGRtoRGB(
    void *rgb, const void *bgr, unsigned numPixels);
extern "C" void LXiStream_Common_VideoFormatConverter_convertYUV1toRGB(
    void *rgb, const void *y, const void *u, const void *v, unsigned numPixels);
extern "C" void LXiStream_Common_VideoFormatConverter_convertYUV2toRGB(
    void *rgb, const void *y, const void *u, const void *v, unsigned numPixels);

// Implemented in videoformatconverter.demosaic.c
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_GRBG8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_GBRG8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_RGGB8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_BGGR8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_postfilter(
    void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines) __attribute__((nonnull(1)));

// Implemented in videoformatconverter.unpack.c
extern "C" void LXiStream_Common_VideoFormatConverter_convertYUYVtoYUV422P(
    const void *, size_t, unsigned, unsigned, void *, size_t, void *, size_t, void *, size_t);
extern "C" void LXiStream_Common_VideoFormatConverter_convertUYVYtoYUV422P(
    const void *, size_t, unsigned, unsigned, void *, size_t, void *, size_t, void *, size_t);
extern "C" void LXiStream_Common_VideoFormatConverter_convertYUYVtoYUV420P(
    const void *, size_t, unsigned, unsigned, void *, size_t, void *, size_t, void *, size_t);
extern "C" void LXiStream_Common_VideoFormatConverter_convertUYVYtoYUV420P(
    const void *, size_t, unsigned, unsigned, void *, size_t, void *, size_t, void *, size_t);

namespace LXiStream {
namespace Common {

template <SVideoFormat::Format _srcFormat, SVideoFormat::Format _dstFormat>
VideoFormatConverterBase<_srcFormat, _dstFormat>::VideoFormatConverterBase(const QString &, QObject *parent)
  : SInterfaces::VideoFormatConverter(parent)
{
}

template <SVideoFormat::Format _srcFormat, SVideoFormat::Format _dstFormat>
VideoFormatConverterBase<_srcFormat, _dstFormat>::~VideoFormatConverterBase()
{
}

template <SVideoFormat::Format _srcFormat, SVideoFormat::Format _dstFormat>
bool VideoFormatConverterBase<_srcFormat, _dstFormat>::openFormat(const SVideoFormat &srcFormat, const SVideoFormat &dstFormat)
{
  if ((srcFormat == _srcFormat) && (dstFormat == _dstFormat))
    return true;

  return false;
}

SVideoBuffer VideoFormatConverter_Format_YUYV422_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_YUYV422))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_Common_VideoFormatConverter_convertYUYVtoRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), w);

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_UYVY422_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_UYVY422))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_Common_VideoFormatConverter_convertUYVYtoRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), w);

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_BGR32_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_BGR32))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_Common_VideoFormatConverter_convertBGRtoRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), w);

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_BGR32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_RGB32))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_BGR32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_Common_VideoFormatConverter_convertBGRtoRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), w);

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_YUV420P_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_YUV420P))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_Common_VideoFormatConverter_convertYUV2toRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), videoBuffer.scanLine(y >> 1, 1), videoBuffer.scanLine(y >> 1, 2), w);

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_YUV422P_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_YUV422P))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_Common_VideoFormatConverter_convertYUV2toRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 1), videoBuffer.scanLine(y, 2), w);

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_YUV444P_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_YUV444P))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_Common_VideoFormatConverter_convertYUV1toRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 1), videoBuffer.scanLine(y, 2), w);

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

template class VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_BGR32, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_BGR32>;
template class VideoFormatConverterBase<SVideoFormat::Format_YUV420P, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_YUV422P, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_YUV444P, SVideoFormat::Format_RGB32>;

SVideoBuffer VideoFormatConverter_Format_GRBG8_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_GRBG8))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    LXiStream_Common_VideoFormatConverter_demosaic_GRBG8(
        videoBuffer.scanLine(0, 0),
        videoBuffer.format().size().width(),
        videoBuffer.lineSize(0),
        videoBuffer.format().size().height(),
        destBuffer.scanLine(0, 0),
        destBuffer.lineSize(0));

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_GBRG8_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_GBRG8))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    LXiStream_Common_VideoFormatConverter_demosaic_GBRG8(
        videoBuffer.scanLine(0, 0),
        videoBuffer.format().size().width(),
        videoBuffer.lineSize(0),
        videoBuffer.format().size().height(),
        destBuffer.scanLine(0, 0),
        destBuffer.lineSize(0));

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_RGGB8_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_RGGB8))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    LXiStream_Common_VideoFormatConverter_demosaic_RGGB8(
        videoBuffer.scanLine(0, 0),
        videoBuffer.format().size().width(),
        videoBuffer.lineSize(0),
        videoBuffer.format().size().height(),
        destBuffer.scanLine(0, 0),
        destBuffer.lineSize(0));

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_BGGR8_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_BGGR8))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    LXiStream_Common_VideoFormatConverter_demosaic_BGGR8(
        videoBuffer.scanLine(0, 0),
        videoBuffer.format().size().width(),
        videoBuffer.lineSize(0),
        videoBuffer.format().size().height(),
        destBuffer.scanLine(0, 0),
        destBuffer.lineSize(0));

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

template class VideoFormatConverterBase<SVideoFormat::Format_GRBG8, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_GBRG8, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_RGGB8, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_BGGR8, SVideoFormat::Format_RGB32>;

SVideoBuffer VideoFormatConverter_Format_YUYV422_Format_YUV420P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_YUYV422))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUV420P,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    LXiStream_Common_VideoFormatConverter_convertYUYVtoYUV420P(
        videoBuffer.scanLine(0, 0), videoBuffer.lineSize(0),
        videoBuffer.format().size().width(), videoBuffer.format().size().height(),
        destBuffer.scanLine(0, 0), destBuffer.lineSize(0),
        destBuffer.scanLine(0, 1), destBuffer.lineSize(1),
        destBuffer.scanLine(0, 2), destBuffer.lineSize(2));

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_UYVY422_Format_YUV420P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_UYVY422))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUV420P,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    LXiStream_Common_VideoFormatConverter_convertUYVYtoYUV420P(
        videoBuffer.scanLine(0, 0), videoBuffer.lineSize(0),
        videoBuffer.format().size().width(), videoBuffer.format().size().height(),
        destBuffer.scanLine(0, 0), destBuffer.lineSize(0),
        destBuffer.scanLine(0, 1), destBuffer.lineSize(1),
        destBuffer.scanLine(0, 2), destBuffer.lineSize(2));

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_YUYV422_Format_YUV422P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_YUYV422))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUV422P,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    LXiStream_Common_VideoFormatConverter_convertYUYVtoYUV422P(
        videoBuffer.scanLine(0, 0), videoBuffer.lineSize(0),
        videoBuffer.format().size().width(), videoBuffer.format().size().height(),
        destBuffer.scanLine(0, 0), destBuffer.lineSize(0),
        destBuffer.scanLine(0, 1), destBuffer.lineSize(1),
        destBuffer.scanLine(0, 2), destBuffer.lineSize(2));

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_UYVY422_Format_YUV422P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_UYVY422))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUV422P,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    LXiStream_Common_VideoFormatConverter_convertUYVYtoYUV422P(
        videoBuffer.scanLine(0, 0), videoBuffer.lineSize(0),
        videoBuffer.format().size().width(), videoBuffer.format().size().height(),
        destBuffer.scanLine(0, 0), destBuffer.lineSize(0),
        destBuffer.scanLine(0, 1), destBuffer.lineSize(1),
        destBuffer.scanLine(0, 2), destBuffer.lineSize(2));

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

template class VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_YUV420P>;
template class VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_YUV420P>;
template class VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_YUV422P>;
template class VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_YUV422P>;

} } // End of namespaces
