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

#ifndef LXSTREAM_SDATACODEC_H
#define LXSTREAM_SDATACODEC_H

#include <QtCore>
#include <LXiCore>
#include "export.h"

namespace LXiStream {

/*! This class represents a data codec.

    \sa SAudioCodec, SVideoCodec
 */
class LXISTREAM_PUBLIC SDataCodec
{
public:
                                SDataCodec(void);
                                SDataCodec(const QString &name, const QByteArray &codepage = QByteArray(), int streamId = -1);

  inline                        operator const QString &() const                { return codec(); }

  bool                          operator==(const SDataCodec &other) const;
  inline bool                   operator!=(const SDataCodec &other) const       { return !operator==(other); }
  bool                          operator==(const QString &other) const          { return d.codec == other; }
  inline bool                   operator!=(const QString &other) const          { return !operator==(other); }

  inline bool                   isNull(void) const                              { return d.codec.isEmpty(); }
  inline const QString        & codec(void) const                               { return d.codec; }
  void                          setCodec(const QString &codec, const QByteArray &codepage = QByteArray(), int streamId = -1);

  inline const QByteArray     & codepage(void) const                            { return d.codepage; }
  inline void                   setCodepage(const QByteArray &c)                { d.codepage = c; }
  inline int                    streamId(void) const                            { return d.streamId; }
  inline void                   setStreamId(int i)                              { d.streamId = i; }

  QString                       toString(void) const;
  static SDataCodec             fromString(const QString &);

private:
  struct
  {
    QString                     codec;
    QByteArray                  codepage;
    int                         streamId;
  }                             d;
};

} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SDataCodec)

#endif
