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

#include "videoresizer.h"
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {

VideoResizer::VideoResizer(const QString &scheme, QObject *parent)
  : SInterfaces::VideoResizer(parent),
    filterFlags(algoFlags(scheme)),
    scaleSize(),
    scaleAspectRatioMode(Qt::KeepAspectRatio),
    lastFormat(SVideoFormat::Format_Invalid),
    destFormat(SVideoFormat::Format_Invalid),
    swsContext(NULL),
    preFilter((scheme == "deinterlace") ? createDeinterlaceFilter() : NULL)
{
}

VideoResizer::~VideoResizer()
{
  if (swsContext)
    ::sws_freeContext(swsContext);

  delete preFilter;
}

int VideoResizer::algoFlags(const QString &name)
{
  if (name == "lanczos")
    return SWS_LANCZOS;
  else if (name == "bicubic")
    return SWS_BICUBIC;
  else // if ((name == "bilinear") || (name == "deinterlace"))
  {
#ifndef Q_OS_MACX
    return SWS_FAST_BILINEAR; // Crashes on Mac
#else
    return SWS_BILINEAR;
#endif
  }
}

void VideoResizer::setSize(const SSize &size)
{
  scaleSize = size;
}

SSize VideoResizer::size(void) const
{
  return scaleSize;
}

void VideoResizer::setAspectRatioMode(Qt::AspectRatioMode a)
{
  scaleAspectRatioMode = a;
}

Qt::AspectRatioMode VideoResizer::aspectRatioMode(void) const
{
  return scaleAspectRatioMode;
}

bool VideoResizer::needsResize(const SVideoFormat &format)
{
  if (lastFormat != format)
  {
    lastFormat = format;

    QSize size = lastFormat.size().absoluteSize();
    size.scale(scaleSize.absoluteSize(), scaleAspectRatioMode);
    if (scaleSize.aspectRatio() > 1.0)
      size.setWidth(int(size.width() / scaleSize.aspectRatio()));
    else if (scaleSize.aspectRatio() < 1.0)
      size.setHeight(int(size.height() / (1.0f / scaleSize.aspectRatio())));

    destFormat = SVideoFormat(
        lastFormat.format(),
        size,
        lastFormat.frameRate(),
        lastFormat.fieldMode());

    if (swsContext)
    {
      ::sws_freeContext(swsContext);
      swsContext = NULL;
    }

    if (destFormat.size() != format.size())
    {
      const ::PixelFormat pf = FFMpegCommon::toFFMpegPixelFormat(lastFormat);

      swsContext = ::sws_getContext(lastFormat.size().width(), lastFormat.size().height(), pf,
                                    destFormat.size().width(), destFormat.size().height(), pf,
                                    filterFlags,
                                    preFilter, NULL, NULL);
      if (swsContext == NULL)
      { // Fallback
        swsContext = ::sws_getContext(lastFormat.size().width(), lastFormat.size().height(), pf,
                                      destFormat.size().width(), destFormat.size().height(), pf,
                                      0,
                                      preFilter, NULL, NULL);
      }
    }
  }

  return swsContext != NULL;
}

SVideoBuffer VideoResizer::processBuffer(const SVideoBuffer &videoBuffer)
{
  if (!scaleSize.isNull() && !videoBuffer.isNull() && VideoResizer::needsResize(videoBuffer.format()))
  {
    quint8      * source[4]       = { (quint8 *)videoBuffer.scanLine(0, 0),
                                      (quint8 *)videoBuffer.scanLine(0, 1),
                                      (quint8 *)videoBuffer.scanLine(0, 2),
                                      (quint8 *)videoBuffer.scanLine(0, 3) };
    int           srcLineSize[4]  = { videoBuffer.lineSize(0),
                                      videoBuffer.lineSize(1),
                                      videoBuffer.lineSize(2),
                                      videoBuffer.lineSize(3) };

    SVideoBuffer destBuffer(destFormat);

    quint8      * dest[4]         = { (quint8 *)destBuffer.scanLine(0, 0),
                                      (quint8 *)destBuffer.scanLine(0, 1),
                                      (quint8 *)destBuffer.scanLine(0, 2),
                                      (quint8 *)destBuffer.scanLine(0, 3) };
    int           dstLineSize[4]  = { destBuffer.lineSize(0),
                                      destBuffer.lineSize(1),
                                      destBuffer.lineSize(2),
                                      destBuffer.lineSize(3) };

    if (processSlice(source, srcLineSize, dest, dstLineSize))
    {
      destBuffer.setTimeStamp(videoBuffer.timeStamp());
      return destBuffer;
    }
  }

  return videoBuffer;
}

bool VideoResizer::processSlice(quint8 **source, int *srcLineSize, quint8 **dest, int *dstLineSize)
{
  int wf = 0, hf = 0;
  const bool planar = destFormat.planarYUVRatio(wf, hf);

  const bool result =
      ::sws_scale(swsContext, source, srcLineSize,
                  0, lastFormat.size().height(),
                  dest, dstLineSize) >= 0;

  // Correct chroma lines on the top
  if (planar && result && destFormat.isYUV() && dest[1] && dest[2])
  {
    const int skip = 4 / hf;

    quint8 * const ref[2] = {
      dest[1] + (dstLineSize[1] * skip),
      dest[2] + (dstLineSize[2] * skip) };

    for (int y=0; y<skip-1; y++)
    {
      memcpy(dest[1] + (dstLineSize[1] * y), ref[0], dstLineSize[1]);
      memcpy(dest[2] + (dstLineSize[2] * y), ref[1], dstLineSize[2]);
    }
  }

  return result;
}

::SwsFilter * VideoResizer::createDeinterlaceFilter(void)
{
  struct Vector1 : ::SwsVector
  {
    inline Vector1(void) { c[0] = 1.0; coeff = c; length = 1; }

    double c[1];
  };

  struct Vector3 : ::SwsVector
  {
    inline Vector3(void) { c[0] = 0.25; c[1] = 0.5; c[2] = 0.25; coeff = c; length = 3; }

    double c[3];
  };

  struct Filter : ::SwsFilter
  {
    inline Filter(void) { lumH = &lh; lumV = &lv; chrH = &ch; chrV = &cv; }

    Vector1 lh, ch;
    Vector3 lv, cv;
  };

  return new Filter();
}

} } // End of namespaces
