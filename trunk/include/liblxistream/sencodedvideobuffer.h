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

#ifndef LXSTREAM_SENCODEDVIDEOBUFFER_H
#define LXSTREAM_SENCODEDVIDEOBUFFER_H

#include <QtGui/QImage>
#include <LXiCore>
#include "svideocodec.h"
#include "sbuffer.h"
#include "ssize.h"
#include "stime.h"
#include "export.h"

namespace LXiStream {

/*! This class represents a buffer containing encoded video data.
 */
class LXISTREAM_PUBLIC SEncodedVideoBuffer : public SBuffer
{
public:
  inline                        SEncodedVideoBuffer(void) : SBuffer()     { }
  inline explicit               SEncodedVideoBuffer(const SVideoCodec &codec, int capacity = 0) : SBuffer(capacity) { d.codec = codec; }
  inline                        SEncodedVideoBuffer(const SVideoCodec &codec, const char *data, int size) : SBuffer(data, size) { d.codec = codec; }
  inline                        SEncodedVideoBuffer(const SVideoCodec &codec, const QByteArray &data) : SBuffer(data) { d.codec = codec; }
  inline                        SEncodedVideoBuffer(const SVideoCodec &codec, const MemoryPtr &memory) : SBuffer(memory) { d.codec = codec; }

  inline const SVideoCodec    & codec(void) const                               { return d.codec; }
  inline void                   setCodec(const SVideoCodec &codec)              { d.codec = codec; }

  inline STime                  decodingTimeStamp(void) const                   { return d.decodingTimeStamp; }
  inline void                   setDecodingTimeStamp(STime t)                   { d.decodingTimeStamp = t; }
  inline STime                  presentationTimeStamp(void) const               { return d.presentationTimeStamp; }
  inline void                   setPresentationTimeStamp(STime t)               { d.presentationTimeStamp = t; }

  inline bool                   isKeyFrame(void) const                          { return d.keyFrame; }
  inline void                   setKeyFrame(bool k)                             { d.keyFrame = k; }

private:
  struct
  {
    SVideoCodec                 codec;
    STime                       decodingTimeStamp;
    STime                       presentationTimeStamp;
    bool                        keyFrame;
  }                             d;
};

typedef QList<SEncodedVideoBuffer> SEncodedVideoBufferList;

} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SEncodedVideoBuffer)

#endif
