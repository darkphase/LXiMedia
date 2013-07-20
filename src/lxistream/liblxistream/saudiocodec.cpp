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
  d.name = QByteArray();
  d.channels = SAudioFormat::Channel_None;
  d.sampleRate = 0;
  d.streamId = -1;
  d.bitRate = 0;
  d.frameSize = 0;
}

/*! Initializes a codec with the specified name, channels, sample rate and bit rate.
 */
SAudioCodec::SAudioCodec(const char *name, SAudioFormat::Channels channels, int sampleRate, int streamId, int bitRate, int frameSize)
{
  setCodec(name, channels, sampleRate, streamId, bitRate, frameSize);
}

/*! Initializes a codec with the specified name, channels, sample rate and bit rate.
 */
SAudioCodec::SAudioCodec(const QByteArray &name, SAudioFormat::Channels channels, int sampleRate, int streamId, int bitRate, int frameSize)
{
  setCodec(name, channels, sampleRate, streamId, bitRate, frameSize);
}

bool SAudioCodec::operator==(const SAudioCodec &comp) const
{
  return (d.name == comp.d.name) && (d.channels == comp.d.channels) &&
      (d.sampleRate == comp.d.sampleRate) && (d.streamId == comp.d.streamId) &&
      (d.bitRate == comp.d.bitRate) && (d.frameSize == comp.d.frameSize);
}

void SAudioCodec::setCodec(const QByteArray &name, SAudioFormat::Channels channels, int sampleRate, int streamId, int bitRate, int frameSize)
{
  d.name = name;
  d.channels = channels;
  d.sampleRate = sampleRate;
  d.streamId = streamId;
  d.bitRate = bitRate;
  d.frameSize = frameSize;
}

void SAudioCodec::serialize(QXmlStreamWriter &writer) const
{
  writer.writeStartElement("audiocodec");

  writer.writeAttribute("name", d.name);
  writer.writeAttribute("channels", QString::number(d.channels, 16));
  writer.writeAttribute("samplerate", QString::number(d.sampleRate));
  writer.writeAttribute("streamid", QString::number(d.streamId));
  writer.writeAttribute("bitrate", QString::number(d.bitRate));
  writer.writeAttribute("framesize", QString::number(d.frameSize));

  writer.writeEndElement();
}

bool SAudioCodec::deserialize(QXmlStreamReader &reader)
{
  if (reader.name() == "audiocodec")
  {
    d.name = reader.attributes().value("name").toString().toLatin1();
    d.channels = SAudioFormat::Channels(reader.attributes().value("channels").toString().toInt(NULL, 16));
    d.sampleRate = reader.attributes().value("samplerate").toString().toInt();
    d.streamId = reader.attributes().value("streamid").toString().toInt();
    d.bitRate = reader.attributes().value("bitrate").toString().toInt();
    d.frameSize = reader.attributes().value("framesize").toString().toInt();

    return true;
  }

  reader.raiseError("Not an audiocodec element.");
  return false;
}

} // End of namespace
