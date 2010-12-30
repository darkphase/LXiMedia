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

#ifndef LXSTREAMCOMMON_PSBUFFERREADER_H
#define LXSTREAMCOMMON_PSBUFFERREADER_H

#include <QtCore>
#include <LXiStream>
#include "mpeg.h"

namespace LXiStream {
namespace Common {

/*
class PsBufferReader : public SInterfaces::BufferReader,
                       private MPEG::Stream
{
Q_OBJECT
public:
  explicit                      PsBufferReader(const QString &, QObject *);
  virtual                       ~PsBufferReader();

public: // From SInterfaces::BufferReader
  virtual bool                  openFormat(const QString &);

  virtual bool                  start(Callback *, bool streamed);
  virtual void                  stop(void);
  virtual void                  process(void);

  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);

  virtual QList<AudioStreamInfo> audioStreams(void) const;
  virtual QList<VideoStreamInfo> videoStreams(void) const;
  virtual QList<DataStreamInfo>  dataStreams(void) const;
  virtual void                  selectStreams(const QList<quint16> &);

private:
  void                          processPESPacket(const MPEG::PESPacket *, size_t);

private:
  Callback                    * callback;
  MPEG::PESPacketStream         pesPacketStream;
  QMap<SBuffer::StreamId, SCodec> codecs;
  SMemoryBuffer               * buffer;
  SAudioBuffer                  audioBuffer;
  SVideoBuffer                  videoBuffer;
  SDataBuffer                   dataBuffer;
  size_t                        expectedSize;
  size_t                        readOffset;
};
*/

} } // End of namespaces

#endif
