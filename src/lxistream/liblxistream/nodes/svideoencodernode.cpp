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

#include "nodes/svideoencodernode.h"
#include "svideobuffer.h"

namespace LXiStream {

struct SVideoEncoderNode::Data
{
  SInterfaces::VideoEncoder   * encoder;
};

SVideoEncoderNode::SVideoEncoderNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->encoder = NULL;
}

SVideoEncoderNode::~SVideoEncoderNode()
{
  delete d->encoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SVideoEncoderNode::codecs(void)
{
  return SInterfaces::VideoEncoder::available();
}

QStringList SVideoEncoderNode::losslessCodecs(void)
{
  static const QSet<QString> losslessCodecs = QSet<QString>()
      << "LJPEG" << "MLP" << "MSZH" << "ZLIB";

  return (QSet<QString>::fromList(codecs()) & losslessCodecs).toList();
}

bool SVideoEncoderNode::openCodec(const SVideoCodec &codec, SIOOutputNode *outputNode, STime duration, SInterfaces::VideoEncoder::Flags flags)
{
  SInterfaces::BufferWriter * const bufferWriter =
      outputNode ? outputNode->bufferWriter() : NULL;

  delete d->encoder;
  d->encoder = SInterfaces::VideoEncoder::create(this, codec, bufferWriter, flags, false);

  if (d->encoder)
  {
    if (bufferWriter)
      return bufferWriter->addStream(d->encoder, duration);
    else
      return true;
  }
  else
    qDebug() << "SVideoEncoderNode::openCodec: Open the SIOOutputNode format first,";

  return false;
}

SVideoCodec SVideoEncoderNode::codec(void) const
{
  if (d->encoder)
    return d->encoder->codec();

  return SVideoCodec();
}

const SInterfaces::VideoEncoder * SVideoEncoderNode::encoder(void) const
{
  return d->encoder;
}

bool SVideoEncoderNode::start(void)
{
  return d->encoder;
}

void SVideoEncoderNode::stop(void)
{
}

void SVideoEncoderNode::input(const SVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  if (d->encoder)
  foreach (const SEncodedVideoBuffer &buffer, d->encoder->encodeBuffer(videoBuffer))
    emit output(buffer);
}

} // End of namespace
