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

#include "tsbufferwriter.h"
#include "mpeg.h"

namespace LXiStream {
namespace Common {

TsBufferWriter::TsBufferWriter(const QString &, QObject *parent)
  : SInterfaces::BufferWriter(parent),
    bufferWriter(NULL),
    filter(this),
    ioDevice(NULL)
{
}

TsBufferWriter::~TsBufferWriter()
{
  delete bufferWriter;
}

bool TsBufferWriter::openFormat(const QString &name)
{
  if (name == "m2ts")
  {
    delete bufferWriter;
    bufferWriter = SInterfaces::BufferWriter::create(this, "mpegts", false);

    if (bufferWriter)
      filter.open(QIODevice::WriteOnly);

    return bufferWriter;
  }

  return false;
}

bool TsBufferWriter::addStream(const SInterfaces::AudioEncoder *encoder, STime duration)
{
  if (bufferWriter)
    return bufferWriter->addStream(encoder, duration);

  return false;
}

bool TsBufferWriter::addStream(const SInterfaces::VideoEncoder *encoder, STime duration)
{
  if (bufferWriter)
    return bufferWriter->addStream(encoder, duration);

  return false;
}

bool TsBufferWriter::start(QIODevice *d)
{
  ioDevice = d;

  if (bufferWriter)
    return bufferWriter->start(&filter);

  return false;
}

void TsBufferWriter::stop(void)
{
  if (bufferWriter)
    bufferWriter->stop();
}

void TsBufferWriter::process(const SEncodedAudioBuffer &buffer)
{
  if (bufferWriter)
    bufferWriter->process(buffer);
}

void TsBufferWriter::process(const SEncodedVideoBuffer &buffer)
{
  if (bufferWriter)
    bufferWriter->process(buffer);
}

void TsBufferWriter::process(const SEncodedDataBuffer &buffer)
{
  if (bufferWriter)
    bufferWriter->process(buffer);
}


TsBufferWriter::Filter::Filter(TsBufferWriter *parent)
  : QIODevice(parent),
    parent(parent),
    bufferSize(0)
{
}

TsBufferWriter::Filter::~Filter()
{
}

qint64 TsBufferWriter::Filter::readData(char *, qint64)
{
  return -1;
}

qint64 TsBufferWriter::Filter::writeData(const char *data, qint64 size)
{
  static const quint32 timeCode = 0;

  if (parent->ioDevice)
  {
    qint64 i = 0;

    if (bufferSize > 0)
    {
      const size_t count = qMin(size_t(size), MPEG::tsPacketSize - bufferSize);
      memcpy(buffer, data, count);
      bufferSize += count;
      i += count;

      Q_ASSERT(bufferSize <= MPEG::tsPacketSize);
      if (bufferSize >= MPEG::tsPacketSize)
      {
        parent->ioDevice->write(reinterpret_cast<const char *>(&timeCode), sizeof(timeCode));
        parent->ioDevice->write(buffer, MPEG::tsPacketSize);
        bufferSize = 0;
      }
    }

    while (parent->ioDevice->bytesToWrite() >= outBufferSize)
    if (!parent->ioDevice->waitForBytesWritten(-1))
      return -1;

    while (i < size)
    {
      if (data[i] == MPEG::tsSyncByte)
      {
        if ((i + qint64(MPEG::tsPacketSize)) <= size)
        {
          parent->ioDevice->write(reinterpret_cast<const char *>(&timeCode), sizeof(timeCode));
          parent->ioDevice->write(data, MPEG::tsPacketSize);

          i += MPEG::tsPacketSize;
        }
        else
          break;
      }
      else
        i++;
    }

    bufferSize = size - i;
    if (bufferSize > 0)
      memcpy(buffer, data + i, bufferSize);

    return i + bufferSize;
  }

  return -1;
}

} } // End of namespaces
