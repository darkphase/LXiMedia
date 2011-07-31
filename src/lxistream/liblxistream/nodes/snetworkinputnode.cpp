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

#include "nodes/snetworkinputnode.h"
#include <LXiCore>

namespace LXiStream {


struct SNetworkInputNode::Data
{
  QUrl                          url;
  bool                          opened;
  SInterfaces::NetworkBufferReader * bufferReader;
};

SNetworkInputNode::SNetworkInputNode(SGraph *parent, const QUrl &url)
  : SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->url = url;
  d->opened = false;
  d->bufferReader = NULL;
}

SNetworkInputNode::~SNetworkInputNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SNetworkInputNode::open(quint16 programId)
{
  d->bufferReader = SInterfaces::NetworkBufferReader::create(this, d->url.scheme(), false);
  if (d->bufferReader)
  {
    if (d->bufferReader->start(d->url, this, programId))
      return d->opened = true;

    delete d->bufferReader;
    d->bufferReader = NULL;
  }

  return false;
}

bool SNetworkInputNode::start(void)
{
  if (!d->opened)
    open(0);

  return d->opened;
}

void SNetworkInputNode::stop(void)
{
  if (d->bufferReader)
  {
    d->bufferReader->stop();

    delete d->bufferReader;
    d->bufferReader = NULL;
  }
}

void SNetworkInputNode::process(void)
{
  LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);

  if (d->bufferReader)
  {
    if (d->bufferReader->process())
      return;

    // Finished; close input.
    d->bufferReader->stop();

    delete d->bufferReader;
    d->bufferReader = NULL;

    emit output(SEncodedAudioBuffer());
    emit output(SEncodedVideoBuffer());
    emit output(SEncodedDataBuffer());
    emit finished();
  }
}

STime SNetworkInputNode::duration(void) const
{
  if (d->bufferReader)
    return d->bufferReader->duration();

  return STime();
}

bool SNetworkInputNode::setPosition(STime pos)
{
  if (d->bufferReader)
    return d->bufferReader->setPosition(pos);

  return false;
}

STime SNetworkInputNode::position(void) const
{
  if (d->bufferReader)
    return d->bufferReader->position();

  return STime();
}

QList<SNetworkInputNode::Chapter> SNetworkInputNode::chapters(void) const
{
  if (d->bufferReader)
    return d->bufferReader->chapters();

  return QList<Chapter>();
}

QList<SNetworkInputNode::AudioStreamInfo> SNetworkInputNode::audioStreams(void) const
{
  if (d->bufferReader)
    return d->bufferReader->audioStreams();

  return QList<AudioStreamInfo>();
}

QList<SNetworkInputNode::VideoStreamInfo> SNetworkInputNode::videoStreams(void) const
{
  if (d->bufferReader)
    return d->bufferReader->videoStreams();

  return QList<VideoStreamInfo>();
}

QList<SNetworkInputNode::DataStreamInfo> SNetworkInputNode::dataStreams(void) const
{
  if (d->bufferReader)
    return d->bufferReader->dataStreams();

  return QList<DataStreamInfo>();
}

void SNetworkInputNode::selectStreams(const QList<StreamId> &streamIds)
{
  if (d->bufferReader)
    d->bufferReader->selectStreams(streamIds);
}

void SNetworkInputNode::produce(const SEncodedAudioBuffer &buffer)
{
  emit output(buffer);
}

void SNetworkInputNode::produce(const SEncodedVideoBuffer &buffer)
{
  emit output(buffer);
}

void SNetworkInputNode::produce(const SEncodedDataBuffer &buffer)
{
  emit output(buffer);
}

} // End of namespace
