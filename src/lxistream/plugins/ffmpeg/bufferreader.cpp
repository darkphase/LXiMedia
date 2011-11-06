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

#include "bufferreader.h"

namespace LXiStream {
namespace FFMpegBackend {

BufferReader::BufferReader(const QString &, QObject *parent)
  : SInterfaces::BufferReader(parent),
    readCallback(NULL),
    format(NULL),
    ioContext(NULL)
{
}

BufferReader::~BufferReader()
{
  if (ioContext)
  {
    ::av_free(ioContext->buffer);
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 0, 0)
    ::av_free(ioContext);
#endif
  }
}

bool BufferReader::openFormat(const QString &name)
{
  format = ::av_find_input_format(name.toAscii().data());

  // Do not allow redirecting.
  if (format)
  if (qstrcmp(format->name, "redir") == 0)
    format = NULL;

  return format != NULL;
}

bool BufferReader::start(ReadCallback *readCallback, ProduceCallback *produceCallback, quint16 /*programId*/, bool streamed)
{
  if (format)
  {
    if (ioContext)
      qFatal("BufferReader already opened a stream.");

    static const int ioBufferSize = 350 * 188;

    this->readCallback = readCallback;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
    ioContext = ::avio_alloc_context(
#else
    ioContext = ::av_alloc_put_byte(
#endif
        (unsigned char *)::av_malloc(ioBufferSize),
        ioBufferSize,
        false,
        this,
        &BufferReader::read,
        NULL,
        &BufferReader::seek);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
    ioContext->seekable = streamed ? 0 : AVIO_SEEKABLE_NORMAL;
#else
    ioContext->is_streamed = streamed;
#endif

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
    ::AVFormatContext * formatContext = avformat_alloc_context();
    formatContext->pb = ioContext;
    if (::avformat_open_input(&formatContext, "", format, NULL) == 0)
      return BufferReaderBase::start(produceCallback, formatContext);
#else
    ::AVFormatContext * formatContext = NULL;
    if (::av_open_input_stream(&formatContext, ioContext, "", format, NULL) == 0)
      return BufferReaderBase::start(produceCallback, formatContext);
#endif
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

  readCallback = NULL;
}

int BufferReader::read(void *opaque, uint8_t *buf, int buf_size)
{
  return reinterpret_cast<BufferReader *>(opaque)->readCallback->read(buf, buf_size);
}

int64_t BufferReader::seek(void *opaque, int64_t offset, int whence)
{
  return reinterpret_cast<BufferReader *>(opaque)->readCallback->seek(offset, (whence == AVSEEK_SIZE) ? -1 : whence);
}


} } // End of namespaces
