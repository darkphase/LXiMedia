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

#include "nodes/saudiodecodernode.h"
#include "saudiobuffer.h"
#include "nodes/sioinputnode.h"
#include "nodes/splaylistnode.h"

namespace LXiStream {

struct SAudioDecoderNode::Data
{
  SInputNode                  * inputNode;
  SAudioDecoderNode::Flags      flags;
  SAudioCodec                   lastCodec;
  SInterfaces::AudioDecoder   * decoder;
};

SAudioDecoderNode::SAudioDecoderNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->inputNode = NULL;
  d->flags = SInterfaces::AudioDecoder::Flag_None;
  d->decoder = NULL;
}

SAudioDecoderNode::~SAudioDecoderNode()
{
  delete d->decoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SAudioDecoderNode::codecs(void)
{
  return SInterfaces::AudioDecoder::available();
}

bool SAudioDecoderNode::open(SInputNode *inputNode, Flags flags)
{
  d->inputNode = inputNode;
  d->flags = flags;

  connect(inputNode, SIGNAL(closeDecoder()), SLOT(closeDecoder()));

  return true;
}

SAudioDecoderNode::Flags SAudioDecoderNode::flags(void) const
{
  return d->flags;
}

void SAudioDecoderNode::setFlags(SAudioDecoderNode::Flags flags)
{
  d->flags = flags;
}

bool SAudioDecoderNode::start(void)
{
  return true;
}

void SAudioDecoderNode::stop(void)
{
}

void SAudioDecoderNode::input(const SEncodedAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull())
  {
    if ((d->decoder == NULL) || (d->lastCodec != audioBuffer.codec()))
    {
      delete d->decoder;

      SInterfaces::AbstractBufferReader * const bufferReader =
          d->inputNode ? d->inputNode->bufferReader() : NULL;

      d->lastCodec = audioBuffer.codec();
      d->decoder = SInterfaces::AudioDecoder::create(NULL, d->lastCodec, bufferReader, d->flags, false);

      if (d->decoder == NULL)
        qWarning() << "Failed to find audio decoder for codec" << d->lastCodec.codec();
    }
  }

  if (d->decoder)
  foreach (const SAudioBuffer &buffer, d->decoder->decodeBuffer(audioBuffer))
    emit output(buffer);
}

void SAudioDecoderNode::closeDecoder(void)
{
  delete d->decoder;
  d->decoder = NULL;
}

} // End of namespace
