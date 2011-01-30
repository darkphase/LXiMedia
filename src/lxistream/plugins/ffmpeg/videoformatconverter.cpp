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

namespace LXiStream {
namespace FFMpegBackend {

VideoFormatConverter::VideoFormatConverter(const QString &, QObject *parent)
  : SInterfaces::VideoFormatConverter(parent),
    srcFormat(SVideoFormat::Format_Invalid),
    dstFormat(SVideoFormat::Format_Invalid),
    width(720),
    height(576)
{
}

VideoFormatConverter::~VideoFormatConverter()
{
  if (swsContextHandle)
    ::sws_freeContext(swsContextHandle);
}

bool VideoFormatConverter::testformat(::PixelFormat srcFormat, ::PixelFormat dstFormat)
{
  ::SwsContext * context =
      ::sws_getContext(720, 576,
                       srcFormat,
                       720, 576,
                       dstFormat,
                       SWS_POINT,
                       NULL, NULL, NULL);

  if (context)
    ::sws_freeContext(context);

  return context != NULL;
}

bool VideoFormatConverter::openFormat(const SVideoFormat &srcFormat, const SVideoFormat &dstFormat)
{
  this->srcFormat = srcFormat.format();
  this->dstFormat = dstFormat.format();

  return openFormat();
}

bool VideoFormatConverter::openFormat(void)
{
  swsContextHandle =
      ::sws_getCachedContext(swsContextHandle,
                             width, height,
                             ::PixelFormat(FFMpegCommon::toFFMpegPixelFormat(srcFormat)),
                             width, height,
                             ::PixelFormat(FFMpegCommon::toFFMpegPixelFormat(dstFormat)),
                             SWS_POINT,
                             NULL, NULL, NULL);

  return swsContextHandle != NULL;
}

SVideoBuffer VideoFormatConverter::convertBuffer(const SVideoBuffer &videoBuffer)
{
  if ((width != videoBuffer.format().size().width()) ||
      (height != videoBuffer.format().size().height()))
  {
    width = videoBuffer.format().size().width();
    height = videoBuffer.format().size().height();
    openFormat();
  }

  if (swsContextHandle && (videoBuffer.format() == srcFormat))
  {

    uint8_t * src[4]     = { (uint8_t *)videoBuffer.scanLine(0, 0),
                             (uint8_t *)videoBuffer.scanLine(0, 1),
                             (uint8_t *)videoBuffer.scanLine(0, 2),
                             (uint8_t *)videoBuffer.scanLine(0, 3) };
    int       srcInc[4]  = { videoBuffer.lineSize(0),
                             videoBuffer.lineSize(1),
                             videoBuffer.lineSize(2),
                             videoBuffer.lineSize(3)};

    SVideoBuffer destBuffer(SVideoFormat(dstFormat,
                                         videoBuffer.format().size(),
                                         videoBuffer.format().frameRate(),
                                         videoBuffer.format().fieldMode()));

    uint8_t * dst[4]     = { (uint8_t *)destBuffer.scanLine(0, 0),
                             (uint8_t *)destBuffer.scanLine(0, 1),
                             (uint8_t *)destBuffer.scanLine(0, 2),
                             (uint8_t *)destBuffer.scanLine(0, 3) };
    int       dstInc[4]  = { destBuffer.lineSize(0),
                             destBuffer.lineSize(1),
                             destBuffer.lineSize(2),
                             destBuffer.lineSize(3)};

    ::sws_scale(swsContextHandle,
                src, srcInc,
                0, height,
                dst, dstInc);

    destBuffer.setTimeStamp(videoBuffer.timeStamp());

    return destBuffer;
  }

  return SVideoBuffer();
}

} } // End of namespaces
