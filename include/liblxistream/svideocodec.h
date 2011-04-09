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
class LXISTREAM_PUBLIC SVideoCodec : public SSerializable
{
public:
                                SVideoCodec(void);
                                SVideoCodec(const char *, SSize = SSize(), SInterval = SInterval(), int bitRate = 0);
                                SVideoCodec(const QString &, SSize = SSize(), SInterval = SInterval(), int bitRate = 0);

  inline                        operator const QString &() const                { return codec(); }

  bool                          operator==(const SVideoCodec &other) const;
  inline bool                   operator!=(const SVideoCodec &other) const      { return !operator==(other); }
  inline bool                   operator==(const char *other) const             { return d.codec == other; }
  inline bool                   operator!=(const char *other) const             { return !operator==(other); }
  inline bool                   operator==(const QString &other) const          { return d.codec == other; }
  inline bool                   operator!=(const QString &other) const          { return !operator==(other); }

  inline bool                   isNull(void) const                              { return d.codec.isEmpty(); }
  inline const QString        & codec(void) const                               { return d.codec; }
  void                          setCodec(const QString &codec, SSize = SSize(), SInterval = SInterval(), int bitRate = 0);

  inline SSize                  size(void) const                                { return d.size; }
  inline void                   setSize(SSize s)                                { d.size = s; }
  inline SInterval              frameRate(void) const                           { return d.frameRate; }
  inline void                   setFrameRate(SInterval r)                       { d.frameRate = r; }
  inline int                    bitRate(void) const                             { return d.bitRate; }
  inline void                   setBitRate(int b)                               { d.bitRate = b; }

  inline const QByteArray     & extraData(void) const                           { return d.extraData; }
  inline void                   setExtraData(const QByteArray &data)            { d.extraData = data; }

public: // From SSerializable
  virtual QDomNode              toXml(QDomDocument &) const;
  virtual void                  fromXml(const QDomNode &);

private:
  struct
  {
    QString                     codec;
    SSize                       size;
    SInterval                   frameRate;
    int                         bitRate;

    QByteArray                  extraData;
  }                             d;
};

} // End of namespace

#endif
