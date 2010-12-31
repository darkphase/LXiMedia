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

#include "nodes/svideodemosaicnode.h"
#include "sgraph.h"
#include "svideobuffer.h"

// Implemented in svideodemosaicnode.demosaic.c
extern "C" void LXiStream_SVideoDemosaicNode_demosaic_GRBG8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_SVideoDemosaicNode_demosaic_GBRG8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_SVideoDemosaicNode_demosaic_RGGB8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_SVideoDemosaicNode_demosaic_BGGR8(
    const void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines,
    void * dstData, unsigned dstStride) __attribute__((nonnull(1, 5)));
extern "C" void LXiStream_SVideoDemosaicNode_demosaic_postfilter(
    void * srcData, unsigned srcWidth, unsigned srcStride, unsigned srcNumLines) __attribute__((nonnull(1)));

namespace LXiStream {

struct SVideoDemosaicNode::Data
{
};

SVideoDemosaicNode::SVideoDemosaicNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
}

SVideoDemosaicNode::~SVideoDemosaicNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

SVideoBuffer SVideoDemosaicNode::demosaic(const SVideoBuffer &videoBuffer)
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
      LXiStream_SVideoDemosaicNode_demosaic_GRBG8(
          videoBuffer.scanLine(0, 0),
          videoBuffer.format().size().width(),
          videoBuffer.lineSize(0),
          videoBuffer.format().size().height(),
          destBuffer.scanLine(0, 0),
          destBuffer.lineSize(0));

      break;

    case SVideoFormat::Format_GBRG8:
      LXiStream_SVideoDemosaicNode_demosaic_GBRG8(
          videoBuffer.scanLine(0, 0),
          videoBuffer.format().size().width(),
          videoBuffer.lineSize(0),
          videoBuffer.format().size().height(),
          destBuffer.scanLine(0, 0),
          destBuffer.lineSize(0));

      break;

    case SVideoFormat::Format_RGGB8:
      LXiStream_SVideoDemosaicNode_demosaic_RGGB8(
          videoBuffer.scanLine(0, 0),
          videoBuffer.format().size().width(),
          videoBuffer.lineSize(0),
          videoBuffer.format().size().height(),
          destBuffer.scanLine(0, 0),
          destBuffer.lineSize(0));

      break;

    case SVideoFormat::Format_BGGR8:
      LXiStream_SVideoDemosaicNode_demosaic_BGGR8(
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

    LXiStream_SVideoDemosaicNode_demosaic_postfilter(
        destBuffer.scanLine(0, 0),
        destBuffer.format().size().width(),
        destBuffer.lineSize(0),
        destBuffer.format().size().height());

    return destBuffer;
  }

  return videoBuffer;
}

void SVideoDemosaicNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
    emit output(demosaic(videoBuffer));
}


} // End of namespace
