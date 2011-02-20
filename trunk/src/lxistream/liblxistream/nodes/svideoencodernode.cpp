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
#include "sgraph.h"
#include "svideobuffer.h"

namespace LXiStream {

struct SVideoEncoderNode::Data
{
  SDependency                 * dependency;
  SInterfaces::VideoEncoder   * encoder;
};

SVideoEncoderNode::SVideoEncoderNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->dependency = parent ? new SDependency() : NULL;
}

SVideoEncoderNode::~SVideoEncoderNode()
{
  delete d->dependency;
  delete d->encoder;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SVideoEncoderNode::codecs(void)
{
  return SInterfaces::VideoEncoder::available();
}

bool SVideoEncoderNode::openCodec(const SVideoCodec &codec, SInterfaces::VideoEncoder::Flags flags)
{
  delete d->encoder;
  d->encoder = SInterfaces::VideoEncoder::create(this, codec, flags, false);

  return d->encoder;
}

SVideoCodec SVideoEncoderNode::codec(void) const
{
  if (d->encoder)
    return d->encoder->codec();

  return SVideoCodec();
}

void SVideoEncoderNode::input(const SVideoBuffer &videoBuffer)
{
  if (d->encoder)
  {
    if (graph)
      graph->queue(this, &SVideoEncoderNode::processTask, videoBuffer, d->dependency);
    else
      processTask(videoBuffer);
  }
}

void SVideoEncoderNode::processTask(const SVideoBuffer &videoBuffer)
{
  foreach (const SEncodedVideoBuffer &buffer, d->encoder->encodeBuffer(videoBuffer))
    emit output(buffer);
}


} // End of namespace
