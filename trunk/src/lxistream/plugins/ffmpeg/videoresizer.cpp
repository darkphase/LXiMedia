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
    filterOverlap(filterFlags == SWS_LANCZOS ? 8 : 4),
    scaleSize(),
    scaleAspectRatioMode(Qt::KeepAspectRatio),
    lastFormat(SVideoFormat::Format_Invalid),
    destFormat(SVideoFormat::Format_Invalid),
    numThreads(0),
    preFilter((scheme == "deinterlace") ? createDeinterlaceFilter() : NULL)
{
  for (int i=0; i<2; i++)
    swsContext[i] = NULL;
}

VideoResizer::~VideoResizer()
{
  for (int i=0; i<2; i++)
  if (swsContext[i])
    ::sws_freeContext(swsContext[i]);

  delete preFilter;
}

int VideoResizer::algoFlags(const QString &name)
{
  if (name == "lanczos")
    return SWS_LANCZOS;
  else if (name == "bicubic")
    return SWS_BICUBIC;
  else // if ((name == "bilinear") || (name == "deinterlace"))
    return SWS_BILINEAR;
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

    size.setHeight((size.height() + 1) & ~1);

    destFormat = SVideoFormat(
        lastFormat.format(),
        size,
        lastFormat.frameRate(),
        lastFormat.fieldMode());

    for (int i=0; i<2; i++)
    if (swsContext[i])
    {
      ::sws_freeContext(swsContext[i]);
      swsContext[i] = NULL;
    }

    if (destFormat.size() != lastFormat.size())
    {
      if ((destFormat.size().height() >= 256) && (lastFormat.size().height() >= 256))
        numThreads = qBound(1, QThread::idealThreadCount(), 2);
      else
        numThreads = 1;

      for (int i=0; i<numThreads; i++)
      {
        const ::PixelFormat pf = FFMpegCommon::toFFMpegPixelFormat(lastFormat);

        swsContext[i] = ::sws_getCachedContext(
            NULL,
            lastFormat.size().width(), lastFormat.size().height(), pf,
            destFormat.size().width(), destFormat.size().height(), pf,
            filterFlags,
            preFilter, NULL, NULL);

        if (swsContext[i] == NULL)
        { // Fallback
          swsContext[i] = ::sws_getCachedContext(
              NULL,
              lastFormat.size().width(), lastFormat.size().height(), pf,
              destFormat.size().width(), destFormat.size().height(), pf,
              0,
              preFilter, NULL, NULL);
        }
      }

      if (swsContext[1] == NULL)
        numThreads = 1;
    }
  }

  return swsContext[0] != NULL;
}

SVideoBuffer VideoResizer::processBuffer(const SVideoBuffer &videoBuffer)
{
  if (!scaleSize.isNull() && !videoBuffer.isNull() && VideoResizer::needsResize(videoBuffer.format()))
  {
    SVideoBuffer destBuffer(destFormat);

    int wf = 1, hf = 1;
    destFormat.planarYUVRatio(wf, hf);

    bool result = true;
    QVector< QFuture<bool> > futures;
    futures.reserve(numThreads);

    const int imageHeight = lastFormat.size().height();
    const int stripSrcHeight = qMin(imageHeight, (imageHeight / numThreads) + filterOverlap);
    for (int i=0; i<numThreads; i++)
    {
      const int stripSrcPos = (i == 0) ? 0 : (imageHeight - stripSrcHeight);

      Slice src;
      src.stride[0] = videoBuffer.lineSize(0);
      src.stride[1] = videoBuffer.lineSize(1);
      src.stride[2] = videoBuffer.lineSize(2);
      src.stride[3] = videoBuffer.lineSize(3);
      src.line[0] = (quint8 *)videoBuffer.scanLine(0, 0) + (stripSrcPos * src.stride[0]);
      src.line[1] = (quint8 *)videoBuffer.scanLine(0, 1) + ((stripSrcPos / hf) * src.stride[1]);
      src.line[2] = (quint8 *)videoBuffer.scanLine(0, 2) + ((stripSrcPos / hf) * src.stride[2]);
      src.line[3] = (quint8 *)videoBuffer.scanLine(0, 3) + ((stripSrcPos / hf) * src.stride[3]);

      Slice dst;
      dst.stride[0] = destBuffer.lineSize(0);
      dst.stride[1] = destBuffer.lineSize(1);
      dst.stride[2] = destBuffer.lineSize(2);
      dst.stride[3] = destBuffer.lineSize(3);
      dst.line[0] = (quint8 *)destBuffer.scanLine(0, 0);
      dst.line[1] = (quint8 *)destBuffer.scanLine(0, 1);
      dst.line[2] = (quint8 *)destBuffer.scanLine(0, 2);
      dst.line[3] = (quint8 *)destBuffer.scanLine(0, 3);

      if (numThreads > 1)
        futures += QtConcurrent::run(this, &VideoResizer::processSlice, i, stripSrcPos, stripSrcHeight, src, dst);
      else
        result = processSlice(i, stripSrcPos, stripSrcHeight, src, dst);
    }

    if (numThreads > 1)
    for (int i=0; i<numThreads; i++)
      result &= futures[i].result();

    if (result)
    {
      destBuffer.setTimeStamp(videoBuffer.timeStamp());
      return destBuffer;
    }
  }

  return videoBuffer;
}

bool VideoResizer::processSlice(int strip, int stripPos, int stripHeight, const Slice &src, const Slice &dst)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  int wf = 1, hf = 1;
  const bool planar = destFormat.planarYUVRatio(wf, hf);

  const bool result = ::sws_scale(
      swsContext[strip], src.line, src.stride,
      stripPos, stripHeight,
      dst.line, dst.stride) >= 0;

  // Correct chroma lines on the top
  if ((strip == 0) && planar && result && destFormat.isYUV() && dst.line[1] && dst.line[2])
  {
    const int skip = 4 / hf;

    quint8 * const ref[2] = {
      dst.line[1] + (dst.stride[1] * skip),
      dst.line[2] + (dst.stride[2] * skip) };

    for (int y=0; y<skip-1; y++)
    {
      memcpy(dst.line[1] + (dst.stride[1] * y), ref[0], dst.stride[1]);
      memcpy(dst.line[2] + (dst.stride[2] * y), ref[1], dst.stride[2]);
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
