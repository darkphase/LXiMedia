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
  SVideoDecoderNode::Flags      flags;
  SVideoCodec                   lastCodec;
  SInterfaces::VideoDecoder   * decoder;
  QFuture<void>                 future;
};

SVideoDecoderNode::SVideoDecoderNode(SGraph *parent, Flags flags)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
  d->flags = flags;
  d->decoder = NULL;
}

SVideoDecoderNode::~SVideoDecoderNode()
{
  d->future.waitForFinished();
  delete d->decoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SVideoDecoderNode::codecs(void)
{
  return SInterfaces::VideoDecoder::available();
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
  d->future.waitForFinished();
}

void SVideoDecoderNode::input(const SEncodedVideoBuffer &videoBuffer)
{
  d->future.waitForFinished();

  if (!videoBuffer.isNull())
  {
    if (d->lastCodec != videoBuffer.codec())
    {
      delete d->decoder;

      d->lastCodec = videoBuffer.codec();
      d->decoder = SInterfaces::VideoDecoder::create(NULL, d->lastCodec, d->flags, false);

      if (d->decoder == NULL)
        qWarning() << "Failed to find video decoder for codec" << d->lastCodec.codec();
    }
  }

  if (d->decoder)
    d->future = QtConcurrent::run(this, &SVideoDecoderNode::processTask, videoBuffer);
  else
    emit output(SVideoBuffer());
}

void SVideoDecoderNode::processTask(const SEncodedVideoBuffer &videoBuffer)
{
  foreach (const SVideoBuffer &buffer, d->decoder->decodeBuffer(videoBuffer))
    emit output(buffer);
}

} // End of namespace
