/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
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
                                SVideoCodec(const QString &, SSize = SSize(), SInterval = SInterval(), int streamId = -1, int bitRate = 0, int gopSize = -1);

  inline                        operator const QString &() const                { return codec(); }

  bool                          operator==(const SVideoCodec &other) const;
  inline bool                   operator!=(const SVideoCodec &other) const      { return !operator==(other); }
  inline bool                   operator==(const char *other) const             { return d.codec == other; }
  inline bool                   operator!=(const char *other) const             { return !operator==(other); }
  inline bool                   operator==(const QString &other) const          { return d.codec == other; }
  inline bool                   operator!=(const QString &other) const          { return !operator==(other); }

  inline bool                   isNull(void) const                              { return d.codec.isEmpty(); }
  inline const QString        & codec(void) const                               { return d.codec; }
  void                          setCodec(const QString &codec, SSize = SSize(), SInterval = SInterval(), int streamId = -1, int bitRate = 0, int gopSize = -1);

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

  QString                       toString(void) const;
  static SVideoCodec            fromString(const QString &);

private:
  struct
  {
    QString                     codec;
    SSize                       size;
    SInterval                   frameRate;
    int                         streamId;
    int                         bitRate;
    int                         gopSize;
  }                             d;
};

} // End of namespace

#endif
