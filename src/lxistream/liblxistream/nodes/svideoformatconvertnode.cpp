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

#include "nodes/svideoformatconvertnode.h"
#include "sgraph.h"
#include "svideobuffer.h"

// Implemented in svideoformatconvertnode.convert.c
extern "C" void LXiStream_SVideoFormatConvertNode_convertYUYVtoRGB(
    void *rgb, const void *yuv, unsigned numPixels);
extern "C" void LXiStream_SVideoFormatConvertNode_convertUYVYtoRGB(
    void *rgb, const void *yuv, unsigned numPixels);
extern "C" void LXiStream_SVideoFormatConvertNode_convertBGRtoRGB(
    void *rgb, const void *bgr, unsigned numPixels);
extern "C" void LXiStream_SVideoFormatConvertNode_convertYUV1toRGB(
    void *rgb, const void *y, const void *u, const void *v, unsigned numPixels);
extern "C" void LXiStream_SVideoFormatConvertNode_convertYUV2toRGB(
    void *rgb, const void *y, const void *u, const void *v, unsigned numPixels);

// Implemented in svideoformatconvertnode.demosaic.c
extern "C" void LXiStream_SVideoFormatConvertNode_demosaic_GRBG8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_SVideoFormatConvertNode_demosaic_GBRG8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_SVideoFormatConvertNode_demosaic_RGGB8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_SVideoFormatConvertNode_demosaic_BGGR8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_SVideoFormatConvertNode_demosaic_postfilter(
    void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines) __attribute__((nonnull(1)));

// Implemented in svideoformatconvertnode.unpack.c
extern "C" void LXiStream_SVideoFormatConvertNode_convertYUYVtoYUV422P(
    const void *, unsigned, size_t, void *, void *, void *);
extern "C" void LXiStream_SVideoFormatConvertNode_convertUYVYtoYUV422P(
    const void *, unsigned, size_t, void *, void *, void *);
extern "C" void LXiStream_SVideoFormatConvertNode_convertYUYVtoYUV420P
    (const void *, unsigned, size_t, void *, void *, void *);
extern "C" void LXiStream_SVideoFormatConvertNode_convertUYVYtoYUV420P(
    const void *, unsigned, size_t, void *, void *, void *);

namespace LXiStream {

struct SVideoFormatConvertNode::Data
{
  SVideoFormat::Format          format;
};

SVideoFormatConvertNode::SVideoFormatConvertNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
}

SVideoFormatConvertNode::~SVideoFormatConvertNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SVideoFormatConvertNode::setFormat(SVideoFormat::Format format)
{
  d->format = format;
}

SVideoFormat::Format SVideoFormatConvertNode::format(void) const
{
  return d->format;
}

SVideoBuffer SVideoFormatConvertNode::convertYUVtoRGB(const SVideoBuffer &videoBuffer)
{
  SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                       videoBuffer.format().size(),
                                       videoBuffer.format().frameRate(),
                                       videoBuffer.format().fieldMode()));

  if (videoBuffer.format().format() == SVideoFormat::Format_YUYV422)
  {
    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertYUYVtoRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), w);

    return destBuffer;
  }
  else if (videoBuffer.format().format() == SVideoFormat::Format_UYVY422)
  {
    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertUYVYtoRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), w);

    return destBuffer;
  }
  else if (videoBuffer.format().format() == SVideoFormat::Format_YUV420P)
  {
    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertYUV2toRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), videoBuffer.scanLine(y >> 1, 1), videoBuffer.scanLine(y >> 1, 2), w);

    return destBuffer;
  }
  else if (videoBuffer.format().format() == SVideoFormat::Format_YUV422P)
  {
    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertYUV2toRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 1), videoBuffer.scanLine(y, 2), w);

    return destBuffer;
  }
  else if (videoBuffer.format().format() == SVideoFormat::Format_YUV444P)
  {
    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertYUV1toRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 1), videoBuffer.scanLine(y, 2), w);

    return destBuffer;
  }

  qWarning() << "SVideoFormatConvertNode::convertYUVtoRGB unimplemented format" << videoBuffer.format().formatName();
  return videoBuffer;
}

SVideoBuffer SVideoFormatConvertNode::convertBGRtoRGB(const SVideoBuffer &videoBuffer)
{
  if (videoBuffer.format().format() == SVideoFormat::Format_BGR32)
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertBGRtoRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), w);

    return destBuffer;
  }
  else if (videoBuffer.format().format() == SVideoFormat::Format_RGB32)
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_BGR32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertBGRtoRGB(destBuffer.scanLine(y, 0), videoBuffer.scanLine(y, 0), w);

    return destBuffer;
  }

  qWarning() << "SVideoFormatConvertNode::convertBGRtoRGB unimplemented format" << videoBuffer.format().formatName();
  return videoBuffer;
}

SVideoBuffer SVideoFormatConvertNode::demosaic(const SVideoBuffer &videoBuffer)
{
  if ((videoBuffer.format().format() >= SVideoFormat::Format_BGGR8) &&
      (videoBuffer.format().format() <= SVideoFormat::Format_RGGB8))
  {
    SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_RGB32,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    switch (videoBuffer.format().format())
    {
    case SVideoFormat::Format_GRBG8:
      LXiStream_SVideoFormatConvertNode_demosaic_GRBG8(
          videoBuffer.scanLine(0, 0),
          videoBuffer.format().size().width(),
          videoBuffer.lineSize(0),
          videoBuffer.format().size().height(),
          destBuffer.scanLine(0, 0),
          destBuffer.lineSize(0));

      break;

    case SVideoFormat::Format_GBRG8:
      LXiStream_SVideoFormatConvertNode_demosaic_GBRG8(
          videoBuffer.scanLine(0, 0),
          videoBuffer.format().size().width(),
          videoBuffer.lineSize(0),
          videoBuffer.format().size().height(),
          destBuffer.scanLine(0, 0),
          destBuffer.lineSize(0));

      break;

    case SVideoFormat::Format_RGGB8:
      LXiStream_SVideoFormatConvertNode_demosaic_RGGB8(
          videoBuffer.scanLine(0, 0),
          videoBuffer.format().size().width(),
          videoBuffer.lineSize(0),
          videoBuffer.format().size().height(),
          destBuffer.scanLine(0, 0),
          destBuffer.lineSize(0));

      break;

    case SVideoFormat::Format_BGGR8:
      LXiStream_SVideoFormatConvertNode_demosaic_BGGR8(
          videoBuffer.scanLine(0, 0),
          videoBuffer.format().size().width(),
          videoBuffer.lineSize(0),
          videoBuffer.format().size().height(),
          destBuffer.scanLine(0, 0),
          destBuffer.lineSize(0));

      break;

    default:
      return videoBuffer;
    }

    LXiStream_SVideoFormatConvertNode_demosaic_postfilter(
        destBuffer.scanLine(0, 0),
        destBuffer.format().size().width(),
        destBuffer.lineSize(0),
        destBuffer.format().size().height());

    return destBuffer;
  }

  qWarning() << "SVideoFormatConvertNode::demosaic unimplemented format" << videoBuffer.format().formatName();
  return videoBuffer;
}

SVideoBuffer SVideoFormatConvertNode::unpackYUVtoYUV420P(const SVideoBuffer &videoBuffer)
{
  SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUV420P,
                                       videoBuffer.format().size(),
                                       videoBuffer.format().frameRate(),
                                       videoBuffer.format().fieldMode()));

  if (videoBuffer.format().format() == SVideoFormat::Format_YUYV422)
  {
    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertYUYVtoYUV420P(videoBuffer.scanLine(y, 0), h, videoBuffer.lineSize(0), destBuffer.scanLine(y, 0), destBuffer.scanLine(y, 1), destBuffer.scanLine(y, 2));

    return destBuffer;
  }
  else if (videoBuffer.format().format() == SVideoFormat::Format_UYVY422)
  {
    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertUYVYtoYUV420P(videoBuffer.scanLine(y, 0), h, videoBuffer.lineSize(0), destBuffer.scanLine(y, 0), destBuffer.scanLine(y, 1), destBuffer.scanLine(y, 2));

    return destBuffer;
  }

  qWarning() << "SVideoFormatConvertNode::unpackYUVtoYUV420P unimplemented format" << videoBuffer.format().formatName();
  return videoBuffer;
}

SVideoBuffer SVideoFormatConvertNode::unpackYUVtoYUV422P(const SVideoBuffer &videoBuffer)
{
  SVideoBuffer destBuffer(SVideoFormat(SVideoFormat::Format_YUV422P,
                                       videoBuffer.format().size(),
                                       videoBuffer.format().frameRate(),
                                       videoBuffer.format().fieldMode()));

  if (videoBuffer.format().format() == SVideoFormat::Format_YUYV422)
  {
    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertYUYVtoYUV422P(videoBuffer.scanLine(y, 0), h, videoBuffer.lineSize(0), destBuffer.scanLine(y, 0), destBuffer.scanLine(y, 1), destBuffer.scanLine(y, 2));

    return destBuffer;
  }
  else if (videoBuffer.format().format() == SVideoFormat::Format_UYVY422)
  {
    const int w = videoBuffer.format().size().width(), h = videoBuffer.format().size().height();
    for (int y=0; y<h; y++)
      LXiStream_SVideoFormatConvertNode_convertUYVYtoYUV422P(videoBuffer.scanLine(y, 0), h, videoBuffer.lineSize(0), destBuffer.scanLine(y, 0), destBuffer.scanLine(y, 1), destBuffer.scanLine(y, 2));

    return destBuffer;
  }

  qWarning() << "SVideoFormatConvertNode::unpackYUVtoYUV420P unimplemented format" << videoBuffer.format().formatName();
  return videoBuffer;
}

void SVideoFormatConvertNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
  {
    if (d->format == videoBuffer.format())
    {
      emit output(videoBuffer);
    }
    else if (d->format == SVideoFormat::Format_RGB32)
    {
      if (videoBuffer.format().isYUV())
        emit output(convertYUVtoRGB(videoBuffer));
      else if (videoBuffer.format().isBayerArray())
        emit output(demosaic(videoBuffer));
      else if (videoBuffer.format() == SVideoFormat::Format_BGR32)
        emit output(convertBGRtoRGB(videoBuffer));
    }
    else if (d->format == SVideoFormat::Format_YUV420P)
    {
      if ((videoBuffer.format() == SVideoFormat::Format_YUYV422) ||
          (videoBuffer.format() == SVideoFormat::Format_UYVY422))
      {
        emit output(unpackYUVtoYUV420P(videoBuffer));
      }
    }
    else if (d->format == SVideoFormat::Format_YUV422P)
    {
      if ((videoBuffer.format() == SVideoFormat::Format_YUYV422) ||
          (videoBuffer.format() == SVideoFormat::Format_UYVY422))
      {
        emit output(unpackYUVtoYUV422P(videoBuffer));
      }
    }
  }
}


} // End of namespace
