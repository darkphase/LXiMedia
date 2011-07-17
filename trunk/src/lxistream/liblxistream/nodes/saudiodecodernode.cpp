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

namespace LXiStream {

struct SAudioDecoderNode::Data
{
  SAudioDecoderNode::Flags      flags;
  SAudioCodec                   lastCodec;
  SInterfaces::AudioDecoder   * decoder;
  QFuture<void>                 future;
};

SAudioDecoderNode::SAudioDecoderNode(SGraph *parent, Flags flags)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->flags = flags;
  d->decoder = NULL;
}

SAudioDecoderNode::~SAudioDecoderNode()
{
  d->future.waitForFinished();
  delete d->decoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SAudioDecoderNode::codecs(void)
{
  return SInterfaces::AudioDecoder::available();
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
  d->future.waitForFinished();
}

void SAudioDecoderNode::input(const SEncodedAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;

  d->future.waitForFinished();

  if (!audioBuffer.isNull())
  {
    if (d->lastCodec != audioBuffer.codec())
    {
      delete d->decoder;

      d->lastCodec = audioBuffer.codec();
      d->decoder = SInterfaces::AudioDecoder::create(NULL, d->lastCodec, d->flags, false);

      if (d->decoder == NULL)
        qWarning() << "Failed to find audio decoder for codec" << d->lastCodec.codec();
    }
  }

  if (d->decoder)
    d->future = QtConcurrent::run(this, &SAudioDecoderNode::processTask, audioBuffer);
  else
    emit output(SAudioBuffer());
}

void SAudioDecoderNode::processTask(const SEncodedAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;

  foreach (const SAudioBuffer &buffer, d->decoder->decodeBuffer(audioBuffer))
    emit output(buffer);
}

} // End of namespace
