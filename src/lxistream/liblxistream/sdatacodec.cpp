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

#include "sdatacodec.h"

namespace LXiStream {

SDataCodec::SDataCodec(void)
{
  d.codec = QString::null;
}

SDataCodec::SDataCodec(const QString &codec, const QByteArray &codepage, int streamId)
{
  setCodec(codec, codepage, streamId);
}

bool SDataCodec::operator==(const SDataCodec &comp) const
{
  return (d.codec == comp.d.codec) &&
      (d.streamId == comp.d.streamId);
}

/*! Sets this codec to the specified format.
 */
void SDataCodec::setCodec(const QString &codec, const QByteArray &codepage, int streamId)
{
  d.codec = codec;
  d.codepage = codepage;
  d.streamId = streamId;
}

QString SDataCodec::toString(void) const
{
  QString result =
      d.codec + ';' +
      QString::number(d.streamId);

  return result;
}

SDataCodec SDataCodec::fromString(const QString &str)
{
  const QStringList items = str.split(';');
  SDataCodec result;

  if (items.count() >= 2)
  {
    result.d.codec = items[0].toAscii();
    result.d.streamId = items[1].toInt();
  }

  return result;
}

} // End of namespace
