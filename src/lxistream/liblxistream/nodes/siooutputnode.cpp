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

#include "nodes/siooutputnode.h"

namespace LXiStream {

const int SIOOutputNode::outBufferSize = 65536;

struct SIOOutputNode::Data
{
  QIODevice                   * ioDevice;
  SInterfaces::BufferWriter   * bufferWriter;
  bool                          autoClose;

  float                         streamingSpeed;
  STime                         streamingPreload;
  STimer                        streamTimer;
};

SIOOutputNode::SIOOutputNode(SGraph *parent, QIODevice *ioDevice)
  : QObject(parent),
    SGraph::SinkNode(parent),
    d(new Data())
{
  d->ioDevice = ioDevice;
  d->bufferWriter = NULL;
  d->autoClose = false;

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

void SIOOutputNode::setIODevice(QIODevice *ioDevice, bool autoClose)
{
  d->ioDevice = ioDevice;
  d->autoClose = autoClose;
}

bool SIOOutputNode::openFormat(const QString &format, const SAudioCodec &audioCodec, STime duration)
{
  return openFormat(format,
                    QList<SAudioCodec>() << audioCodec,
                    QList<SVideoCodec>(),
                    duration);
}

bool SIOOutputNode::openFormat(const QString &format, const SAudioCodec &audioCodec, const SVideoCodec &videoCodec, STime duration)
{
  return openFormat(format,
                    QList<SAudioCodec>() << audioCodec,
                    QList<SVideoCodec>() << videoCodec,
                    duration);
}

bool SIOOutputNode::openFormat(const QString &format, const QList<SAudioCodec> &audioCodecs, const QList<SVideoCodec> &videoCodecs, STime duration)
{
  delete d->bufferWriter;
  d->bufferWriter = SInterfaces::BufferWriter::create(this, format, false);

  if (d->bufferWriter)
    return d->bufferWriter->createStreams(audioCodecs, videoCodecs, duration);

  return false;
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

    connect(d->ioDevice, SIGNAL(readChannelFinished()), SLOT(closed()));

    return d->bufferWriter->start(this);
  }

  return false;
}

void SIOOutputNode::stop(void)
{
  if (d->bufferWriter)
    d->bufferWriter->stop();

  QTime timer;
  timer.start();

  while (d->ioDevice && (d->ioDevice->bytesToWrite() >= 0))
  if (!d->ioDevice->waitForBytesWritten(qMax(0, 5000 - qAbs(timer.elapsed()))))
    break;

  closed();
}

void SIOOutputNode::input(const SEncodedAudioBuffer &buffer)
{
  LXI_PROFILE_FUNCTION;
  Q_ASSERT(QThread::currentThread() == thread());

  if (!qFuzzyCompare(d->streamingSpeed, 0.0f))
    blockUntil(buffer.decodingTimeStamp().isValid() ? buffer.decodingTimeStamp() : buffer.presentationTimeStamp());

  if (d->bufferWriter)
    d->bufferWriter->process(buffer);
}

void SIOOutputNode::input(const SEncodedVideoBuffer &buffer)
{
  LXI_PROFILE_FUNCTION;
  Q_ASSERT(QThread::currentThread() == thread());

  if (!qFuzzyCompare(d->streamingSpeed, 0.0f))
    blockUntil(buffer.decodingTimeStamp().isValid() ? buffer.decodingTimeStamp() : buffer.presentationTimeStamp());

  if (d->bufferWriter)
    d->bufferWriter->process(buffer);
}

void SIOOutputNode::input(const SEncodedDataBuffer &buffer)
{
  LXI_PROFILE_FUNCTION;
  Q_ASSERT(QThread::currentThread() == thread());

  if (!qFuzzyCompare(d->streamingSpeed, 0.0f))
    blockUntil(buffer.decodingTimeStamp().isValid() ? buffer.decodingTimeStamp() : buffer.presentationTimeStamp());

  if (d->bufferWriter)
    d->bufferWriter->process(buffer);
}

void SIOOutputNode::write(const uchar *buffer, qint64 size)
{
  if (d->ioDevice)
  while (d->ioDevice->bytesToWrite() >= outBufferSize)
  if (!d->ioDevice->waitForBytesWritten(-1))
    return;

  for (qint64 i=0; (i<size) && d->ioDevice; )
  {
    const qint64 r = d->ioDevice->write((char *)buffer + i, size - i);
    if (r > 0)
      i += r;
    else
      return;
  }
}

void SIOOutputNode::closed(void)
{
  if (d->ioDevice)
  {
    if (d->autoClose)
    {
      d->ioDevice->close();
      d->ioDevice->deleteLater();
    }

    d->ioDevice = NULL;
  }
}

void SIOOutputNode::blockUntil(STime timeStamp)
{
  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  static const int maxDelay = 250;

  if (timeStamp >= d->streamingPreload)
  {
    const STime correctedTime = STime::fromMSec(qint64(float(timeStamp.toMSec()) / d->streamingSpeed));

    // This blocks the thread until it is time to process the buffer. The
    // timestamp is divided by 2 to allow processing 2 times realtime.
    const STime duration = d->streamTimer.correctOffset(correctedTime, STime::fromMSec(maxDelay));
    if (duration.isPositive())
      T::msleep(qBound(0, int(duration.toMSec()), maxDelay));
  }
}


} // End of namespace
