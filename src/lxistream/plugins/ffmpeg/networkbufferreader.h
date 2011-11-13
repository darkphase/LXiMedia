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

#ifndef __NETWORKBUFFERREADER_H
#define __NETWORKBUFFERREADER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"
#include "bufferreaderbase.h"

namespace LXiStream {
namespace FFMpegBackend {

class NetworkBufferReader : public SInterfaces::NetworkBufferReader,
                            public BufferReaderBase
{
Q_OBJECT
public:
  explicit                      NetworkBufferReader(const QString &, QObject *);
  virtual                       ~NetworkBufferReader();

public: // From SInterfaces::AbstractBufferReader
  inline virtual STime          duration(void) const                            { return BufferReaderBase::duration(); }
  inline virtual bool           setPosition(STime p)                            { return BufferReaderBase::setPosition(p); }
  inline virtual STime          position(void) const                            { return BufferReaderBase::position(); }
  inline virtual QList<Chapter> chapters(void) const                            { return BufferReaderBase::chapters(); }

  inline virtual QList<AudioStreamInfo> audioStreams(void) const                { return BufferReaderBase::audioStreams(); }
  inline virtual QList<VideoStreamInfo> videoStreams(void) const                { return BufferReaderBase::videoStreams(); }
  inline virtual QList<DataStreamInfo> dataStreams(void) const                  { return BufferReaderBase::dataStreams(); }
  inline virtual void           selectStreams(const QVector<StreamId> &s)       { return BufferReaderBase::selectStreams(s); }

  virtual bool                  process(void);

public: // From SInterfaces::NetworkBufferReader
  virtual bool                  openProtocol(const QString &);

  virtual bool                  start(const QUrl &url, ProduceCallback *, quint16 programId);
  virtual void                  stop(void);

  inline virtual bool           buffer(void)                                    { return BufferReaderBase::buffer(); }
  inline virtual STime          bufferDuration(void) const                      { return BufferReaderBase::bufferDuration(); }

private:
  QList<QUrl>                   resolveAsf(const QUrl &);
};

} } // End of namespaces

#endif
