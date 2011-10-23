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
    callback(NULL)
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

bool TsBufferWriter::start(SInterfaces::BufferWriter::WriteCallback *c, bool /*sequential*/)
{
  callback = c;

  if (bufferWriter)
    return bufferWriter->start(this, true);

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

void TsBufferWriter::write(const uchar *buffer, qint64 size)
{
  static const quint32 timeCode = 0;

  if (callback)
  for (qint64 i=0; i<size; )
  {
    const MPEG::TSPacket * const tsPacket =
        reinterpret_cast<const MPEG::TSPacket *>(buffer + i);

    if (tsPacket->isValid())
    {
      callback->write(reinterpret_cast<const uchar *>(tsPacket), MPEG::tsPacketSize);
      callback->write(reinterpret_cast<const uchar *>(&timeCode), sizeof(timeCode));

      i += MPEG::tsPacketSize;
    }
    else
      i++;
  }
}

qint64 TsBufferWriter::seek(qint64 offset, int whence)
{
  if (whence == SEEK_SET)
    return -1;
  else if (whence == SEEK_CUR)
    return -1;
  else if (whence == SEEK_END)
    return -1;
  else if (whence == -1) // get size
    return callback->seek(offset, whence);

  return -1;
}

} } // End of namespaces
