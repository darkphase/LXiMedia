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

QString SVideoCodec::toString(bool addExtraData) const
{
  QString result = d.codec + ';' +
    d.size.toString() + ';' +
    QString::number(d.frameRate.toFrequency()) + ';' +
    QString::number(d.bitRate);

  if (addExtraData && !d.extraData.isEmpty())
    result += ';' + QString::fromAscii(d.extraData.toBase64());

  return result;
}

SVideoCodec SVideoCodec::fromString(const QString &str)
{
  const QStringList items = str.split(';');
  SVideoCodec result;

  if (items.count() >= 4)
  {
    result.d.codec = items[0].toAscii();
    result.d.size = SSize::fromString(items[1]);
    result.d.frameRate = SInterval::fromFrequency(items[2].toFloat());
    result.d.bitRate = items[3].toInt();
  }

  if (items.count() >= 5)
    result.d.extraData = QByteArray::fromBase64(items[4].toAscii());

  return result;
}

} // End of namespace
