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

#ifndef __DATADECODER_H
#define __DATADECODER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {


class DataDecoder : public SInterfaces::DataDecoder
{
Q_OBJECT
public:
                                DataDecoder(const QString &, QObject *);
  virtual                       ~DataDecoder();

public: // From SBufferDecoder
  virtual bool                  openCodec(const SDataCodec &, SInterfaces::AbstractBufferReader *, Flags = Flag_None);
  virtual SDataBufferList       decodeBuffer(const SEncodedDataBuffer &);

private:
  SDataCodec                    inCodec;
  ::AVDictionary              * codecDict;
  ::AVCodec                   * codecHandle;
  ::AVCodecContext            * contextHandle;
  bool                          contextHandleOwner;
};


} } // End of namespaces

#endif
