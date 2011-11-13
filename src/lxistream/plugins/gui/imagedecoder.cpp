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

#include "imagedecoder.h"

namespace LXiStream {
namespace GuiBackend {

ImageDecoder::ImageDecoder(const QString &, QObject *parent)
  : SInterfaces::VideoDecoder(parent)
{
}

ImageDecoder::~ImageDecoder()
{
}

bool ImageDecoder::openCodec(const SVideoCodec &, SInterfaces::AbstractBufferReader *, Flags)
{
  return true;
}

SVideoBufferList ImageDecoder::decodeBuffer(const SEncodedVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
  {
    SVideoBuffer imageBuffer = SImage::fromData(videoBuffer);
    if (!imageBuffer.isNull())
    {
      if (videoBuffer.presentationTimeStamp().isValid())
        imageBuffer.setTimeStamp(videoBuffer.presentationTimeStamp());
      else if (videoBuffer.decodingTimeStamp().isValid())
        imageBuffer.setTimeStamp(videoBuffer.decodingTimeStamp());

      return SVideoBufferList() << imageBuffer;
    }
  }

  return SVideoBufferList();
}

} } // End of namespaces
