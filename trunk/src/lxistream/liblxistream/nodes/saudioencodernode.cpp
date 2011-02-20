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

#include "nodes/saudioencodernode.h"
#include "saudiobuffer.h"
#include "sgraph.h"

namespace LXiStream {

struct SAudioEncoderNode::Data
{
  SDependency                 * dependency;
  SInterfaces::AudioEncoder   * encoder;
};

SAudioEncoderNode::SAudioEncoderNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->dependency = parent ? new SDependency() : NULL;
  d->encoder = NULL;
}

SAudioEncoderNode::~SAudioEncoderNode()
{
  delete d->dependency;
  delete d->encoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SAudioEncoderNode::codecs(void)
{
  return SInterfaces::AudioEncoder::available();
}

bool SAudioEncoderNode::openCodec(const SAudioCodec &codec, SInterfaces::AudioEncoder::Flags flags)
{
  delete d->encoder;
  d->encoder = SInterfaces::AudioEncoder::create(this, codec, flags, false);

  return d->encoder;
}

SAudioCodec SAudioEncoderNode::codec(void) const
{
  if (d->encoder)
    return d->encoder->codec();

  return SAudioCodec();
}

void SAudioEncoderNode::input(const SAudioBuffer &audioBuffer)
{
  if (d->encoder)
  {
    if (graph)
      graph->queue(this, &SAudioEncoderNode::processTask, audioBuffer, d->dependency);
    else
      processTask(audioBuffer);
  }
}

void SAudioEncoderNode::processTask(const SAudioBuffer &audioBuffer)
{
  foreach (const SEncodedAudioBuffer &buffer, d->encoder->encodeBuffer(audioBuffer))
    emit output(buffer);
}

} // End of namespace
