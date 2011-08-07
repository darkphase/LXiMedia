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

#include "nodes/sbufferserializernode.h"

namespace LXiStream {

struct SBufferSerializerNode::Data
{
  QMutex                        mutex;
  QIODevice                   * ioDevice;
};

SBufferSerializerNode::SBufferSerializerNode(SGraph *parent, QIODevice *ioDevice)
  : SInterfaces::SinkNode(parent),
    d(new Data())
{
  d->ioDevice = ioDevice;
}

SBufferSerializerNode::~SBufferSerializerNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SBufferSerializerNode::start(STimer *)
{
  return true;
}

void SBufferSerializerNode::stop(void)
{
}

template <>
void SBufferSerializerNode::serialize(const QByteArray &buffer, quint32 bufferId)
{
  LXI_PROFILE_WAIT(d->mutex.lock());
  LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);

  const struct { quint32 bufferId, dataLen; } header = { bufferId, buffer.size() };

  d->ioDevice->write(reinterpret_cast<const char *>(&header), sizeof(header));
  d->ioDevice->write(buffer.data(), header.dataLen);

  d->mutex.unlock();
}

template <class _buffer>
void SBufferSerializerNode::serialize(const _buffer &buffer, quint32 bufferId)
{
  LXI_PROFILE_WAIT(d->mutex.lock());
  LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);

  const struct { quint32 bufferId, metaLen, dataLen; } header = { bufferId, sizeof(buffer.d), buffer.size() };

  d->ioDevice->write(reinterpret_cast<const char *>(&header), sizeof(header));
  d->ioDevice->write(reinterpret_cast<const char *>(&buffer.d), header.metaLen);
  d->ioDevice->write(buffer.data(), header.dataLen);

  d->mutex.unlock();
}

void SBufferSerializerNode::input(const QByteArray &buffer)
{
  serialize(buffer, 0xA5B60100);
}

void SBufferSerializerNode::input(const SAudioBuffer &buffer)
{
  serialize(buffer, 0xA5B60101);
}

void SBufferSerializerNode::input(const SVideoBuffer &buffer)
{
  serialize(buffer, 0xA5B60102);
}

void SBufferSerializerNode::input(const SSubtitleBuffer &buffer)
{
  serialize(buffer, 0xA5B60103);
}

void SBufferSerializerNode::input(const SSubpictureBuffer &buffer)
{
  serialize(buffer, 0xA5B60104);
}

} // End of namespace
