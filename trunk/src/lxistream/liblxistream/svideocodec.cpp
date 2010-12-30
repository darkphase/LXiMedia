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

#include "svideocodec.h"

namespace LXiStream {

SVideoCodec::SVideoCodec(void)
{
  d.codec = QString::null;
  d.size = SSize();
  d.frameRate = SInterval();
  d.bitRate = 0;
}

SVideoCodec::SVideoCodec(const char *codec, SSize size, SInterval frameRate, int bitRate)
{
  setCodec(codec, size, frameRate, bitRate);
}

SVideoCodec::SVideoCodec(const QString &codec, SSize size, SInterval frameRate, int bitRate)
{
  setCodec(codec, size, frameRate, bitRate);
}

bool SVideoCodec::operator==(const SVideoCodec &comp) const
{
  return (d.codec == comp.d.codec) && (d.size == comp.d.size) &&
      qFuzzyCompare(d.frameRate.toFrequency(), comp.d.frameRate.toFrequency()) &&
      (d.bitRate == comp.d.bitRate);
}

/*! Sets this codec to the specified video codec.
 */
void SVideoCodec::setCodec(const QString &codec, SSize size, SInterval frameRate, int bitRate)
{
  d.codec = codec;
  d.size = size;
  d.frameRate = frameRate;
  d.bitRate = bitRate;
}

QDomNode SVideoCodec::toXml(QDomDocument &doc) const
{
  QDomElement codecElm = createElement(doc, "videocodec");

  if (!d.codec.isEmpty())
    codecElm.setAttribute("codec", QString(d.codec));

  codecElm.setAttribute("width", d.size.width());
  codecElm.setAttribute("height", d.size.height());
  codecElm.setAttribute("aspectratio", d.size.aspectRatio());
  codecElm.setAttribute("framerate", d.frameRate.toFrequency());
  codecElm.setAttribute("bitrate", d.bitRate);

  if (!d.extraData.isEmpty())
    codecElm.setAttribute("extradata", QString(d.extraData.toBase64()));

  return codecElm;
}

void SVideoCodec::fromXml(const QDomNode &elm)
{
  QDomElement codecElm = findElement(elm, "videocodec");

  if (codecElm.hasAttribute("codec"))
    d.codec = codecElm.attribute("codec").toAscii();

  d.size.setWidth(codecElm.attribute("width").toInt());
  d.size.setHeight(codecElm.attribute("height").toInt());
  d.size.setAspectRatio(codecElm.attribute("aspectratio").toFloat());
  d.frameRate = SInterval::fromFrequency(codecElm.attribute("framerate").toDouble());
  d.bitRate = codecElm.attribute("bitrate").toInt();

  if (codecElm.hasAttribute("extradata"))
    d.extraData = QByteArray::fromBase64(codecElm.attribute("extradata").toAscii());
}

} // End of namespace
