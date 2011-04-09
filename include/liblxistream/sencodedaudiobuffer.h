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

#ifndef LXSTREAM_SENCODEDAUDIOBUFFER_H
#define LXSTREAM_SENCODEDAUDIOBUFFER_H

#include <QtCore>
#include <LXiCore>
#include "saudiocodec.h"
#include "sbuffer.h"
#include "stime.h"
#include "export.h"

namespace LXiStream {

/*! This class represents a buffer containing encoded audio data.
 */
class LXISTREAM_PUBLIC SEncodedAudioBuffer : public SBuffer
{
public:
  inline                        SEncodedAudioBuffer(void) : SBuffer()     { }
  inline                        SEncodedAudioBuffer(const SAudioCodec &codec, int capacity = 0) : SBuffer(capacity) { d.codec = codec; }
  inline                        SEncodedAudioBuffer(const SAudioCodec &codec, const char *data, int size) : SBuffer(data, size) { d.codec = codec; }
  inline                        SEncodedAudioBuffer(const SAudioCodec &codec, const QByteArray &data) : SBuffer(data) { d.codec = codec; }
  inline                        SEncodedAudioBuffer(const SAudioCodec &codec, const MemoryPtr &memory) : SBuffer(memory) { d.codec = codec; }

  inline const SAudioCodec    & codec(void) const                               { return d.codec; }
  inline void                   setCodec(const SAudioCodec &codec)              { d.codec = codec; }

  inline STime                  decodingTimeStamp(void) const                   { return d.decodingTimeStamp; }
  inline void                   setDecodingTimeStamp(STime t)                   { d.decodingTimeStamp = t; }
  inline STime                  presentationTimeStamp(void) const               { return d.presentationTimeStamp; }
  inline void                   setPresentationTimeStamp(STime t)               { d.presentationTimeStamp = t; }

private:
  struct
  {
    SAudioCodec                 codec;
    STime                       decodingTimeStamp;
    STime                       presentationTimeStamp;
  }                             d;
};

typedef QList<SEncodedAudioBuffer> SEncodedAudioBufferList;

} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SEncodedAudioBuffer)

#endif
