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

#include "bufferreader.h"

namespace LXiStream {
namespace FFMpegBackend {

BufferReader::BufferReader(const QString &, QObject *parent)
  : SInterfaces::BufferReader(parent),
    ioDevice(NULL),
    format(NULL),
    ioContext(NULL)
{
}

BufferReader::~BufferReader()
{
  if (ioContext)
  {
    ::av_free(ioContext->buffer);
    ::av_free(ioContext);
  }
}

bool BufferReader::openFormat(const QString &name)
{
  format = ::av_find_input_format(name.toLatin1().data());

  // Do not allow redirecting.
  if (format)
  if (qstrcmp(format->name, "redir") == 0)
    format = NULL;

  return format != NULL;
}

bool BufferReader::start(QIODevice *ioDevice, ProduceCallback *produceCallback, bool streamed, bool fast)
{
  if (format)
  {
    if (ioContext)
      qFatal("BufferReader already opened a stream.");

    static const int ioBufferSize = 350 * 188;

    this->ioDevice = ioDevice;

    ioContext = ::avio_alloc_context(
        (unsigned char *)::av_malloc(ioBufferSize),
        ioBufferSize,
        false,
        this,
        &BufferReader::read,
        NULL,
        &BufferReader::seek);

    ioContext->seekable = streamed ? 0 : AVIO_SEEKABLE_NORMAL;

    ::AVFormatContext * formatContext = avformat_alloc_context();
    formatContext->pb = ioContext;
    if (::avformat_open_input(&formatContext, "", format, NULL) == 0)
      return BufferReaderBase::start(produceCallback, formatContext, fast);
  }

  return false;
}

void BufferReader::stop(void)
{
  BufferReaderBase::stop();

  if (ioContext)
  {
    ::av_free(ioContext->buffer);
    ::av_free(ioContext);
    ioContext = NULL;
  }

  ioDevice = NULL;
}

int BufferReader::read(void *opaque, uint8_t *buf, int buf_size)
{
  return reinterpret_cast<BufferReader *>(opaque)->ioDevice->read(reinterpret_cast<char *>(buf), buf_size);
}

int64_t BufferReader::seek(void *opaque, int64_t offset, int whence)
{
  QIODevice * const ioDevice = reinterpret_cast<BufferReader *>(opaque)->ioDevice;

  if (whence == SEEK_SET)
    return ioDevice->seek(offset) ? 0 : -1;
  else if (whence == SEEK_CUR)
    return ioDevice->seek(ioDevice->pos() + offset) ? 0 : -1;
  else if (whence == SEEK_END)
    return ioDevice->seek(ioDevice->size() + offset) ? 0 : -1;
  else if (whence == AVSEEK_SIZE)
    return ioDevice->size();

  return -1;
}


} } // End of namespaces
