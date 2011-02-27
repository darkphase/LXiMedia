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

bool ImageEncoder::openCodec(const SVideoCodec &c, Flags flags)
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

      QBuffer b;
      image.save(&b, outCodec.codec().toAscii().data(), quality);
      SEncodedVideoBuffer destBuffer(outCodec, b.data());

      destBuffer.setPresentationTimeStamp(input.timeStamp());
      destBuffer.setDecodingTimeStamp(input.timeStamp());
      destBuffer.setKeyFrame(true);
      return SEncodedVideoBufferList() << destBuffer;
    }
  }

  return SEncodedVideoBufferList();
}


} } // End of namespaces