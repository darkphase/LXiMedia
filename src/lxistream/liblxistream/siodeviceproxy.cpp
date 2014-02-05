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

#include "siodeviceproxy.h"
#include "sbuffer.h"

namespace LXiStream {

class SIODeviceProxy::Reader : public QIODevice
{
public:
  explicit                      Reader(SIODeviceProxy *, QObject *parent = NULL);
  virtual                       ~Reader();

  void                          detach();

public: // From QIODevice
  virtual bool                  isSequential() const;
  virtual bool                  open(OpenMode mode);
  virtual void                  close(void);

  virtual bool                  seek(qint64 pos);
  virtual qint64                pos() const;
  virtual qint64                size(void) const;
  virtual qint64                bytesAvailable() const;

  virtual qint64                readData(char *data, qint64 maxSize);
  virtual qint64                writeData(const char *, qint64);

private:
  void                          setReadPos(qint64);

private:
  QExplicitlySharedDataPointer<Data> parent;
  bool                          opened;
  qint64                        readPos;
  QTime                         lastRead;
};

struct SIODeviceProxy::Data : public QSharedData
{
  Data() : mutex(QMutex::NonRecursive) { }

  mutable QMutex                mutex;
  QWaitCondition                writePossible;
  QWaitCondition                readPossible;

  bool                          opened;
  SBuffer                       buffer;
  qint64                        absolutePos;
  QSet<Reader *>                readers;
  qint64                        available;
  QTime                         timer;
};

SIODeviceProxy::SIODeviceProxy(QObject *parent)
  : QIODevice(parent),
    d(new Data())
{
  d->opened = false;
}

SIODeviceProxy::~SIODeviceProxy()
{
  QMutexLocker l(&d->mutex);

  d->opened = false;

  d->writePossible.wakeOne();
}

QIODevice * SIODeviceProxy::createReader(QObject *parent)
{
  QMutexLocker l(&d->mutex);

  return new Reader(this, parent);
}

bool SIODeviceProxy::isActive() const
{
  QMutexLocker l(&d->mutex);

  return
      !d->readers.isEmpty() ||
      ((d->absolutePos == 0) && (qAbs(d->timer.elapsed()) < 5000));
}

bool SIODeviceProxy::isReusable() const
{
  QMutexLocker l(&d->mutex);

  return (d->absolutePos == 0) && (qAbs(d->timer.elapsed()) < 4000);
}

bool SIODeviceProxy::isSequential() const
{
  return true;
}

bool SIODeviceProxy::open(OpenMode mode)
{
  QMutexLocker l(&d->mutex);

  if (!d->opened && (mode == QIODevice::WriteOnly))
  if (QIODevice::open(mode))
  {
    d->opened = true;
    d->buffer.resize(4096 * 1024);
    d->absolutePos = 0;
    d->available = 0;

    return true;
  }

  return false;
}

void SIODeviceProxy::close(void)
{
  emit aboutToClose();

  QMutexLocker l(&d->mutex);

  d->opened = false;

  d->writePossible.wakeOne();
  d->readPossible.wakeAll();

  QIODevice::close();
}

bool SIODeviceProxy::seek(qint64)
{
  QMutexLocker l(&d->mutex);

  return false;
}

qint64 SIODeviceProxy::pos() const
{
  QMutexLocker l(&d->mutex);

  return d->absolutePos + d->available;
}

qint64 SIODeviceProxy::size(void) const
{
  return bytesAvailable();
}

qint64 SIODeviceProxy::bytesAvailable() const
{
  return 0;
}

qint64 SIODeviceProxy::readData(char *, qint64)
{
  return -1;
}

qint64 SIODeviceProxy::writeData(const char *data, qint64 maxSize)
{
  QMutexLocker l(&d->mutex);

  qint64 written = 0;
  while (d->opened && (maxSize > 0))
  {
    qint64 freeSize = 0;
    while (d->opened && ((freeSize = (d->buffer.size() - d->available)) <= 0))
      d->writePossible.wait(&d->mutex);

    if (freeSize == 0)
      break;

    const qint64 relativePos = (d->absolutePos + d->available) % d->buffer.size();
    const qint64 copy = qMin(qMin(maxSize, freeSize), d->buffer.size() - relativePos);
    memcpy(d->buffer.data() + relativePos, data, copy);

    maxSize -= copy;
    data += copy;
    written += copy;

    d->available += copy;
    d->readPossible.wakeAll();
  }

  return (written > 0) ? written : -1;
}


SIODeviceProxy::Reader::Reader(SIODeviceProxy *parent, QObject *parentObject)
  : QIODevice(parentObject),
    parent(parent->d),
    opened(false),
    readPos(0)
{
}

SIODeviceProxy::Reader::~Reader()
{
  QMutexLocker l(&parent->mutex);

  detach();
}

void SIODeviceProxy::Reader::detach()
{
  if (opened)
  {
    parent->readers.remove(this);
    opened = false;

    parent->readPossible.wakeAll();
  }
}

bool SIODeviceProxy::Reader::isSequential() const
{
  return true;
}

bool SIODeviceProxy::Reader::open(OpenMode mode)
{
  QMutexLocker l(&parent->mutex);

  if (!opened && (parent->absolutePos == 0) && (mode == QIODevice::ReadOnly))
  if (QIODevice::open(mode))
  {
    readPos = parent->absolutePos;
    opened = true;
    parent->readers.insert(this);
    parent->timer.start();

    return true;
  }

  return false;
}

void SIODeviceProxy::Reader::close(void)
{
  emit aboutToClose();

  QMutexLocker l(&parent->mutex);

  detach();

  QIODevice::close();
}

bool SIODeviceProxy::Reader::seek(qint64 pos)
{
  QMutexLocker l(&parent->mutex);

  if (opened && (pos >= parent->absolutePos) && (pos <= (parent->absolutePos + parent->available)))
  {
    QIODevice::seek(pos);
    setReadPos(pos);
    return true;
  }

  return false;
}

qint64 SIODeviceProxy::Reader::pos() const
{
  QMutexLocker l(&parent->mutex);

  return readPos;
}

qint64 SIODeviceProxy::Reader::size(void) const
{
  return bytesAvailable();
}

qint64 SIODeviceProxy::Reader::bytesAvailable() const
{
  QMutexLocker l(&parent->mutex);

  if (opened)
    return parent->available - (readPos - parent->absolutePos);

  return 0;
}

qint64 SIODeviceProxy::Reader::readData(char *data, qint64 maxSize)
{
  QMutexLocker l(&parent->mutex);

  qint64 read = 0;
  while (opened && (maxSize > 0))
  {
    qint64 available = 0;
    while (opened && ((available = (parent->available - (readPos - parent->absolutePos))) <= 0) && parent->opened)
      parent->readPossible.wait(&parent->mutex);

    if (available == 0)
      break; // Parent has stopped writing.

    const qint64 relativePos = readPos % parent->buffer.size();
    const qint64 copy = qMin(qMin(maxSize, available), parent->buffer.size() - relativePos);

    memcpy(data, parent->buffer.data() + relativePos, copy);

    setReadPos(readPos + copy);

    maxSize -= copy;
    data += copy;
    read += copy;

    lastRead.start();
  }

  return (read > 0) ? read : -1;
}

qint64 SIODeviceProxy::Reader::writeData(const char *, qint64)
{
  return -1;
}

void SIODeviceProxy::Reader::setReadPos(qint64 pos)
{
  readPos = pos;

  qint64 newAbsolutePos = parent->absolutePos + parent->available;
  foreach (Reader *reader, parent->readers)
  {
    if (((parent->available - (reader->readPos - parent->absolutePos)) >= parent->buffer.size()) &&
        (reader->lastRead.elapsed() > 1000))
    {
      reader->detach();
    }
    else
      newAbsolutePos = qMin(newAbsolutePos, reader->readPos);
  }

  // Initially fill up the buffer until all readers read at least 3/4 of the buffer.
  if ((newAbsolutePos > parent->absolutePos) &&
      ((parent->absolutePos > 0) || (newAbsolutePos >= (parent->buffer.size() * 3 / 4))))
  {
    parent->available -= (newAbsolutePos - parent->absolutePos);
    parent->absolutePos = newAbsolutePos;

    parent->writePossible.wakeOne();
  }
}

} // End of namespace
