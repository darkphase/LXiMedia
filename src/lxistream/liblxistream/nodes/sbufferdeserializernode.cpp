/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include "nodes/sbufferdeserializernode.h"

namespace LXiStream {

struct SBufferDeserializerNode::Data
{
  QIODevice                   * ioDevice;
};

SBufferDeserializerNode::SBufferDeserializerNode(SGraph *parent, QIODevice *ioDevice)
  : SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->ioDevice = ioDevice;
}

void SBufferDeserializerNode::setIODevice(QIODevice *ioDevice)
{
  d->ioDevice = ioDevice;
}

bool SBufferDeserializerNode::hasIODevice(void) const
{
  return d->ioDevice != NULL;
}

SBufferDeserializerNode::~SBufferDeserializerNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SBufferDeserializerNode::start(void)
{
  if (d->ioDevice && d->ioDevice->isOpen())
  {
    d->ioDevice->setParent(this);

    connect(d->ioDevice, SIGNAL(readChannelFinished()), SIGNAL(finished()));

    return true;
  }

  return false;
}

void SBufferDeserializerNode::stop(void)
{
}

template <>
void SBufferDeserializerNode::deserialize<QByteArray>(void)
{
  struct { quint32 dataLen; } header = { 0 };
  if (read(d->ioDevice, reinterpret_cast<char *>(&header), sizeof(header)))
  if (header.dataLen <= 536870912) // 512 MiB
  {
    QByteArray buffer;
    buffer.resize(header.dataLen);

    if (read(d->ioDevice, buffer.data(), header.dataLen))
      emit output(buffer);
  }
}

template <class _buffer>
void SBufferDeserializerNode::deserialize(void)
{
  struct { quint32 metaLen, dataLen; } header = { 0, 0 };
  if (read(d->ioDevice, reinterpret_cast<char *>(&header), sizeof(header)))
  if (header.dataLen <= 536870912) // 512 MiB
  {
    _buffer buffer;
    buffer.resize(header.dataLen);

    if (header.metaLen == sizeof(buffer.d))
    if (read(d->ioDevice, reinterpret_cast<char *>(&buffer.d), header.metaLen))
    if (read(d->ioDevice, buffer.data(), header.dataLen))
      emit output(buffer);
  }
}

bool SBufferDeserializerNode::process(void)
{
  if (d->ioDevice && d->ioDevice->isOpen())
  while (d->ioDevice->bytesAvailable() >= 4)
  {
    quint32 bufferId = 0;
    if (read(d->ioDevice, reinterpret_cast<char *>(&bufferId), sizeof(bufferId)))
    {
      switch (bufferId)
      {
      case 0xA5B60100: deserialize<QByteArray>();           return true;
      case 0xA5B60101: deserialize<SAudioBuffer>();         return true;
      case 0xA5B60102: deserialize<SVideoBuffer>();         return true;
      case 0xA5B60103: deserialize<SSubtitleBuffer>();      return true;
      case 0xA5B60104: deserialize<SSubpictureBuffer>();    return true;
      }
    }
    else
      break;
  }

  return false;
}

bool SBufferDeserializerNode::read(QIODevice *ioDevice, char *buffer, unsigned length)
{
  for (unsigned i=0; i<length; )
  {
    if (ioDevice->bytesAvailable() > 0)
    {
      const qint64 r = ioDevice->read(buffer + i, length - i);
      if (r > 0)
        i += r;
      else
        return false;
    }
    else if (ioDevice->waitForReadyRead(5000) == false)
      return false;
  }

  return true;
}

} // End of namespace
