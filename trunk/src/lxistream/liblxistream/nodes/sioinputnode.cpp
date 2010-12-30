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
#include "sdebug.h"
#include "sgraph.h"

namespace LXiStream {

struct SIOInputNode::Data
{
  QIODevice                   * ioDevice;
  bool                          opened;
  SInterfaces::BufferReader   * bufferReader;
};

SIOInputNode::SIOInputNode(SGraph *parent, QIODevice *ioDevice)
  : QObject(parent),
    SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->ioDevice = ioDevice;
  d->opened = false;
  d->bufferReader = NULL;
}

SIOInputNode::~SIOInputNode()
{
  delete d->bufferReader;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SIOInputNode::setIODevice(QIODevice *ioDevice)
{
  d->ioDevice = ioDevice;
}

bool SIOInputNode::open(void)
{
  if (d->ioDevice)
  if (d->ioDevice->isOpen())
  {
    // Detect format.
    const QByteArray buffer = d->ioDevice->peek(65536);

    QMultiMap<int, QString> formats;
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(this))
    {
      foreach (const SInterfaces::FormatProber::Format &format, prober->probeFormat(buffer))
        formats.insert(-format.confidence, format.name);

      delete prober;
    }

    // Now try to open a parser.
    foreach (const QString &format, formats)
    {
      d->bufferReader = SInterfaces::BufferReader::create(this, format, false);
      if (d->bufferReader)
      {
        if (d->bufferReader->start(this, d->ioDevice->isSequential()))
          return d->opened = true;

        delete d->bufferReader;
        d->bufferReader = NULL;
      }
    }
  }

  return false;
}

STime SIOInputNode::duration(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->duration();

  return STime();
}

bool SIOInputNode::setPosition(STime pos)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->setPosition(pos);

  return false;
}

STime SIOInputNode::position(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->position();

  return STime();
}

QList<SIOInputNode::AudioStreamInfo> SIOInputNode::audioStreams(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->audioStreams();

  return QList<AudioStreamInfo>();
}

QList<SIOInputNode::VideoStreamInfo> SIOInputNode::videoStreams(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->videoStreams();

  return QList<VideoStreamInfo>();
}

QList<SIOInputNode::DataStreamInfo> SIOInputNode::dataStreams(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->dataStreams();

  return QList<DataStreamInfo>();
}

void SIOInputNode::selectStreams(const QList<quint16> &streamIds)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    d->bufferReader->selectStreams(streamIds);
}

bool SIOInputNode::start(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (!d->opened)
    open();

  return d->opened;
}

void SIOInputNode::stop(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
  {
    d->bufferReader->stop();

    delete d->bufferReader;
    d->bufferReader = NULL;
  }
}

void SIOInputNode::process(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->ioDevice && d->bufferReader)
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
    else if (whence == SEEK_SET)
      return d->ioDevice->seek(d->ioDevice->pos() + offset) ? 0 : -1;
    else if (whence == SEEK_END)
      return d->ioDevice->seek(d->ioDevice->size() + offset) ? 0 : -1;
    else if (whence == -1) // get size
      return d->ioDevice->size();
  }

  return -1;
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