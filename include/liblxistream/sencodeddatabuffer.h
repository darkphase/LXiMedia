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

#ifndef LXSTREAM_SENCODEDDATABUFFER_H
#define LXSTREAM_SENCODEDDATABUFFER_H

#include <QtCore>
#include <LXiCore>
#include "sdatacodec.h"
#include "sbuffer.h"
#include "stime.h"
#include "export.h"

namespace LXiStream {

/*! This class represents a buffer containing encoded misc data.
 */
class LXISTREAM_PUBLIC SEncodedDataBuffer : public SBuffer
{
public:
  inline                        SEncodedDataBuffer(void) : SBuffer()     { }
  inline explicit               SEncodedDataBuffer(const SDataCodec &codec, int capacity = 0) : SBuffer(capacity) { d.codec = codec; }
  inline                        SEncodedDataBuffer(const SDataCodec &codec, const char *data, int size) : SBuffer(data, size) { d.codec = codec; }
  inline                        SEncodedDataBuffer(const SDataCodec &codec, const QByteArray &data) : SBuffer(data) { d.codec = codec; }
  inline                        SEncodedDataBuffer(const SDataCodec &codec, const MemoryPtr &memory) : SBuffer(memory) { d.codec = codec; }

  inline const SDataCodec     & codec(void) const                               { return d.codec; }
  inline void                   setCodec(const SDataCodec &codec)               { d.codec = codec; }

  inline STime                  decodingTimeStamp(void) const                   { return d.decodingTimeStamp; }
  inline void                   setDecodingTimeStamp(STime t)                   { d.decodingTimeStamp = t; }
  inline STime                  presentationTimeStamp(void) const               { return d.presentationTimeStamp; }
  inline void                   setPresentationTimeStamp(STime t)               { d.presentationTimeStamp = t; }
  inline STime                  duration(void) const                            { return d.duration; }
  inline void                   setDuration(STime t)                            { d.duration = t; }

private:
  struct
  {
    SDataCodec                  codec;
    STime                       decodingTimeStamp;
    STime                       presentationTimeStamp;
    STime                       duration;
  }                             d;
};

typedef QList<SEncodedDataBuffer> SEncodedDataBufferList;

} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SEncodedDataBuffer)

#endif
