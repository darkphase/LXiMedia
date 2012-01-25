/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "imageencoder.h"
#include "imagedecoder.h"

namespace LXiStream {
namespace GuiBackend {

ImageEncoder::ImageEncoder(const QString &, QObject *parent)
  : SInterfaces::VideoEncoder(parent),
    quality(80)
{
}

ImageEncoder::~ImageEncoder()
{
}

bool ImageEncoder::openCodec(const SVideoCodec &c, SInterfaces::BufferWriter *, Flags flags)
{
  outCodec = c;

  quality = 80;
  if (flags & Flag_LowQuality)
    quality = 50;
  else if (flags & Flag_HighQuality)
    quality = 95;

  return true;
}

SVideoCodec ImageEncoder::codec(void) const
{
  return outCodec;
}

SEncodedVideoBufferList ImageEncoder::encodeBuffer(const SVideoBuffer &input)
{
  if (!input.isNull())
  {
    const SImage image(input, quality < 80);
    if (!image.isNull())
    {
      outCodec = SVideoCodec(outCodec.codec(), image.size());

      QBuffer buffer;
      buffer.open(QBuffer::WriteOnly);
      if (image.save(&buffer, outCodec.codec().toAscii().data(), quality))
      {
        buffer.close();

        SEncodedVideoBuffer destBuffer(outCodec, buffer.data());

        destBuffer.setPresentationTimeStamp(input.timeStamp());
        destBuffer.setDecodingTimeStamp(input.timeStamp());
        destBuffer.setKeyFrame(true);
        return SEncodedVideoBufferList() << destBuffer;
      }
    }
  }

  return SEncodedVideoBufferList();
}

} } // End of namespaces
