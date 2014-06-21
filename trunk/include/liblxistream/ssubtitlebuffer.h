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

#ifndef LXISTREAM_SSUBTITLEBUFFER_H
#define LXISTREAM_SSUBTITLEBUFFER_H

#include <QtCore>
#include <LXiCore>
#include "sbuffer.h"
#include "stime.h"
#include "export.h"

namespace LXiStream {

class SBufferDeserializerNode;
class SBufferSerializerNode;

/*! This class represents a buffer containing subtitle data.
 */
class LXISTREAM_PUBLIC SSubtitleBuffer : public SBuffer
{
friend class SBufferDeserializerNode;
friend class SBufferSerializerNode;
public:
  inline                        SSubtitleBuffer(void) : SBuffer()               { }
  inline                        SSubtitleBuffer(const QStringList &s) : SBuffer() { setSubtitle(s); }

  inline STime                  timeStamp(void) const                           { return d.timeStamp; }
  inline void                   setTimeStamp(STime t)                           { d.timeStamp = t; }
  inline STime                  duration(void) const                            { return d.duration; }
  inline void                   setDuration(STime t)                            { d.duration = t; }

  inline QStringList            subtitle(void) const                            { return QString::fromUtf8(data(), size()).split('\n'); }
  inline void                   setSubtitle(const QStringList &s)               { setData(s.join("\n").toUtf8()); }

private:
  // Ensure all these struct members are serializable.
  struct
  {
    STime                       timeStamp;
    STime                       duration;
  }                             d;
};

typedef QList<SSubtitleBuffer>  SSubtitleBufferList;

} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SSubtitleBuffer)

#endif
