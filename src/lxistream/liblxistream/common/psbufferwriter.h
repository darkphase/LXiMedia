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

#ifndef LXSTREAMCOMMON_PSBUFFERWRITER_H
#define LXSTREAMCOMMON_PSBUFFERWRITER_H

#include <QtCore>
#include <LXiStream>
#include "mpeg.h"

namespace LXiStream {
namespace Common {

/*
class PsBufferWriter : public SInterfaces::BufferWriter
{
Q_OBJECT
public:
  struct BufferHeader
  {
    static const quint16        SyncWord;

    quint16                     syncWord;
    quint16                     typeId;
    quint16                     subStreamId;
    quint32                     dataSize;
    STime                       timeStamp;
    STime                       decodingTimeStamp;
    STime                       presentationTimeStamp;
  } __attribute__((packed));

  struct VideoBufferHeader : BufferHeader
  {
    quint8                      keyFrame;
    quint8                      __reserved1;
    quint16                     numChannels;
    quint16                     lineSize[SVideoBuffer::maxChannels];
    quint32                     offset[SVideoBuffer::maxChannels];
  } __attribute__((packed));

  static const char     * const formatName;
  static const quint16          CodecMapSyncWord;

public:
  explicit                      PsBufferWriter(const QString &, QObject *);
  virtual                       ~PsBufferWriter();

  static QByteArray             buildHeader(void);

public: // From SInterfaces::BufferWriter
  virtual bool                  openFormat(const QString &);

  virtual bool                  start(Callback *);
  virtual void                  stop(void);
  virtual void                  process(const SAudioBuffer &);
  virtual void                  process(const SVideoBuffer &);
  virtual void                  process(const SDataBuffer &);

public:
  void                          toPESPackets(QList<MPEG::PESPacket> &, const SBuffer &);
  static quint8                 getMPEGStreamID(const SBuffer &);
  static void                   fillPSMap(MPEG::PSMap &psMap, const SBuffer &);

private:
  void                          writeBuffer(const SMemoryBuffer &buffer);
  static void                   setTimeStamp(MPEG::PESPacket *, qint64 pts, qint64 dts);

private:
  Callback                    * callback;
  MPEG::PSMap                   psMap;
  MPEG::PESPacket               codecsPacket;
  quint16                       codecsLen;
  quint8                        lastTypeId;
  STime                         lastPackHeaderTimeStamp;
  STime                         lastPSMapTimeStamp;
  STime                         lastAudioTimeStamp;
  quint16                       sid;
  quint8                        ccounter;
};
*/

} } // End of namespaces

#endif
