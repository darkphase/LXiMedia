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

#include "nodes/svideoboxnode.h"
#include "svideobuffer.h"

// Implemented in svideobox.box.c
extern "C" void LXiStream_SVideoBoxNode_boxVideo8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstWidth, unsigned dstStride, unsigned dstNumLines,
    int nullPixel);

extern "C" void LXiStream_SVideoBoxNode_boxVideo32(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstWidth, unsigned dstStride, unsigned dstNumLines,
    quint32 nullPixel);

namespace LXiStream {

struct SVideoBoxNode::Data
{
  SSize                         destSize;
  QFuture<void>                 future;
};

SVideoBoxNode::SVideoBoxNode(SGraph *parent)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
  d->destSize = SSize(768, 576);
}

SVideoBoxNode::~SVideoBoxNode()
{
  d->future.waitForFinished();
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

SSize SVideoBoxNode::size(void) const
{
  return d->destSize;
}

void SVideoBoxNode::setSize(const SSize &s)
{
  d->destSize = s;
}

bool SVideoBoxNode::start(void)
{
  return true;
}

void SVideoBoxNode::stop(void)
{
  d->future.waitForFinished();
}

void SVideoBoxNode::input(const SVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION;

  d->future.waitForFinished();

  if (!videoBuffer.isNull())
  {
    if (videoBuffer.format().size() == d->destSize)
      emit output(videoBuffer);
    else
      d->future = QtConcurrent::run(this, &SVideoBoxNode::processTask, videoBuffer);
  }
  else
    emit output(videoBuffer);
}

void SVideoBoxNode::processTask(const SVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION;

  const SSize size = videoBuffer.format().size();

  SVideoBuffer destBuffer(SVideoFormat(videoBuffer.format().format(),
                                       d->destSize,
                                       videoBuffer.format().frameRate(),
                                       videoBuffer.format().fieldMode()));

  if (destBuffer.format().sampleSize() == 1)
  {
    int wr = 1, hr = 1;
    videoBuffer.format().planarYUVRatio(wr, hr);

    if (destBuffer.lineSize(0) > 0)
      LXiStream_SVideoBoxNode_boxVideo8(videoBuffer.scanLine(0, 0),
                                  size.width(), videoBuffer.lineSize(0),
                                  size.height(), destBuffer.scanLine(0, 0),
                                  d->destSize.width(), destBuffer.lineSize(0),
                                  d->destSize.height(), 0);

    if (destBuffer.lineSize(1) > 0)
      LXiStream_SVideoBoxNode_boxVideo8(videoBuffer.scanLine(0, 1),
                                  size.width() / wr, videoBuffer.lineSize(1),
                                  size.height() / hr, destBuffer.scanLine(0, 1),
                                  d->destSize.width() / wr, destBuffer.lineSize(1),
                                  d->destSize.height() / hr, 127);

    if (destBuffer.lineSize(2) > 0)
      LXiStream_SVideoBoxNode_boxVideo8(videoBuffer.scanLine(0, 2),
                                  size.width() / wr, videoBuffer.lineSize(2),
                                  size.height() / hr, destBuffer.scanLine(0, 2),
                                  d->destSize.width() / wr, destBuffer.lineSize(2),
                                  d->destSize.height() / hr, 127);
  }
  if (destBuffer.format().sampleSize() == 2)
  {
    if (destBuffer.lineSize(0) > 0)
      LXiStream_SVideoBoxNode_boxVideo32(videoBuffer.scanLine(0, 0),
                                   size.width() / 2, videoBuffer.lineSize(0),
                                   size.height(), destBuffer.scanLine(0, 0),
                                   d->destSize.width() / 2, destBuffer.lineSize(0),
                                   d->destSize.height(), videoBuffer.format().nullPixelValue());
  }
  if (destBuffer.format().sampleSize() == 4)
  {
    if (destBuffer.lineSize(0) > 0)
      LXiStream_SVideoBoxNode_boxVideo32(videoBuffer.scanLine(0, 0),
                                   size.width(), videoBuffer.lineSize(0),
                                   size.height(), destBuffer.scanLine(0, 0),
                                   d->destSize.width(), destBuffer.lineSize(0),
                                   d->destSize.height(), videoBuffer.format().nullPixelValue());
  }

  destBuffer.setTimeStamp(videoBuffer.timeStamp());
  emit output(destBuffer);
}


} // End of namespace
