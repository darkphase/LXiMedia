/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef __BUFFERREADER_H
#define __BUFFERREADER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"
#include "bufferreaderbase.h"

namespace LXiStream {
namespace FFMpegBackend {

class BufferReader : public SInterfaces::BufferReader,
                     public BufferReaderBase
{
Q_OBJECT
public:
  explicit                      BufferReader(const QString &, QObject *);
  virtual                       ~BufferReader();

  inline bool                   process(bool fast)                              { return BufferReaderBase::demux(BufferReaderBase::read(fast)); }

public: // From SInterfaces::AbstractBufferReader
  inline virtual STime          duration(void) const                            { return BufferReaderBase::duration(); }
  inline virtual bool           setPosition(STime p)                            { return BufferReaderBase::setPosition(p); }
  inline virtual STime          position(void) const                            { return BufferReaderBase::position(); }
  inline virtual QList<Chapter> chapters(void) const                            { return BufferReaderBase::chapters(); }

  inline virtual QList<AudioStreamInfo> audioStreams(void) const                { return BufferReaderBase::audioStreams(); }
  inline virtual QList<VideoStreamInfo> videoStreams(void) const                { return BufferReaderBase::videoStreams(); }
  inline virtual QList<DataStreamInfo> dataStreams(void) const                  { return BufferReaderBase::dataStreams(); }
  inline virtual void           selectStreams(const QVector<StreamId> &s)       { return BufferReaderBase::selectStreams(s); }

  inline virtual bool           process(void)                                   { return BufferReaderBase::demux(BufferReaderBase::read()); }

public: // From SInterfaces::BufferReader
  virtual bool                  openFormat(const QString &);

  virtual bool                  start(ReadCallback *, ProduceCallback *, quint16 programId, bool streamed);
  virtual void                  stop(void);

private:
  static int                    read(void *opaque, uint8_t *buf, int buf_size);
  static int64_t                seek(void *opaque, int64_t offset, int whence);

private:
  ReadCallback                * readCallback;
  ::AVInputFormat             * format;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  ::AVIOContext               * ioContext;
#else
  ::ByteIOContext             * ioContext;
#endif
};

} } // End of namespaces

#endif
