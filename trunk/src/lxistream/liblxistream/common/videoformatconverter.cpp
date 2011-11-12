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

template <SVideoFormat::Format _srcFormat, SVideoFormat::Format _dstFormat>
template <typename _srcType, typename _dstType>
_lxi_always_inline QFuture<void> VideoFormatConverterBase<_srcFormat, _dstFormat>::convertSlice(
    void(* func)(_dstType *, const _srcType *, int),
    const Buffers &buffers,
    int top, int bottom,
    int)
{
  struct T
  {
    static void processSlice(void(* func)(_dstType *, const _srcType *, int), const Buffers *buffers, int top, int bottom)
    {
      LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

      for (int y=top; y<bottom; y++)
      {
        func(
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y, 0)),
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y, 0)),
            buffers->width);
      }
    }
  };

  return QtConcurrent::run(&T::processSlice, func, &buffers, top, bottom);
}

template <SVideoFormat::Format _srcFormat, SVideoFormat::Format _dstFormat>
template <typename _srcType, typename _dstType>
_lxi_always_inline QFuture<void> VideoFormatConverterBase<_srcFormat, _dstFormat>::convertSlice(
    void(* func)(_dstType *, const _srcType *, const _srcType *, const _srcType *, int),
    const Buffers &buffers,
    int top, int bottom,
    int subsample)
{
  struct T
  {
    static void processSlice0(void(* func)(_dstType *, const _srcType *, const _srcType *, const _srcType *, int), const Buffers *buffers, int top, int bottom)
    {
      LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

      for (int y=top; y<bottom; y++)
      {
        func(
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y, 0)),
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y, 0)),
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y, 1)),
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y, 2)),
            buffers->width);
      }
    }

    static void processSlice1(void(* func)(_dstType *, const _srcType *, const _srcType *, const _srcType *, int), const Buffers *buffers, int top, int bottom)
    {
      LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

      for (int y=top; y<bottom; y++)
      {
        func(
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y, 0)),
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y, 0)),
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y / 2, 1)),
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y / 2, 2)),
            buffers->width);
      }
    }
  };

  if (subsample == 0)
    return QtConcurrent::run(&T::processSlice0, func, &buffers, top, bottom);
  else
    return QtConcurrent::run(&T::processSlice1, func, &buffers, top, bottom);
}

template <SVideoFormat::Format _srcFormat, SVideoFormat::Format _dstFormat>
template <typename _srcType, typename _dstType>
_lxi_always_inline QFuture<void> VideoFormatConverterBase<_srcFormat, _dstFormat>::convertSlice(
    void(* func)(_dstType *, _dstType *, _dstType *, const _srcType *, int),
    const Buffers &buffers,
    int top, int bottom,
    int subsample)
{
  struct T
  {
    static void processSlice0(void(* func)(_dstType *, _dstType *, _dstType *, const _srcType *, int), const Buffers *buffers, int top, int bottom)
    {
      LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

      for (int y=top; y<bottom; y++)
      {
        func(
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y, 0)),
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y, 1)),
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y, 2)),
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y, 0)),
            buffers->width);
      }
    }

    static void processSlice1(void(* func)(_dstType *, _dstType *, _dstType *, const _srcType *, int), const Buffers *buffers, int top, int bottom)
    {
      LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

      _dstType _lxi_align ulinea[buffers->width / 2];
      _dstType _lxi_align vlinea[buffers->width / 2];
      _dstType _lxi_align ulineb[buffers->width / 2];
      _dstType _lxi_align vlineb[buffers->width / 2];

      for (int y=top; y<bottom; y+=2)
      {
        func(
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y, 0)),
            ulinea,
            vlinea,
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y, 0)),
            buffers->width);

        func(
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y + 1, 0)),
            ulineb,
            vlineb,
            reinterpret_cast<const _srcType *>(buffers->src->scanLine(y + 1, 0)),
            buffers->width);

        Algorithms::VideoConvert::mergeUVlines(
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y / 2, 1)),
            reinterpret_cast<_dstType *>(buffers->dst->scanLine(y / 2, 2)),
            ulinea, ulineb,
            vlinea, vlineb,
            buffers->width / 2);
      }
    }
  };

  if (subsample == 0)
    return QtConcurrent::run(&T::processSlice0, func, &buffers, top, bottom);
  else
    return QtConcurrent::run(&T::processSlice1, func, &buffers, top, bottom);
}

template <SVideoFormat::Format _srcFormat, SVideoFormat::Format _dstFormat>
template <typename _funcType>
_lxi_always_inline SVideoBuffer VideoFormatConverterBase<_srcFormat, _dstFormat>::convert(
    _funcType func,
    const SVideoBuffer &srcBuffer,
    int subsample)
{
  if ((srcBuffer.format() == _srcFormat))
  {
    SVideoBuffer dstBuffer(SVideoFormat(
        _dstFormat,
        srcBuffer.format().size(),
        srcBuffer.format().frameRate(),
        srcBuffer.format().fieldMode()));

    const int threadCount = QThread::idealThreadCount();
    const Buffers buffers = { &dstBuffer, &srcBuffer, srcBuffer.format().size().width() };
    const int h = srcBuffer.format().size().height();
    const int sh = (((h + threadCount - 1) / threadCount) + 1) & ~1;

    QVector< QFuture<void> > future;
    future.reserve(threadCount);

    for (int y=0; y<h; y+=sh)
      future += convertSlice(func, buffers, y, qMin(y + sh, h), subsample);

    for (int i=0; i<future.count(); i++)
      future[i].waitForFinished();

    dstBuffer.setTimeStamp(srcBuffer.timeStamp());

    return dstBuffer;
  }

  return SVideoBuffer();
}


SVideoBuffer VideoFormatConverter_Format_YUYV422_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::YUYVtoRGB, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_YUYV422::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::RGBtoYUYV, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_UYVY422_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::UYVYtoRGB, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_UYVY422::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::RGBtoUYVY, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_BGR32_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::BGRtoRGB, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_BGR32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::BGRtoRGB, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_YUV420P_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::YUV2toRGB, videoBuffer, 1);
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_YUV420P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::RGBtoYUV2, videoBuffer, 1);
}

SVideoBuffer VideoFormatConverter_Format_YUV422P_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::YUV2toRGB, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_YUV422P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::RGBtoYUV2, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_YUV444P_Format_RGB32::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::YUV1toRGB, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_RGB32_Format_YUV444P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::RGBtoYUV1, videoBuffer);
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
  return convert(&Algorithms::VideoConvert::YUYVtoYUV2, videoBuffer, 1);
}

SVideoBuffer VideoFormatConverter_Format_UYVY422_Format_YUV420P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::UYVYtoYUV2, videoBuffer, 1);
}

SVideoBuffer VideoFormatConverter_Format_YUYV422_Format_YUV422P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::YUYVtoYUV2, videoBuffer);
}

SVideoBuffer VideoFormatConverter_Format_UYVY422_Format_YUV422P::convertBuffer(const SVideoBuffer &videoBuffer)
{
  return convert(&Algorithms::VideoConvert::UYVYtoYUV2, videoBuffer);
}

template class VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_YUV420P>;
template class VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_YUV420P>;
template class VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_YUV422P>;
template class VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_YUV422P>;

} } // End of namespaces
