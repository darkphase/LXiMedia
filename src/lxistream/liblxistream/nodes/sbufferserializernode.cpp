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

#include "nodes/sbufferserializernode.h"

namespace LXiStream {

struct SBufferSerializerNode::Data
{
  inline Data(void) : mutex(QMutex::Recursive) { }

  QMutex                        mutex;
  QIODevice                   * ioDevice;
  bool                          autoClose;
};

SBufferSerializerNode::SBufferSerializerNode(SGraph *parent, QIODevice *ioDevice)
  : SInterfaces::SinkNode(parent),
    d(new Data())
{
  d->ioDevice = ioDevice;
  d->autoClose = false;
}

SBufferSerializerNode::~SBufferSerializerNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SBufferSerializerNode::setIODevice(QIODevice *ioDevice, bool autoClose)
{
  d->ioDevice = ioDevice;
  d->autoClose = autoClose;
}

bool SBufferSerializerNode::hasIODevice(void) const
{
  return d->ioDevice != NULL;
}

bool SBufferSerializerNode::start(STimer *)
{
  if (d->ioDevice && d->ioDevice->isOpen())
  {
    d->ioDevice->setParent(this);

    connect(d->ioDevice, SIGNAL(readChannelFinished()), SLOT(close()));
    if (d->ioDevice->metaObject()->indexOfSignal("disconnected()") >= 0)
      connect(d->ioDevice, SIGNAL(disconnected()), SLOT(close()));

    return true;
  }

  return false;
}

void SBufferSerializerNode::stop(void)
{
  closed();
}

template <>
void SBufferSerializerNode::serialize(const QByteArray &buffer, quint32 bufferId)
{
  LXI_PROFILE_WAIT(d->mutex.lock());

  if (d->ioDevice)
  {
    while (d->ioDevice->bytesToWrite() > 262144)
    if (d->ioDevice->waitForBytesWritten(5000) == false)
    {
      d->mutex.unlock();

      close();
      return;
    }

    const struct { quint32 bufferId, dataLen; } header = { bufferId, buffer.size() };

    d->ioDevice->write(reinterpret_cast<const char *>(&header), sizeof(header));
    d->ioDevice->write(buffer.data(), header.dataLen);
  }

  d->mutex.unlock();
}

template <class _buffer>
void SBufferSerializerNode::serialize(const _buffer &buffer, quint32 bufferId)
{
  LXI_PROFILE_WAIT(d->mutex.lock());

  if (d->ioDevice)
  {
    while (d->ioDevice->bytesToWrite() > 262144)
    if (!d->ioDevice->waitForBytesWritten(-1))
    {
      d->mutex.unlock();

      close();
      return;
    }

    const struct { quint32 bufferId, metaLen, dataLen; } header = { bufferId, sizeof(buffer.d), buffer.size() };
    d->ioDevice->write(reinterpret_cast<const char *>(&header), sizeof(header));
    d->ioDevice->write(reinterpret_cast<const char *>(&buffer.d), header.metaLen);
    d->ioDevice->write(buffer.data(), header.dataLen);
  }

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

void SBufferSerializerNode::close(void)
{
  LXI_PROFILE_WAIT(d->mutex.lock());

  if (d->ioDevice)
  {
    QIODevice * const device = d->ioDevice; // May be called recursively from close().
    d->ioDevice = NULL;

    if (d->autoClose)
    {
      device->close();
      device->deleteLater();
    }

    //qDebug() << "SBufferSerializerNode: Client disconnected";
    emit closed();
  }

  d->mutex.unlock();
}

} // End of namespace
