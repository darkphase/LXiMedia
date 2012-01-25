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

#include "nodes/svideodecodernode.h"
#include "svideobuffer.h"

namespace LXiStream {

struct SVideoDecoderNode::Data
{
  SInputNode                  * inputNode;
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

bool SVideoDecoderNode::open(SInputNode *inputNode, Flags flags)
{
  d->inputNode = inputNode;
  d->flags = flags;

  connect(inputNode, SIGNAL(closeDecoder()), SLOT(closeDecoder()));

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
    if ((d->decoder == NULL) || (d->lastCodec != videoBuffer.codec()))
    {
      closeDecoder();

      SInterfaces::AbstractBufferReader * const bufferReader =
          d->inputNode ? d->inputNode->bufferReader() : NULL;

      d->lastCodec = videoBuffer.codec();
      d->decoder = SInterfaces::VideoDecoder::create(NULL, d->lastCodec, bufferReader, d->flags, false);

      if (d->decoder == NULL)
        qWarning() << "Failed to find video decoder for codec" << d->lastCodec.codec();
    }
  }

  if (d->decoder)
  {
    foreach (const SVideoBuffer &buffer, d->decoder->decodeBuffer(videoBuffer))
      emit output(buffer);
  }
  else if (videoBuffer.isNull())
    emit output(SVideoBuffer()); // Flush
}

void SVideoDecoderNode::closeDecoder(void)
{
  if (d->decoder)
  {
    // Flush
    foreach (const SVideoBuffer &buffer, d->decoder->decodeBuffer(SEncodedVideoBuffer()))
    if (!buffer.isNull())
      emit output(buffer);

    delete d->decoder;
    d->decoder = NULL;
  }
}

} // End of namespace
