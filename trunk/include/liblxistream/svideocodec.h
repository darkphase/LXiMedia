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

#ifndef LXISTREAM_SVIDEOCODEC_H
#define LXISTREAM_SVIDEOCODEC_H

#include <QtCore>
#include <LXiCore>
#include "sinterval.h"
#include "ssize.h"
#include "export.h"

namespace LXiStream {

/*! This class represents a video codec.

    \sa SAudioCodec, SDataCodec
 */
class LXISTREAM_PUBLIC SVideoCodec
{
public:
                                SVideoCodec(void);
                                SVideoCodec(const char *, SSize = SSize(), SInterval = SInterval(), int streamId = -1, int bitRate = 0, int gopSize = -1);
                                SVideoCodec(const QByteArray &, SSize = SSize(), SInterval = SInterval(), int streamId = -1, int bitRate = 0, int gopSize = -1);

  inline                        operator const QByteArray &() const             { return name(); }

  bool                          operator==(const SVideoCodec &other) const;
  inline bool                   operator!=(const SVideoCodec &other) const      { return !operator==(other); }
  inline bool                   operator==(const char *other) const             { return d.name == other; }
  inline bool                   operator!=(const char *other) const             { return !operator==(other); }
  inline bool                   operator==(const QByteArray &other) const       { return d.name == other; }
  inline bool                   operator!=(const QByteArray &other) const       { return !operator==(other); }

  inline bool                   isNull(void) const                              { return d.name.isEmpty(); }
  inline const QByteArray     & name(void) const                                { return d.name; }
  void                          setCodec(const QByteArray &codec, SSize = SSize(), SInterval = SInterval(), int streamId = -1, int bitRate = 0, int gopSize = -1);

  inline SSize                  size(void) const                                { return d.size; }
  inline void                   setSize(SSize s)                                { d.size = s; }
  inline SInterval              frameRate(void) const                           { return d.frameRate; }
  inline void                   setFrameRate(SInterval r)                       { d.frameRate = r; }
  inline int                    streamId(void) const                            { return d.streamId; }
  inline void                   setStreamId(int i)                              { d.streamId = i; }
  inline int                    bitRate(void) const                             { return d.bitRate; }
  inline void                   setBitRate(int b)                               { d.bitRate = b; }
  inline int                    gopSize(void) const                             { return d.gopSize; }
  inline void                   setGopSize(int g)                               { d.gopSize = g; }

  void                          serialize(QXmlStreamWriter &) const;            //!< Serializes all information to the XML stream.
  bool                          deserialize(QXmlStreamReader &);                //!< Deserializes all information from the XML stream, make sure readNextStartElement() is invoked on the reader.

private:
  struct
  {
    QByteArray                  name;
    SSize                       size;
    SInterval                   frameRate;
    int                         streamId;
    int                         bitRate;
    int                         gopSize;
  }                             d;
};

} // End of namespace

#endif
