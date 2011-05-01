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
    swsContext(NULL)
{
}

VideoResizer::~VideoResizer()
{
  if (swsContext)
  {
    sws_freeContext(swsContext);
    swsContext = NULL;
  }
}

int VideoResizer::algoFlags(const QString &name)
{
  if (name == "lanczos")
    return SWS_LANCZOS;
  else if (name == "bicubic")
    return SWS_BICUBIC;
  else // if (name == "bilinear")
    return SWS_FAST_BILINEAR;
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
        SSize(unsigned(size.width()) & ~0x0Fu, size.height()),
        lastFormat.frameRate(),
        lastFormat.fieldMode());

    if (swsContext)
    {
      sws_freeContext(swsContext);
      swsContext = NULL;
    }

    if (destFormat.size() != format.size())
    {
      const ::PixelFormat pf = FFMpegCommon::toFFMpegPixelFormat(lastFormat);

      swsContext = sws_getContext(lastFormat.size().width(), lastFormat.size().height(), pf,
                                  destFormat.size().width(), destFormat.size().height(), pf,
                                  filterFlags,
                                  NULL, NULL, NULL);
      if (swsContext == NULL)
      { // Fallback
        swsContext = sws_getContext(lastFormat.size().width(), lastFormat.size().height(), pf,
                                    destFormat.size().width(), destFormat.size().height(), pf,
                                    0,
                                    NULL, NULL, NULL);
      }
    }
  }

  return swsContext != NULL;
}

SVideoBuffer VideoResizer::processBuffer(const SVideoBuffer &videoBuffer)
{
  if (!scaleSize.isNull() && !videoBuffer.isNull() && VideoResizer::needsResize(videoBuffer.format()))
  {
    SVideoBuffer destBuffer(destFormat);

    uint8_t       * source[4]     = { (uint8_t *)videoBuffer.scanLine(0, 0),
                                      (uint8_t *)videoBuffer.scanLine(0, 1),
                                      (uint8_t *)videoBuffer.scanLine(0, 2),
                                      (uint8_t *)videoBuffer.scanLine(0, 3) };
    int             srcLineSize[4]= { videoBuffer.lineSize(0),
                                      videoBuffer.lineSize(1),
                                      videoBuffer.lineSize(2),
                                      videoBuffer.lineSize(3) };

    uint8_t       * dest[4]       = { (uint8_t *)destBuffer.scanLine(0, 0),
                                      (uint8_t *)destBuffer.scanLine(0, 1),
                                      (uint8_t *)destBuffer.scanLine(0, 2),
                                      (uint8_t *)destBuffer.scanLine(0, 3) };
    int             dstLineSize[4]= { destBuffer.lineSize(0),
                                      destBuffer.lineSize(1),
                                      destBuffer.lineSize(2),
                                      destBuffer.lineSize(3) };

    if (sws_scale(swsContext, source, srcLineSize,
                  0, lastFormat.size().height(),
                  dest, dstLineSize) >= 0)
    {
      destBuffer.setTimeStamp(videoBuffer.timeStamp());
      return destBuffer;
    }
  }

  return videoBuffer;
}

} } // End of namespaces
