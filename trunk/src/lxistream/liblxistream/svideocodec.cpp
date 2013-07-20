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

#include "svideocodec.h"

namespace LXiStream {

SVideoCodec::SVideoCodec(void)
{
  d.name = QByteArray();
  d.size = SSize();
  d.frameRate = SInterval();
  d.streamId = -1;
  d.bitRate = 0;
  d.gopSize = -1;
}

SVideoCodec::SVideoCodec(const char *name, SSize size, SInterval frameRate, int streamId, int bitRate, int gopSize)
{
  setCodec(name, size, frameRate, streamId, bitRate, gopSize);
}

SVideoCodec::SVideoCodec(const QByteArray &name, SSize size, SInterval frameRate, int streamId, int bitRate, int gopSize)
{
  setCodec(name, size, frameRate, streamId, bitRate, gopSize);
}

bool SVideoCodec::operator==(const SVideoCodec &comp) const
{
  return (d.name == comp.d.name) && (d.size == comp.d.size) &&
      qFuzzyCompare(d.frameRate.toFrequency(), comp.d.frameRate.toFrequency()) &&
      (d.streamId == comp.d.streamId) && (d.bitRate == comp.d.bitRate) &&
      (d.gopSize == comp.d.gopSize);
}

/*! Sets this codec to the specified video codec.
 */
void SVideoCodec::setCodec(const QByteArray &name, SSize size, SInterval frameRate, int streamId, int bitRate, int gopSize)
{
  d.name = name;
  d.size = size;
  d.frameRate = frameRate;
  d.streamId = streamId;
  d.bitRate = bitRate;
  d.gopSize = gopSize;
}

void SVideoCodec::serialize(QXmlStreamWriter &writer) const
{
  writer.writeStartElement("videocodec");

  writer.writeAttribute("name", d.name);
  writer.writeAttribute("size", d.size.toString());
  writer.writeAttribute("framerate", QString::number(d.frameRate.num()) + '/' + QString::number(d.frameRate.den()));
  writer.writeAttribute("streamid", QString::number(d.streamId));
  writer.writeAttribute("bitrate", QString::number(d.bitRate));
  writer.writeAttribute("gopsize", QString::number(d.gopSize));

  writer.writeEndElement();
}

bool SVideoCodec::deserialize(QXmlStreamReader &reader)
{
  if (reader.name() == "videocodec")
  {
    d.name = reader.attributes().value("name").toString().toLatin1();
    d.size = SSize::fromString(reader.attributes().value("size").toString());

    const QStringList rate = reader.attributes().value("framerate").toString().split('/');
    if (rate.count() >= 2)
      d.frameRate = SInterval(rate[0].toLongLong(), rate[1].toLongLong());

    d.streamId = reader.attributes().value("streamid").toString().toInt();
    d.bitRate = reader.attributes().value("bitrate").toString().toInt();
    d.gopSize = reader.attributes().value("gopsize").toString().toInt();

    return true;
  }

  reader.raiseError("Not a videocodec element.");
  return false;
}

} // End of namespace
