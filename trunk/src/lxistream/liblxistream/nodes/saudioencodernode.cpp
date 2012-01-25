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

#include "nodes/saudioencodernode.h"
#include "saudiobuffer.h"
#include "nodes/siooutputnode.h"

namespace LXiStream {

struct SAudioEncoderNode::Data
{
  SInterfaces::AudioEncoder   * encoder;
};

SAudioEncoderNode::SAudioEncoderNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->encoder = NULL;
}

SAudioEncoderNode::~SAudioEncoderNode()
{
  delete d->encoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SAudioEncoderNode::codecs(void)
{
  return SInterfaces::AudioEncoder::available();
}

QStringList SAudioEncoderNode::losslessCodecs(void)
{
  static const QSet<QString> losslessCodecs = QSet<QString>()
      << "ALAC" << "ALS" << "FLAC" << "SONICLS"
      << "PCM/S16LE" << "PCM/S16BE" << "PCM/U16LE" << "PCM/U16BE" << "PCM/S8"
      << "PCM/U8" << "PCM/MULAW" << "PCM/ALAW" << "PCM/S32LE" << "PCM/S32BE"
      << "PCM/U32LE" << "PCM/U32BE" << "PCM/S24LE" << "PCM/S24BE" << "PCM/U24LE"
      << "PCM/U24BE" << "PCM/S24DAUD" << "PCM/ZORK" << "PCM/S16LEP" << "PCM/DVD"
      << "PCM/F32BE" << "PCM/F32LE" << "PCM/F64BE" << "PCM/F64LE";

  return (QSet<QString>::fromList(codecs()) & losslessCodecs).toList();
}

bool SAudioEncoderNode::openCodec(const SAudioCodec &codec, SIOOutputNode *outputNode, STime duration, SInterfaces::AudioEncoder::Flags flags)
{
  SInterfaces::BufferWriter * const bufferWriter =
      outputNode ? outputNode->bufferWriter() : NULL;

  delete d->encoder;
  d->encoder = SInterfaces::AudioEncoder::create(this, codec, bufferWriter, flags, false);

  if (d->encoder)
  {
    if (bufferWriter)
      return bufferWriter->addStream(d->encoder, duration);
    else
      return true;
  }
  else
    qDebug() << "SAudioEncoderNode::openCodec: Open the SIOOutputNode format first,";

  return false;
}

SAudioCodec SAudioEncoderNode::codec(void) const
{
  if (d->encoder)
    return d->encoder->codec();

  return SAudioCodec();
}

const SInterfaces::AudioEncoder * SAudioEncoderNode::encoder(void) const
{
  return d->encoder;
}

bool SAudioEncoderNode::start(void)
{
  return d->encoder;
}

void SAudioEncoderNode::stop(void)
{

  delete d->encoder;
  d->encoder = NULL;
}

void SAudioEncoderNode::input(const SAudioBuffer &audioBuffer)
{
  if (d->encoder)
  foreach (const SEncodedAudioBuffer &buffer, d->encoder->encodeBuffer(audioBuffer))
    emit output(buffer);
}

} // End of namespace
