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

  bool                          start(QIODevice *, ProduceCallback *, bool streamed, bool fast);

public: // From SInterfaces::AbstractBufferReader
  inline virtual STime          duration(void) const                            { return BufferReaderBase::duration(); }
  inline virtual bool           setPosition(STime p)                            { return BufferReaderBase::setPosition(p); }
  inline virtual STime          position(void) const                            { return BufferReaderBase::position(); }
  inline virtual QList<Chapter> chapters(void) const                            { return BufferReaderBase::chapters(); }

  inline virtual int            numTitles(void) const                           { return 1; }
  inline virtual QList<AudioStreamInfo> audioStreams(int) const                 { return BufferReaderBase::audioStreams(); }
  inline virtual QList<VideoStreamInfo> videoStreams(int) const                 { return BufferReaderBase::videoStreams(); }
  inline virtual QList<DataStreamInfo> dataStreams(int) const                   { return BufferReaderBase::dataStreams(); }
  inline virtual void           selectStreams(int, const QVector<StreamId> &s)  { return BufferReaderBase::selectStreams(s); }

  inline virtual bool           process(void)                                   { return BufferReaderBase::demux(BufferReaderBase::read()); }

public: // From SInterfaces::BufferReader
  virtual bool                  openFormat(const QString &);

  inline virtual bool           start(QIODevice *rc, ProduceCallback *pc, bool streamed) { return start(rc, pc, streamed, false); }
  virtual void                  stop(void);

private:
  static int                    read(void *opaque, uint8_t *buf, int buf_size);
  static int64_t                seek(void *opaque, int64_t offset, int whence);

private:
  QIODevice                   * ioDevice;
  ::AVInputFormat             * format;
  ::AVIOContext               * ioContext;
};

} } // End of namespaces

#endif
