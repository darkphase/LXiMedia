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

#include "nodes/svideodecodernode.h"
#include "svideobuffer.h"

namespace LXiStream {

struct SVideoDecoderNode::Data
{
  SIOInputNode                * inputNode;
  SVideoDecoderNode::Flags      flags;
  SVideoCodec                   lastCodec;
  SInterfaces::VideoDecoder   * decoder;
};

SVideoDecoderNode::SVideoDecoderNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->inputNode = NULL;
  d->flags = SInterfaces::VideoDecoder::Flag_None;
  d->decoder = NULL;
}

SVideoDecoderNode::~SVideoDecoderNode()
{
  delete d->decoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SVideoDecoderNode::codecs(void)
{
  return SInterfaces::VideoDecoder::available();
}

bool SVideoDecoderNode::open(SIOInputNode *inputNode, Flags flags)
{
  d->inputNode = inputNode;
  d->flags = flags;

  return true;
}

SVideoDecoderNode::Flags SVideoDecoderNode::flags(void) const
{
  return d->flags;
}

void SVideoDecoderNode::setFlags(SVideoDecoderNode::Flags flags)
{
  d->flags = flags;
}

bool SVideoDecoderNode::start(void)
{
  return true;
}

void SVideoDecoderNode::stop(void)
{
}

void SVideoDecoderNode::input(const SEncodedVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
  {
    if (d->lastCodec != videoBuffer.codec())
    {
      delete d->decoder;

      SInterfaces::BufferReader * const bufferReader =
          d->inputNode ? d->inputNode->bufferReader() : NULL;

      d->lastCodec = videoBuffer.codec();
      d->decoder = SInterfaces::VideoDecoder::create(NULL, d->lastCodec, bufferReader, d->flags, false);

      if (d->decoder == NULL)
        qWarning() << "Failed to find video decoder for codec" << d->lastCodec.codec();
    }
  }

  if (d->decoder)
  foreach (const SVideoBuffer &buffer, d->decoder->decodeBuffer(videoBuffer))
    emit output(buffer);
}

} // End of namespace
