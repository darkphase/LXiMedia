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
#include "../../algorithms/videoconvert.h"

// Implemented in videoformatconverter.demosaic.c
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_GRBG8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride);
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_GBRG8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride);
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_RGGB8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride);
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_BGGR8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride);
extern "C" void LXiStream_Common_VideoFormatConverter_demosaic_postfilter(
    void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines);

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
    {
      Algorithms::VideoConvert::YUYVtoRGB(
          reinterpret_cast<quint32 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_YUYV422::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_RGB32))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUYV422,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
    {
      Algorithms::VideoConvert::RGBtoYUYV(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

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
    {
      Algorithms::VideoConvert::UYVYtoRGB(
          reinterpret_cast<quint32 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_UYVY422::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_RGB32))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_UYVY422,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
    {
      Algorithms::VideoConvert::RGBtoUYVY(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

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
    {
      Algorithms::VideoConvert::BGRtoRGB(
          reinterpret_cast<quint32 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

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
    {
      Algorithms::VideoConvert::BGRtoRGB(
          reinterpret_cast<quint32 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

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
    {
      Algorithms::VideoConvert::YUV2toRGB(
          reinterpret_cast<quint32 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y >> 1, 1)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y >> 1, 2)),
          w);
    }

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_YUV420P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_RGB32))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUV420P,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y+=2)
    {
      quint8 _lxi_align ulinea[w / 2];
      quint8 _lxi_align vlinea[w / 2];

      Algorithms::VideoConvert::RGBtoYUV2(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 0)),
          ulinea,
          vlinea,
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(y, 0)),
          w);

      quint8 _lxi_align ulineb[w / 2];
      quint8 _lxi_align vlineb[w / 2];

      Algorithms::VideoConvert::RGBtoYUV2(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y + 1, 0)),
          ulineb,
          vlineb,
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(y + 1, 0)),
          w);

      Algorithms::VideoConvert::mergeUVlines(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y / 2, 1)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y / 2, 2)),
          ulinea, ulineb,
          vlinea, vlineb,
          w / 2);
    }

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
    {
      Algorithms::VideoConvert::YUV2toRGB(
          reinterpret_cast<quint32 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 1)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 2)),
          w);
    }

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_YUV422P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_RGB32))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUV422P,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
    {
      Algorithms::VideoConvert::RGBtoYUV2(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 1)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 2)),
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

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
    {
      Algorithms::VideoConvert::YUV1toRGB(
          reinterpret_cast<quint32 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 0)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 1)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 2)),
          w);
    }

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_YUV444P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format() == SVideoFormat::Format_RGB32))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUV444P,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
    {
      Algorithms::VideoConvert::RGBtoYUV1(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 1)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 2)),
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

template class VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUYV422>;
template class VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_UYVY422>;
template class VideoFormatConverterBase<SVideoFormat::Format_BGR32, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_BGR32>;
template class VideoFormatConverterBase<SVideoFormat::Format_YUV420P, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUV420P>;
template class VideoFormatConverterBase<SVideoFormat::Format_YUV422P, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUV422P>;
template class VideoFormatConverterBase<SVideoFormat::Format_YUV444P, SVideoFormat::Format_RGB32>;
template class VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUV444P>;

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
                                         SSize(videoBuffer.format().size().width(),
                                               videoBuffer.format().size().height() & ~1u,
                                               videoBuffer.format().size().aspectRatio()),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = destBuffer.format().size().width(), h = destBuffer.format().size().height();
    for (int y=0; y<h; y+=2)
    {
      quint8 _lxi_align ulinea[w / 2];
      quint8 _lxi_align vlinea[w / 2];

      Algorithms::VideoConvert::YUYVtoYUV2(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 0)),
          ulinea,
          vlinea,
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 0)),
          w);

      quint8 _lxi_align ulineb[w / 2];
      quint8 _lxi_align vlineb[w / 2];

      Algorithms::VideoConvert::YUYVtoYUV2(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y + 1, 0)),
          ulineb,
          vlineb,
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y + 1, 0)),
          w);

      Algorithms::VideoConvert::mergeUVlines(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y / 2, 1)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y / 2, 2)),
          ulinea, ulineb,
          vlinea, vlineb,
          w / 2);
    }

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
                                         SSize(videoBuffer.format().size().width(),
                                               videoBuffer.format().size().height() & ~1u,
                                               videoBuffer.format().size().aspectRatio()),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = destBuffer.format().size().width(), h = destBuffer.format().size().height();
    for (int y=0; y<h; y+=2)
    {
      quint8 _lxi_align ulinea[w / 2];
      quint8 _lxi_align vlinea[w / 2];

      Algorithms::VideoConvert::UYVYtoYUV2(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 0)),
          ulinea,
          vlinea,
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 0)),
          w);

      quint8 _lxi_align ulineb[w / 2];
      quint8 _lxi_align vlineb[w / 2];

      Algorithms::VideoConvert::UYVYtoYUV2(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y + 1, 0)),
          ulineb,
          vlineb,
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y + 1, 0)),
          w);

      Algorithms::VideoConvert::mergeUVlines(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y / 2, 1)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y / 2, 2)),
          ulinea, ulineb,
          vlinea, vlineb,
          w / 2);
    }

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

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
    {
      Algorithms::VideoConvert::YUYVtoYUV2(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 1)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 2)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

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

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
    {
      Algorithms::VideoConvert::UYVYtoYUV2(
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 0)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 1)),
          reinterpret_cast<quint8 *>(destBuffer.scanLine(y, 2)),
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(y, 0)),
          w);
    }

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
