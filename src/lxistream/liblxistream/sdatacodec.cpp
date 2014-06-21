/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include "sdatacodec.h"

namespace LXiStream {

SDataCodec::SDataCodec(void)
{
  d.name = QByteArray();
}

SDataCodec::SDataCodec(const char *name, const QByteArray &codepage, int streamId)
{
  setCodec(name, codepage, streamId);
}

SDataCodec::SDataCodec(const QByteArray &name, const QByteArray &codepage, int streamId)
{
  setCodec(name, codepage, streamId);
}

bool SDataCodec::operator==(const SDataCodec &comp) const
{
  return (d.name == comp.d.name) &&
      (d.streamId == comp.d.streamId);
}

/*! Sets this codec to the specified format.
 */
void SDataCodec::setCodec(const QByteArray &name, const QByteArray &codepage, int streamId)
{
  d.name = name;
  d.codepage = codepage;
  d.streamId = streamId;
}

void SDataCodec::serialize(QXmlStreamWriter &writer) const
{
  writer.writeStartElement("datacodec");

  writer.writeAttribute("name", d.name);
  writer.writeAttribute("streamid", QString::number(d.streamId));

  writer.writeEndElement();
}

bool SDataCodec::deserialize(QXmlStreamReader &reader)
{
  if (reader.name() == "datacodec")
  {
    d.name = reader.attributes().value("name").toString().toLatin1();
    d.streamId = reader.attributes().value("streamid").toString().toInt();

    return true;
  }

  reader.raiseError("Not a datacodec element.");
  return false;
}

} // End of namespace
