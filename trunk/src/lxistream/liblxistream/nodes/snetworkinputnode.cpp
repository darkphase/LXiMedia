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
  quint16                       programId;
  STime                         bufferDuration;
  SInterfaces::NetworkBufferReader * bufferReader;

  QSemaphore                    bufferSem;
  bool                          bufferState;
  QFuture<void>                 future;
  QFutureWatcher<void>          futureWatcher;
};

SNetworkInputNode::SNetworkInputNode(SGraph *parent, const QUrl &url)
  : SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->url = url;
  d->programId = 0;
  d->bufferDuration = STime::fromSec(10);
  d->bufferReader = NULL;
  d->bufferSem.release(1);
  d->bufferState = false;

  connect(&d->futureWatcher, SIGNAL(finished()), SLOT(fillBuffer()), Qt::QueuedConnection);
}

SNetworkInputNode::~SNetworkInputNode()
{
  d->future.waitForFinished();
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SNetworkInputNode::setBufferDuration(const STime &bufferDuration)
{
  d->bufferDuration = bufferDuration;
}

STime SNetworkInputNode::bufferDuration(void) const
{
  return d->bufferDuration;
}

bool SNetworkInputNode::open(quint16 programId)
{
  d->bufferReader = SInterfaces::NetworkBufferReader::create(this, d->url.scheme(), false);
  d->programId = programId;
  d->bufferState = false;

  return true;
}

bool SNetworkInputNode::start(void)
{
  if (d->bufferReader == NULL)
    open();

  if (d->bufferReader)
  {
    if (d->bufferReader->start(d->url, this, d->programId))
      return true;

    delete d->bufferReader;
    d->bufferReader = NULL;
  }

  return false;
}

void SNetworkInputNode::stop(void)
{
  d->future.waitForFinished();

  if (d->bufferReader)
  {
    d->bufferReader->stop();

    delete d->bufferReader;
    d->bufferReader = NULL;
  }
}

bool SNetworkInputNode::process(void)
{
  LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);

  bool result = false;
  if (d->bufferReader)
  {
    if (d->bufferSem.tryAcquire(1, 15))
    {
      if (d->bufferState)
      if (!d->bufferReader->process())
      {
        // Finished; close input.
        d->bufferReader->stop();

        delete d->bufferReader;
        d->bufferReader = NULL;

        emit output(SEncodedAudioBuffer());
        emit output(SEncodedVideoBuffer());
        emit output(SEncodedDataBuffer());
        emit finished();
      }

      d->bufferSem.release(1);
      result = true;
    }

    fillBuffer();
  }

  return result;
}

STime SNetworkInputNode::duration(void) const
{
  d->future.waitForFinished();

  if (d->bufferReader)
    return d->bufferReader->duration();

  return STime();
}

bool SNetworkInputNode::setPosition(STime pos)
{
  d->future.waitForFinished();

  if (d->bufferReader)
    return d->bufferReader->setPosition(pos);

  return false;
}

STime SNetworkInputNode::position(void) const
{
  d->future.waitForFinished();

  if (d->bufferReader)
    return d->bufferReader->position();

  return STime();
}

QList<SNetworkInputNode::Chapter> SNetworkInputNode::chapters(void) const
{
  d->future.waitForFinished();

  if (d->bufferReader)
    return d->bufferReader->chapters();

  return QList<Chapter>();
}

QList<SNetworkInputNode::AudioStreamInfo> SNetworkInputNode::audioStreams(void) const
{
  d->future.waitForFinished();

  if (d->bufferReader)
    return d->bufferReader->audioStreams();

  return QList<AudioStreamInfo>();
}

QList<SNetworkInputNode::VideoStreamInfo> SNetworkInputNode::videoStreams(void) const
{
  d->future.waitForFinished();

  if (d->bufferReader)
    return d->bufferReader->videoStreams();

  return QList<VideoStreamInfo>();
}

QList<SNetworkInputNode::DataStreamInfo> SNetworkInputNode::dataStreams(void) const
{
  d->future.waitForFinished();

  if (d->bufferReader)
    return d->bufferReader->dataStreams();

  return QList<DataStreamInfo>();
}

void SNetworkInputNode::selectStreams(const QVector<StreamId> &streamIds)
{
  d->future.waitForFinished();

  if (d->bufferReader)
    d->bufferReader->selectStreams(streamIds);
}

void SNetworkInputNode::fillBuffer(void)
{
  if (d->bufferReader)
  if (d->bufferSem.tryAcquire(1, 0))
  {
    const STime currentDuration = d->bufferReader->bufferDuration();
    const float currentProgress = float(currentDuration.toMSec()) / float(d->bufferDuration.toMSec());

    if (currentDuration < d->bufferDuration)
    {
      if (d->future.isFinished())
      {
        d->future = QtConcurrent::run(this, &SNetworkInputNode::bufferTask);
        d->futureWatcher.setFuture(d->future);
      }

      if (d->bufferState && (currentDuration < STime::fromSec(1)))
        emit bufferState(d->bufferState = false, currentProgress);
      else if (!d->bufferState)
        emit bufferState(d->bufferState, currentProgress);
    }
    else if (!d->bufferState)
      emit bufferState(d->bufferState = true, currentProgress);

    d->bufferSem.release(1);
  }
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

void SNetworkInputNode::bufferTask(void)
{
  LXI_PROFILE_WAIT(d->bufferSem.acquire(1));
  LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);

  d->bufferReader->buffer();

  d->bufferSem.release(1);
}

} // End of namespace
