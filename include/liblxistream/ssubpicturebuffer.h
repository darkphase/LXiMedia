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

#ifndef LXISTREAM_SSUBPICTUREBUFFER_H
#define LXISTREAM_SSUBPICTUREBUFFER_H

#include <QtCore>
#include <LXiCore>
#include "sbuffer.h"
#include "stime.h"
#include "export.h"

namespace LXiStream {

class SBufferDeserializerNode;
class SBufferSerializerNode;

/*! This class represents a buffer containing subpicture data.
 */
class LXISTREAM_PUBLIC SSubpictureBuffer : public SBuffer
{
friend class SBufferDeserializerNode;
friend class SBufferSerializerNode;
public:
  struct Rect
  {
    qint16                      x, y;
    quint16                     width, height;
    quint16                     lineStride;
    quint16                     paletteSize;
  };

public:
                                SSubpictureBuffer(void);
  explicit                      SSubpictureBuffer(const QList<Rect> &);

  inline STime                  timeStamp(void) const                           { return d.timeStamp; }
  inline void                   setTimeStamp(STime t)                           { d.timeStamp = t; }
  inline STime                  duration(void) const                            { return d.duration; }
  inline void                   setDuration(STime t)                            { d.duration = t; }

  void                          setRects(const QList<Rect> &);
  QList<Rect>                   rects(void) const;

  const quint32               * palette(int rectId) const;
  quint32                     * palette(int rectId);
  const quint8                * lines(int rectId) const;
  quint8                      * lines(int rectId);

private:
  static int                    rectSize(const Rect &rect);

private:
  // Ensure all these struct members are serializable.
  struct
  {
    STime                       timeStamp;
    STime                       duration;
  }                             d;
};

typedef QList<SSubpictureBuffer> SSubpictureBufferList;

} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SSubpictureBuffer)

#endif
