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

#include "sdatacodec.h"

namespace LXiStream {

SDataCodec::SDataCodec(void)
{
  d.codec = QString::null;
}

SDataCodec::SDataCodec(const QString &codec)
{
  setCodec(codec);
}

bool SDataCodec::operator==(const SDataCodec &comp) const
{
  return (d.codec == comp.d.codec);
}

/*! Sets this codec to the specified format.
 */
void SDataCodec::setCodec(const QString &codec)
{
  d.codec = codec;
}

QDomNode SDataCodec::toXml(QDomDocument &doc) const
{
  QDomElement codecElm = createElement(doc, "datacodec");

  if (!d.codec.isEmpty())
    codecElm.setAttribute("codec", QString(d.codec));

  if (!d.extraData.isEmpty())
    codecElm.setAttribute("extradata", QString(d.extraData.toBase64()));

  return codecElm;
}

void SDataCodec::fromXml(const QDomNode &elm)
{
  QDomElement codecElm = findElement(elm, "datacodec");

  if (codecElm.hasAttribute("codec"))
    d.codec = codecElm.attribute("codec").toAscii();

  if (codecElm.hasAttribute("extradata"))
    d.extraData = QByteArray::fromBase64(codecElm.attribute("extradata").toAscii());
}

} // End of namespace
