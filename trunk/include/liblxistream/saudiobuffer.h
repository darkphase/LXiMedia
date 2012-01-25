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

#ifndef LXSTREAM_SAUDIOBUFFER_H
#define LXSTREAM_SAUDIOBUFFER_H

#include <QtCore>
#include <LXiCore>
#include "saudioformat.h"
#include "sbuffer.h"
#include "stime.h"
#include "export.h"

namespace LXiStream {

class SBufferDeserializerNode;
class SBufferSerializerNode;

class SAudioBuffer;
typedef QList<SAudioBuffer>     SAudioBufferList;

/*! This class represents a buffer containing raw audio samples.
 */
class LXISTREAM_PUBLIC SAudioBuffer : public SBuffer
{
friend class SBufferDeserializerNode;
friend class SBufferSerializerNode;
public:
  inline                        SAudioBuffer(void) : SBuffer()            { }
  explicit                      SAudioBuffer(const SAudioFormat &, unsigned numSamples = 0);
                                SAudioBuffer(const SAudioFormat &, const MemoryPtr &);
                                SAudioBuffer(const SAudioBufferList &);

  SAudioBuffer                & operator=(const SAudioBufferList &);

  inline const SAudioFormat   & format(void) const                              { return d.format; }
  void                          setFormat(const SAudioFormat &format);

  inline STime                  timeStamp(void) const                           { return d.timeStamp; }
  inline void                   setTimeStamp(STime t)                           { d.timeStamp = t; }

  unsigned                      numSamples(void) const;
  void                          setNumSamples(unsigned s);
  STime                         duration(void) const;

  SAudioBuffer                  getChannel(SAudioFormat::Channel) const;
  void                          setChannel(const SAudioBuffer &, SAudioFormat::Channel);
  void                          setChannels(const SAudioBuffer &);

private:
  // Ensure all these struct members are serializable.
  struct
  {
    SAudioFormat                format;
    STime                       timeStamp;
  }                             d;
};


} // End of namespace

Q_DECLARE_METATYPE(LXiStream::SAudioBuffer)

#endif
