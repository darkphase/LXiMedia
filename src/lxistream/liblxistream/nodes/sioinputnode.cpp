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

#include "nodes/sioinputnode.h"

namespace LXiStream {

struct SIOInputNode::Data
{
  SScheduler::Dependency      * dependency;
  QIODevice                   * ioDevice;
  bool                          opened;
  SInterfaces::BufferReader   * bufferReader;
};

SIOInputNode::SIOInputNode(SGraph *parent, QIODevice *ioDevice, const QString &path)
  : QObject(parent),
    SGraph::SourceNode(parent),
    SInterfaces::BufferReader::ReadCallback(path),
    d(new Data())
{
  d->dependency = parent ? new SScheduler::Dependency(parent) : NULL;
  d->ioDevice = ioDevice;
  d->opened = false;
  d->bufferReader = NULL;
}

SIOInputNode::~SIOInputNode()
{
  delete d->dependency;
  delete d->bufferReader;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SIOInputNode::setIODevice(QIODevice *ioDevice)
{
  d->ioDevice = ioDevice;
}

bool SIOInputNode::open(quint16 programId)
{
  QByteArray buffer;
  bool sequential = true;

  if (d->ioDevice && d->ioDevice->isOpen())
  {
    buffer = d->ioDevice->peek(SInterfaces::FormatProber::defaultProbeSize);
    sequential = d->ioDevice->isSequential();
  }

  QMultiMap<int, QString> formats;
  foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(this))
  {
    foreach (const SInterfaces::FormatProber::Format &format, prober->probeFormat(buffer, path))
      formats.insert(-format.confidence, format.name);

    delete prober;
  }

  // Now try to open a parser.
  foreach (const QString &format, formats)
  {
    d->bufferReader = SInterfaces::BufferReader::create(this, format, false);
    if (d->bufferReader)
    {
      if (d->bufferReader->start(this, this, programId, sequential))
        return d->opened = true;

      delete d->bufferReader;
      d->bufferReader = NULL;
    }
  }

  return false;
}

bool SIOInputNode::start(void)
{
  if (!d->opened)
    open(0);

  return d->opened;
}

void SIOInputNode::stop(void)
{
  if (d->bufferReader)
  {
    d->bufferReader->stop();

    delete d->bufferReader;
    d->bufferReader = NULL;
  }
}

void SIOInputNode::process(void)
{
  if (d->bufferReader)
  {
    if (!d->ioDevice->atEnd())
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

qint64 SIOInputNode::read(uchar *buffer, qint64 size)
{
  if (d->ioDevice)
    return d->ioDevice->read((char *)buffer, size);

  return -1;
}

qint64 SIOInputNode::seek(qint64 offset, int whence)
{
  if (d->ioDevice)
  {
    if (whence == SEEK_SET)
      return d->ioDevice->seek(offset) ? 0 : -1;
    else if (whence == SEEK_CUR)
      return d->ioDevice->seek(d->ioDevice->pos() + offset) ? 0 : -1;
    else if (whence == SEEK_END)
      return d->ioDevice->seek(d->ioDevice->size() + offset) ? 0 : -1;
    else if (whence == -1) // get size
      return d->ioDevice->size();
  }

  return -1;
}

STime SIOInputNode::duration(void) const
{
  if (d->bufferReader)
    return d->bufferReader->duration();

  return STime();
}

bool SIOInputNode::setPosition(STime pos)
{
  if (d->bufferReader)
    return d->bufferReader->setPosition(pos);

  return false;
}

STime SIOInputNode::position(void) const
{
  if (d->bufferReader)
    return d->bufferReader->position();

  return STime();
}

QList<SIOInputNode::Chapter> SIOInputNode::chapters(void) const
{
  if (d->bufferReader)
    return d->bufferReader->chapters();

  return QList<Chapter>();
}

QList<SIOInputNode::AudioStreamInfo> SIOInputNode::audioStreams(void) const
{
  if (d->bufferReader)
    return d->bufferReader->audioStreams();

  return QList<AudioStreamInfo>();
}

QList<SIOInputNode::VideoStreamInfo> SIOInputNode::videoStreams(void) const
{
  if (d->bufferReader)
    return d->bufferReader->videoStreams();

  return QList<VideoStreamInfo>();
}

QList<SIOInputNode::DataStreamInfo> SIOInputNode::dataStreams(void) const
{
  if (d->bufferReader)
    return d->bufferReader->dataStreams();

  return QList<DataStreamInfo>();
}

void SIOInputNode::selectStreams(const QList<StreamId> &streamIds)
{
  if (d->bufferReader)
    d->bufferReader->selectStreams(streamIds);
}

void SIOInputNode::produce(const SEncodedAudioBuffer &buffer)
{
  emit output(buffer);
}

void SIOInputNode::produce(const SEncodedVideoBuffer &buffer)
{
  emit output(buffer);
}

void SIOInputNode::produce(const SEncodedDataBuffer &buffer)
{
  emit output(buffer);
}


} // End of namespace
