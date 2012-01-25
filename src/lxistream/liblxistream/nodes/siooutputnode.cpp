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

#include "nodes/siooutputnode.h"
#include "nodes/saudioencodernode.h"
#include "nodes/svideoencodernode.h"

namespace LXiStream {

struct SIOOutputNode::Data
{
  inline Data(void) : mutex(QMutex::Recursive) { }

  QMutex                        mutex;
  QIODevice                   * ioDevice;
  SInterfaces::BufferWriter   * bufferWriter;

  float                         streamingSpeed;
  STime                         streamingPreload;
  STimer                        streamTimer;
};

SIOOutputNode::SIOOutputNode(SGraph *parent, QIODevice *ioDevice)
  : SInterfaces::SinkNode(parent),
    d(new Data())
{
  d->ioDevice = ioDevice;
  d->bufferWriter = NULL;

  d->streamingSpeed = 0.0f;
}

SIOOutputNode::~SIOOutputNode()
{
  delete d->bufferWriter;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QStringList SIOOutputNode::formats(void)
{
  return SInterfaces::BufferWriter::available();
}

void SIOOutputNode::setIODevice(QIODevice *ioDevice)
{
  d->ioDevice = ioDevice;
}

bool SIOOutputNode::hasIODevice(void) const
{
  return d->ioDevice != NULL;
}

bool SIOOutputNode::openFormat(const QString &format)
{
  delete d->bufferWriter;
  d->bufferWriter = SInterfaces::BufferWriter::create(this, format, false);

  return d->bufferWriter != NULL;
}

SInterfaces::BufferWriter * SIOOutputNode::bufferWriter(void)
{
  return d->bufferWriter;
}

void SIOOutputNode::enablePseudoStreaming(float speed, STime preload)
{
  d->streamingSpeed = speed;
  d->streamingPreload = preload;
}

bool SIOOutputNode::start(STimer *)
{
  if (d->ioDevice && d->bufferWriter)
  if (d->ioDevice->isOpen())
  {
    d->ioDevice->moveToThread(thread());
    d->ioDevice->setParent(this);

    connect(d->ioDevice, SIGNAL(readChannelFinished()), SLOT(close()));
    if (d->ioDevice->metaObject()->indexOfSignal("disconnected()") >= 0)
      connect(d->ioDevice, SIGNAL(disconnected()), SLOT(close()));

    return d->bufferWriter->start(d->ioDevice);
  }

  return false;
}

void SIOOutputNode::stop(void)
{
  if (d->bufferWriter && d->ioDevice)
    d->bufferWriter->stop();

  close();
}

void SIOOutputNode::input(const SEncodedAudioBuffer &buffer)
{
  LXI_PROFILE_WAIT(d->mutex.lock());
  Q_ASSERT(QThread::currentThread() == thread());

  if (!qFuzzyCompare(d->streamingSpeed, 0.0f))
    blockUntil(buffer.decodingTimeStamp().isValid() ? buffer.decodingTimeStamp() : buffer.presentationTimeStamp());

  if (d->bufferWriter && d->ioDevice)
    d->bufferWriter->process(buffer);

  d->mutex.unlock();
}

void SIOOutputNode::input(const SEncodedVideoBuffer &buffer)
{
  LXI_PROFILE_WAIT(d->mutex.lock());
  Q_ASSERT(QThread::currentThread() == thread());

  if (!qFuzzyCompare(d->streamingSpeed, 0.0f))
    blockUntil(buffer.decodingTimeStamp().isValid() ? buffer.decodingTimeStamp() : buffer.presentationTimeStamp());

  if (d->bufferWriter && d->ioDevice)
    d->bufferWriter->process(buffer);

  d->mutex.unlock();
}

void SIOOutputNode::input(const SEncodedDataBuffer &buffer)
{
  LXI_PROFILE_WAIT(d->mutex.lock());
  Q_ASSERT(QThread::currentThread() == thread());

  if (!qFuzzyCompare(d->streamingSpeed, 0.0f))
    blockUntil(buffer.decodingTimeStamp().isValid() ? buffer.decodingTimeStamp() : buffer.presentationTimeStamp());

  if (d->bufferWriter && d->ioDevice)
    d->bufferWriter->process(buffer);

  d->mutex.unlock();
}

void SIOOutputNode::close(void)
{
  LXI_PROFILE_WAIT(d->mutex.lock());

  emit closed(d->ioDevice);
  d->ioDevice = NULL;

  d->mutex.unlock();
}

void SIOOutputNode::blockUntil(STime timeStamp)
{
  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  static const int maxDelay = 250;

  if (timeStamp >= d->streamingPreload)
  {
    const STime correctedTime = STime::fromMSec(qint64(float(timeStamp.toMSec()) / d->streamingSpeed));

    // This blocks the thread until it is time to process the buffer.
    const STime duration = d->streamTimer.correctOffset(correctedTime, STime::fromMSec(maxDelay));
    if (duration.isPositive())
      T::msleep(qBound(0, int(duration.toMSec()), maxDelay));
  }
}


} // End of namespace
