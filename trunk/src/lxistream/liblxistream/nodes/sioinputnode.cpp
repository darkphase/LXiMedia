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
  QIODevice                   * ioDevice;
};

SIOInputNode::SIOInputNode(SGraph *parent, QIODevice *ioDevice, quint16 programId)
  : SInputNode(parent),
    SInterfaces::BufferReader::ReadCallback(QString::null),
    d(new Data())
{
  setIODevice(ioDevice, programId);
}

SIOInputNode::~SIOInputNode()
{
  delete bufferReader();
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SIOInputNode::setIODevice(QIODevice *ioDevice, quint16 programId)
{
  if (bufferReader())
  {
    delete bufferReader();
    setBufferReader(NULL);
  }

  d->ioDevice = ioDevice;

  if (ioDevice && ioDevice->isOpen())
  {
    const QByteArray buffer = ioDevice->peek(SInterfaces::FormatProber::defaultProbeSize);

    QMultiMap<int, QString> formats;
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(this))
    {
      foreach (const SInterfaces::FormatProber::Format &format, prober->probeFormat(buffer, QString::null))
        formats.insert(-format.confidence, format.name);

      delete prober;
    }

    // Now try to open a parser.
    foreach (const QString &format, formats)
    {
      SInterfaces::BufferReader * const bufferReader = SInterfaces::BufferReader::create(this, format, false);
      if (bufferReader)
      {
        if (bufferReader->start(this, this, programId, ioDevice->isSequential()))
        {
          setBufferReader(bufferReader);
          return;
        }

        delete bufferReader;
      }
    }
  }

  d->ioDevice = NULL;
}

const QIODevice * SIOInputNode::ioDevice(void) const
{
  return d->ioDevice;
}

QIODevice * SIOInputNode::ioDevice(void)
{
  return d->ioDevice;
}

bool SIOInputNode::start(void)
{
  if (d->ioDevice)
    return SInputNode::start();

  return false;
}

void SIOInputNode::stop(void)
{
  SInterfaces::BufferReader * const bufferReader = static_cast<SInterfaces::BufferReader *>(SInputNode::bufferReader());
  if (bufferReader)
    bufferReader->stop();

  SInputNode::stop();
}

bool SIOInputNode::process(void)
{
  if (d->ioDevice)
  {
    if (!d->ioDevice->atEnd())
      return SInputNode::process();

    endReached();

    return true;
  }

  return false;
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

void SIOInputNode::endReached(void)
{
  while (SInputNode::process())
    continue;

  stop();

  emit output(SEncodedAudioBuffer());
  emit output(SEncodedVideoBuffer());
  emit output(SEncodedDataBuffer());
  emit finished();
}

} // End of namespace
