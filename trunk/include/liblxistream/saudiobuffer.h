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

#ifndef LXSTREAM_SAUDIOBUFFER_H
#define LXSTREAM_SAUDIOBUFFER_H

#include <QtCore>
#include <LXiCore>
#include "saudioformat.h"
#include "sbuffer.h"
#include "stime.h"
#include "export.h"

namespace LXiStream {

class SAudioBuffer;
typedef QList<SAudioBuffer>     SAudioBufferList;

/*! This class represents a buffer containing raw audio samples.
 */
class LXISTREAM_PUBLIC SAudioBuffer : public SBuffer
{
public:
  inline                        SAudioBuffer(void) : SBuffer()            { }
  explicit                      SAudioBuffer(const SAudioFormat &, int numSamples = 0);
                                SAudioBuffer(const SAudioFormat &, const MemoryPtr &);
                                SAudioBuffer(const SAudioBufferList &);

  SAudioBuffer                & operator=(const SAudioBufferList &);

  inline const SAudioFormat   & format(void) const                              { return d.format; }
  void                          setFormat(const SAudioFormat &format);

  inline STime                  timeStamp(void) const                           { return d.timeStamp; }
  inline void                   setTimeStamp(STime t)                           { d.timeStamp = t; }

  int                           numSamples(void) const;
  void                          setNumSamples(int s);
  STime                         duration(void) const;

private:
  struct
  {
    SAudioFormat                format;
    STime                       timeStamp;
  }                             d;
};


} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SAudioBuffer)

#endif
