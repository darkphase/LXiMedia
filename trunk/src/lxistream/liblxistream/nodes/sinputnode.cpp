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

#include "nodes/sinputnode.h"

namespace LXiStream {

struct SInputNode::Data
{
  SInterfaces::AbstractBufferReader * bufferReader;
};

SInputNode::SInputNode(SGraph *parent, SInterfaces::AbstractBufferReader *bufferReader)
  : SInterfaces::SourceNode(parent),
    d(new Data())
{
  setBufferReader(bufferReader);
}

SInputNode::~SInputNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SInputNode::setBufferReader(SInterfaces::AbstractBufferReader *bufferReader)
{
  d->bufferReader = bufferReader;
}

const SInterfaces::AbstractBufferReader * SInputNode::bufferReader(void) const
{
  return d->bufferReader;
}

SInterfaces::AbstractBufferReader * SInputNode::bufferReader(void)
{
  return d->bufferReader;
}

STime SInputNode::duration(void) const
{
  if (d->bufferReader)
    return d->bufferReader->duration();

  return STime();
}

bool SInputNode::setPosition(STime pos)
{
  if (d->bufferReader)
    return d->bufferReader->setPosition(pos);

  return false;
}

STime SInputNode::position(void) const
{
  if (d->bufferReader)
    return d->bufferReader->position();

  return STime();
}

QList<SInputNode::Chapter> SInputNode::chapters(void) const
{
  if (d->bufferReader)
    return d->bufferReader->chapters();

  return QList<Chapter>();
}

QList<SInputNode::AudioStreamInfo> SInputNode::audioStreams(void) const
{
  if (d->bufferReader)
    return d->bufferReader->audioStreams();

  return QList<AudioStreamInfo>();
}

QList<SInputNode::VideoStreamInfo> SInputNode::videoStreams(void) const
{
  if (d->bufferReader)
    return d->bufferReader->videoStreams();

  return QList<VideoStreamInfo>();
}

QList<SInputNode::DataStreamInfo> SInputNode::dataStreams(void) const
{
  if (d->bufferReader)
    return d->bufferReader->dataStreams();

  return QList<DataStreamInfo>();
}

void SInputNode::selectStreams(const QVector<StreamId> &streamIds)
{
  if (d->bufferReader)
    d->bufferReader->selectStreams(streamIds);
}

bool SInputNode::start(void)
{
  return true;
}

void SInputNode::stop(void)
{
  emit closeDecoder();
}

bool SInputNode::process(void)
{
  if (d->bufferReader)
    return d->bufferReader->process();

  return false;
}

void SInputNode::produce(const SEncodedAudioBuffer &buffer)
{
  emit output(buffer);
}

void SInputNode::produce(const SEncodedVideoBuffer &buffer)
{
  emit output(buffer);
}

void SInputNode::produce(const SEncodedDataBuffer &buffer)
{
  emit output(buffer);
}

} // End of namespace
