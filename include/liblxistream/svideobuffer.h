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

#ifndef LXISTREAM_SVIDEOBUFFER_H
#define LXISTREAM_SVIDEOBUFFER_H

#include <QtGui/QImage>
#include <LXiCore>
#include "svideoformat.h"
#include "sbuffer.h"
#include "ssize.h"
#include "stime.h"
#include "export.h"

namespace LXiStream {

/*! This class represents a buffer containing raw video pixels.
 */
class LXISTREAM_PUBLIC SVideoBuffer : public SBuffer
{
public:
                                SVideoBuffer(void);
  explicit                      SVideoBuffer(const SVideoFormat &);
                                SVideoBuffer(const SVideoFormat &, const MemoryPtr &, const int *offset, const int *lineSize);

  inline const SVideoFormat   & format(void) const                              { return d.format; }
  void                          setFormat(const SVideoFormat &);
  void                          overrideFormat(const SVideoFormat &);

  inline STime                  timeStamp(void) const                           { return d.timeStamp; }
  inline void                   setTimeStamp(STime t)                           { d.timeStamp = t; }

  const char                  * scanLine(int y, int plane) const;
  char                        * scanLine(int y, int plane);

  inline int                    lineSize(int plane) const                       { Q_ASSERT(plane < d.maxPlanes); return d.lineSize[plane]; }
  inline void                   setLineSize(int plane, int s)                   { Q_ASSERT(plane < d.maxPlanes); d.lineSize[plane] = s; }
  inline size_t                 offset(int plane) const                         { Q_ASSERT(plane < d.maxPlanes); return d.offset[plane]; }
  inline void                   setOffset(int plane, size_t o)                  { Q_ASSERT(plane < d.maxPlanes); d.offset[plane] = o; }

private:
  struct
  {
    SVideoFormat                format;
    STime                       timeStamp;

    static const int            maxPlanes = 4;
    int                         offset[maxPlanes];
    int                         lineSize[maxPlanes];
  }                             d;
};

typedef QList<SVideoBuffer>     SVideoBufferList;

} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SVideoBuffer)

#endif
