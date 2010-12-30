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

QDomNode SAudioCodec::toXml(QDomDocument &doc) const
{
  QDomElement codecElm = createElement(doc, "audiocodec");

  if (!d.codec.isEmpty())
    codecElm.setAttribute("codec", QString(d.codec));

  codecElm.setAttribute("channels", d.channels);
  codecElm.setAttribute("samplerate", d.sampleRate);
  codecElm.setAttribute("bitrate", d.bitRate);

  if (!d.extraData.isEmpty())
    codecElm.setAttribute("extradata", QString(d.extraData.toBase64()));

  return codecElm;
}

void SAudioCodec::fromXml(const QDomNode &elm)
{
  QDomElement codecElm = findElement(elm, "audiocodec");

  if (codecElm.hasAttribute("codec"))
    d.codec = codecElm.attribute("codec").toAscii();

  d.channels = SAudioFormat::Channels(codecElm.attribute("channels").toUInt());
  d.sampleRate = codecElm.attribute("samplerate").toInt();
  d.bitRate = codecElm.attribute("bitrate").toInt();

  if (codecElm.hasAttribute("extradata"))
    d.extraData = QByteArray::fromBase64(codecElm.attribute("extradata").toAscii());
}

} // End of namespace
