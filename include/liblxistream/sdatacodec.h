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
                                SDataCodec(const char *name, const QByteArray &codepage = QByteArray(), int streamId = -1);
                                SDataCodec(const QByteArray &name, const QByteArray &codepage = QByteArray(), int streamId = -1);

  inline                        operator const QByteArray &() const             { return name(); }

  bool                          operator==(const SDataCodec &other) const;
  inline bool                   operator!=(const SDataCodec &other) const       { return !operator==(other); }
  bool                          operator==(const QByteArray &other) const       { return d.name == other; }
  inline bool                   operator!=(const QByteArray &other) const       { return !operator==(other); }

  inline bool                   isNull(void) const                              { return d.name.isEmpty(); }
  inline const QByteArray     & name(void) const                                { return d.name; }
  void                          setCodec(const QByteArray &name, const QByteArray &codepage = QByteArray(), int streamId = -1);

  inline const QByteArray     & codepage(void) const                            { return d.codepage; }
  inline void                   setCodepage(const QByteArray &c)                { d.codepage = c; }
  inline int                    streamId(void) const                            { return d.streamId; }
  inline void                   setStreamId(int i)                              { d.streamId = i; }

  void                          serialize(QXmlStreamWriter &) const;            //!< Serializes all information to the XML stream.
  bool                          deserialize(QXmlStreamReader &);                //!< Deserializes all information from the XML stream, make sure readNextStartElement() is invoked on the reader.

private:
  struct
  {
    QByteArray                  name;
    QByteArray                  codepage;
    int                         streamId;
  }                             d;
};

} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SDataCodec)

#endif
