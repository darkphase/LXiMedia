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
  d.streamId = -1;
  d.bitRate = 0;
  d.gopSize = -1;
}

SVideoCodec::SVideoCodec(const char *codec, SSize size, SInterval frameRate, int streamId, int bitRate, int gopSize)
{
  setCodec(codec, size, frameRate, streamId, bitRate, gopSize);
}

SVideoCodec::SVideoCodec(const QString &codec, SSize size, SInterval frameRate, int streamId, int bitRate, int gopSize)
{
  setCodec(codec, size, frameRate, streamId, bitRate, gopSize);
}

bool SVideoCodec::operator==(const SVideoCodec &comp) const
{
  return (d.codec == comp.d.codec) && (d.size == comp.d.size) &&
      qFuzzyCompare(d.frameRate.toFrequency(), comp.d.frameRate.toFrequency()) &&
      (d.streamId == comp.d.streamId) && (d.bitRate == comp.d.bitRate) &&
      (d.gopSize == comp.d.gopSize);
}

/*! Sets this codec to the specified video codec.
 */
void SVideoCodec::setCodec(const QString &codec, SSize size, SInterval frameRate, int streamId, int bitRate, int gopSize)
{
  d.codec = codec;
  d.size = size;
  d.frameRate = frameRate;
  d.streamId = streamId;
  d.bitRate = bitRate;
  d.gopSize = gopSize;
}

QString SVideoCodec::toString(void) const
{
  QString result = d.codec + ';' +
    d.size.toString() + ';' +
    QString::number(d.frameRate.num()) + '/' +
    QString::number(d.frameRate.den()) + ';' +
    QString::number(d.streamId) + ';' +
    QString::number(d.bitRate) + ';' +
    QString::number(d.gopSize);

  return result;
}

SVideoCodec SVideoCodec::fromString(const QString &str)
{
  const QStringList items = str.split(';');
  SVideoCodec result;

  if (items.count() >= 6)
  {
    result.d.codec = items[0].toAscii();
    result.d.size = SSize::fromString(items[1]);

    const QStringList rate = items[2].split('/');
    if (rate.count() >= 2)
      result.d.frameRate = SInterval(rate[0].toLongLong(), rate[1].toLongLong());

    result.d.streamId = items[3].toInt();
    result.d.bitRate = items[4].toInt();
    result.d.gopSize = items[5].toInt();
  }

  return result;
}

} // End of namespace
