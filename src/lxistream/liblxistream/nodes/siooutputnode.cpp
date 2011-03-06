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

const int SIOOutputNode::outBufferSize = 2097152;
const int SIOOutputNode::outBufferDelay = outBufferSize / (1048576 / 1000); // msec

struct SIOOutputNode::Data
{
  QIODevice                   * ioDevice;
  SInterfaces::BufferWriter   * bufferWriter;
};

SIOOutputNode::SIOOutputNode(SGraph *parent, QIODevice *ioDevice)
  : QObject(parent),
    SGraph::SinkNode(parent),
    d(new Data())
{
  d->ioDevice = ioDevice;
  d->bufferWriter = NULL;
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

bool SIOOutputNode::start(STimer *)
{
  if (d->ioDevice && d->bufferWriter)
  if (d->ioDevice->isOpen())
  {
    return d->bufferWriter->start(this);
  }

  return false;
}

void SIOOutputNode::stop(void)
{
  if (d->bufferWriter)
    d->bufferWriter->stop();

  if (d->ioDevice)
  while (d->ioDevice->bytesToWrite() > 0)
  if (!d->ioDevice->waitForBytesWritten(outBufferDelay))
    break;
}

void SIOOutputNode::input(const SEncodedAudioBuffer &buffer)
{
  if (d->bufferWriter)
    d->bufferWriter->process(buffer);
}

void SIOOutputNode::input(const SEncodedVideoBuffer &buffer)
{
  if (d->bufferWriter)
    d->bufferWriter->process(buffer);
}

void SIOOutputNode::input(const SEncodedDataBuffer &buffer)
{
  if (d->bufferWriter)
    d->bufferWriter->process(buffer);
}

void SIOOutputNode::write(const uchar *buffer, qint64 size)
{
  if (d->ioDevice)
  {
    while (d->ioDevice->bytesToWrite() >= outBufferSize)
    if (!d->ioDevice->waitForBytesWritten(-1))
      return; // Error, don't write anything.

    for (qint64 i=0; i<size; )
    {
      const qint64 r = d->ioDevice->write((char *)buffer + i, size - i);
      if (r > 0)
        i += r;
      else
        break;
    }
  }
}


} // End of namespace
