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

namespace LXiStream {

struct SDataDecoderNode::Data
{
  SDataDecoderNode::Flags       flags;
  SDataCodec                    lastCodec;
  SInterfaces::DataDecoder    * decoder;
  QFuture<void>                 future;
};

SDataDecoderNode::SDataDecoderNode(SGraph *parent, Flags flags)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
  d->flags = flags;
  d->decoder = NULL;
}

SDataDecoderNode::~SDataDecoderNode()
{
  d->future.waitForFinished();
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

bool SDataDecoderNode::start(void)
{
  return true;
}

void SDataDecoderNode::stop(void)
{
  d->future.waitForFinished();
}

void SDataDecoderNode::input(const SEncodedDataBuffer &dataBuffer)
{
  d->future.waitForFinished();

  if (!dataBuffer.isNull())
  {
    if (d->lastCodec != dataBuffer.codec())
    {
      delete d->decoder;

      d->lastCodec = dataBuffer.codec();
      d->decoder = SInterfaces::DataDecoder::create(NULL, d->lastCodec, d->flags, false);

      if (d->decoder == NULL)
        qWarning() << "Failed to find data decoder for codec" << d->lastCodec.codec();
    }

    if (d->decoder)
      d->future = QtConcurrent::run(this, &SDataDecoderNode::processTask, dataBuffer);
  }
}

void SDataDecoderNode::processTask(const SEncodedDataBuffer &dataBuffer)
{
  foreach (const SDataBuffer &buffer, d->decoder->decodeBuffer(dataBuffer))
  switch(buffer.type())
  {
  case SDataBuffer::Type_None:
    break;

  case SDataBuffer::Type_SubtitleBuffer:
    emit output(buffer.subtitleBuffer());
    break;

  case SDataBuffer::Type_SubpictureBuffer:
    emit output(buffer.subpictureBuffer());
    break;
  }
}

} // End of namespace
