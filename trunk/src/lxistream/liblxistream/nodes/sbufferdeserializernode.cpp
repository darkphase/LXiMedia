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

SBufferDeserializerNode::~SBufferDeserializerNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SBufferDeserializerNode::start(void)
{
  return true;
}

void SBufferDeserializerNode::stop(void)
{
}

template <>
void SBufferDeserializerNode::deserialize<QByteArray>(void)
{
  LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);

  struct { quint32 bufferId, dataLen; } header = { 0, 0 };

  for (unsigned i=0; i<sizeof(header); )
  {
    const qint64 r = d->ioDevice->read(reinterpret_cast<char *>(&header) + i, sizeof(header) - i);
    if (r > 0)
      i += r;
    else
      return;
  }

  if (header.dataLen <= 536870912) // 512 MiB
  {
    QByteArray buffer;
    buffer.resize(header.dataLen);

    for (unsigned i=0; i<header.dataLen; )
    {
      const qint64 r = d->ioDevice->read(buffer.data() + i, header.dataLen - i);
      if (r > 0)
        i += r;
      else
        return;
    }

    emit output(buffer);
  }
}

template <class _buffer>
void SBufferDeserializerNode::deserialize(void)
{
  LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);

  struct { quint32 bufferId, metaLen, dataLen; } header = { 0, 0, 0 };

  for (unsigned i=0; i<sizeof(header); )
  {
    const qint64 r = d->ioDevice->read(reinterpret_cast<char *>(&header) + i, sizeof(header) - i);
    if (r > 0)
      i += r;
    else
      return;
  }

  if (header.dataLen <= 536870912) // 512 MiB
  {
    _buffer buffer;
    buffer.resize(header.dataLen);

    if (header.metaLen == sizeof(buffer.d))
    {
      for (unsigned i=0; i<header.metaLen; )
      {
        const qint64 r = d->ioDevice->read(reinterpret_cast<char *>(&buffer.d) + i, header.metaLen - i);
        if (r > 0)
          i += r;
        else
          return;
      }

      for (unsigned i=0; i<header.dataLen; )
      {
        const qint64 r = d->ioDevice->read(buffer.data() + i, header.dataLen - i);
        if (r > 0)
          i += r;
        else
          return;
      }

      emit output(buffer);
    }
  }
}

void SBufferDeserializerNode::process(void)
{
  while (d->ioDevice->bytesAvailable() >= 4)
  {
    quint32 bufferId = 0;
    if (d->ioDevice->peek(reinterpret_cast<char *>(&bufferId), sizeof(bufferId)) == sizeof(bufferId))
    {
      switch (bufferId)
      {
      case 0xA5B60100: deserialize<QByteArray>();           return;
      case 0xA5B60101: deserialize<SAudioBuffer>();         return;
      case 0xA5B60102: deserialize<SVideoBuffer>();         return;
      case 0xA5B60103: deserialize<SSubtitleBuffer>();      return;
      case 0xA5B60104: deserialize<SSubpictureBuffer>();    return;

      default:
        // Read one byte to resync the stream.
        if (d->ioDevice->read(reinterpret_cast<char *>(&bufferId), 1) == 1)
          break;
        else
          return;
      }
    }
    else
      return;
  }
}

} // End of namespace
