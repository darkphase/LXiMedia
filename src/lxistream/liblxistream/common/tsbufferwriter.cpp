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
    parent(parent)
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
    while (parent->ioDevice->bytesToWrite() >= outBufferSize)
    if (!parent->ioDevice->waitForBytesWritten(-1))
      return -1;

    qint64 i = 0;
    while ((i + qint64(MPEG::tsPacketSize)) <= size)
    {
      const MPEG::TSPacket * const tsPacket =
          reinterpret_cast<const MPEG::TSPacket *>(data + i);

      if (tsPacket->isValid())
      {
        parent->ioDevice->write(reinterpret_cast<const char *>(tsPacket), MPEG::tsPacketSize);
        parent->ioDevice->write(reinterpret_cast<const char *>(&timeCode), sizeof(timeCode));

        i += MPEG::tsPacketSize;
      }
      else
        i++;
    }

    return i;
  }

  return -1;
}

} } // End of namespaces
