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

#include "saudiocodec.h"

namespace LXiStream {

/*! \class SAudioCodec
    \brief The SAudioCodec class defines the properties of an audio codec.

    An SAudioCodec can be used to store all properties of an audio codec. Such
    as the name of the codec (e.g. "VORBIS"), the number of channels, the sample
    rate and the bit rate. Additionally a piece of opaque codec data can be
    provided called extra data.

    Additionally, an SAudioCodec can be serialized to XML.

    \sa SEncodedAudioBuffer
 */

/*! Initializes an empty codec.
 */
SAudioCodec::SAudioCodec(void)
{
  d.codec = QString::null;
  d.channels = SAudioFormat::Channel_None;
  d.sampleRate = 0;
  d.bitRate = 0;
}

/*! Initializes a codec with the specified name, channels, sample rate and bit rate.
 */
SAudioCodec::SAudioCodec(const char *codec, SAudioFormat::Channels channels, int sampleRate, int bitRate)
{
  setCodec(codec, channels, sampleRate, bitRate);
}

/*! Initializes a codec with the specified name, channels, sample rate and bit rate.
 */
SAudioCodec::SAudioCodec(const QString &codec, SAudioFormat::Channels channels, int sampleRate, int bitRate)
{
  setCodec(codec, channels, sampleRate, bitRate);
}

bool SAudioCodec::operator==(const SAudioCodec &comp) const
{
  return (d.codec == comp.d.codec) && (d.channels == comp.d.channels) &&
      (d.sampleRate == comp.d.sampleRate) && (d.bitRate == comp.d.bitRate);
}

void SAudioCodec::setCodec(const QString &codec, SAudioFormat::Channels channels, int sampleRate, int bitRate)
{
  d.codec = codec;
  d.channels = channels;
  d.sampleRate = sampleRate;
  d.bitRate = bitRate;
}

QString SAudioCodec::toString(bool addExtraData) const
{
  QString result = d.codec + ';' +
    QString::number(d.channels) + ';' +
    QString::number(d.sampleRate) + ';' +
    QString::number(d.bitRate);

  if (addExtraData && !d.extraData.isEmpty())
    result += ';' + QString::fromAscii(d.extraData.toBase64());

  return result;
}

SAudioCodec SAudioCodec::fromString(const QString &str)
{
  const QStringList items = str.split(';');
  SAudioCodec result;

  if (items.count() >= 4)
  {
    result.d.codec = items[0].toAscii();
    result.d.channels = SAudioFormat::Channels(items[1].toInt());
    result.d.sampleRate = items[2].toInt();
    result.d.bitRate = items[3].toInt();
  }

  if (items.count() >= 5)
    result.d.extraData = QByteArray::fromBase64(items[4].toAscii());

  return result;
}

} // End of namespace
