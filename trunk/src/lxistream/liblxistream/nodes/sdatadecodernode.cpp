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

#include "nodes/sdatadecodernode.h"
#include "ssubtitlebuffer.h"
#include "sgraph.h"

namespace LXiStream {

struct SDataDecoderNode::Data
{
  SDataDecoderNode::Flags       flags;
  SDataCodec                    lastCodec;
  SInterfaces::DataDecoder    * decoder;
};

SDataDecoderNode::SDataDecoderNode(SGraph *parent, Flags flags)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->flags = flags;
  d->decoder = NULL;
}

SDataDecoderNode::~SDataDecoderNode()
{
  delete d->decoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SDataDecoderNode::codecs(void)
{
  return SInterfaces::AudioDecoder::available();
}

SDataDecoderNode::Flags SDataDecoderNode::flags(void) const
{
  return d->flags;
}

void SDataDecoderNode::setFlags(SDataDecoderNode::Flags flags)
{
  d->flags = flags;
}

void SDataDecoderNode::input(const SEncodedDataBuffer &dataBuffer)
{
  // Open the correct codec. The previous codec is not deleted as there may
  // still be tasts that depend on it, it will be deleted by Qt when this object
  // is destroyed.
  if (!dataBuffer.isNull() && (d->lastCodec != dataBuffer.codec()))
  {
    d->lastCodec = dataBuffer.codec();
    d->decoder = SInterfaces::DataDecoder::create(NULL, d->lastCodec, d->flags, false);

    if (d->decoder == NULL)
      qWarning() << "Failed to find data decoder for codec" << d->lastCodec.codec();
  }

  if (graph)
    graph->runTask(this, &SDataDecoderNode::process, dataBuffer, d->decoder);
  else
    process(dataBuffer, d->decoder);
}

void SDataDecoderNode::process(const SEncodedDataBuffer &dataBuffer, SInterfaces::DataDecoder *decoder)
{
  if (decoder)
  foreach (const SSubtitleBuffer &buffer, decoder->decodeBuffer(dataBuffer))
    emit output(buffer);
}

} // End of namespace