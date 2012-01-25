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

#ifndef LINUXDVBBACKEND_DVBINPUT_H
#define LINUXDVBBACKEND_DVBINPUT_H

#include <QtCore>
#include <LXiStream>
#include "dvbdevice.h"

namespace LXiStream {
namespace LinuxDvbBackend {


class DVBInput : public SNode,
                 private Common::MPEG::Stream
{
Q_OBJECT
public:
  static QStringList            schemes(void);

public:
                                DVBInput(DVBDevice *, unsigned, quint16);
  virtual                       ~DVBInput();

public: // From SNode
  virtual bool                  prepare(const SCodecList &);
  virtual bool                  unprepare(void);
  virtual Result                processBuffer(const SBuffer &, SBufferList &);

private:
  void                          findProgram(void);

  void                          audioPacketReceived(const Common::MPEG::PESPacket *, size_t);
  void                          audioAFieldReceived(const Common::MPEG::TSPacket::AdaptationField *);
  void                          videoPacketReceived(const Common::MPEG::PESPacket *, size_t);
  void                          videoAFieldReceived(const Common::MPEG::TSPacket::AdaptationField *);
  void                          teletextPacketReceived(const Common::MPEG::PESPacket *, size_t);
  void                          teletextAFieldReceived(const Common::MPEG::TSPacket::AdaptationField *);

private:
  DVBDevice             * const parent;
  const unsigned                streamID;
  const quint16                 serviceID;
  DVBDevice::Program            program;

  int                           audioDesc;
  int                           videoDesc;
  int                           teletextDesc;

  const dmx_pes_type_t          audioPES;
  const dmx_pes_type_t          videoPES;
  const dmx_pes_type_t          teletextPES;

  SAudioCodec                   audioCodec;
  SVideoCodec                   videoCodec;

  STimer                        timer;
  SBufferQueue                  buffers;
  SBuffer                       teletextMagazines[8];

  Common::MPEG::PESPacketStream pesAudioPacketStream;
  Common::MPEG::PESPacketStream pesVideoPacketStream;
  Common::MPEG::PESPacketStream pesTeletextPacketStream;
  bool                          videoKeyFrame;
};


} } // End of namespaces

#endif
