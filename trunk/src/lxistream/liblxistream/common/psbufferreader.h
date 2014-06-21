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
  virtual void                  selectStreams(const QVector<StreamId> &);

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
