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

#ifndef LXSTREAM_SAUDIOCODEC_H
#define LXSTREAM_SAUDIOCODEC_H

#include <QtCore>
#include <LXiCore>
#include "saudioformat.h"
#include "export.h"

namespace LXiStream {

/*! This class represents an audio codec.

    \sa SVideoCodec, SDataCodec
 */
class LXISTREAM_PUBLIC SAudioCodec
{
public:
                                SAudioCodec(void);
                                SAudioCodec(const char *, SAudioFormat::Channels = SAudioFormat::Channel_None, int sampleRate = 0, int streamId = -1, int bitRate = 0, int frameSize = 0);
                                SAudioCodec(const QByteArray &, SAudioFormat::Channels = SAudioFormat::Channel_None, int sampleRate = 0, int streamId = -1, int bitRate = 0, int frameSize = 0);

  inline                        operator const QByteArray &() const             { return name(); }

  bool                          operator==(const SAudioCodec &other) const;
  inline bool                   operator!=(const SAudioCodec &other) const      { return !operator==(other); }
  inline bool                   operator==(const char *other) const             { return d.name == other; }
  inline bool                   operator!=(const char *other) const             { return !operator==(other); }
  inline bool                   operator==(const QByteArray &other) const       { return d.name == other; }
  inline bool                   operator!=(const QByteArray &other) const       { return !operator==(other); }

  inline bool                   isNull(void) const                              { return d.name.isEmpty(); }
  inline const QByteArray     & name(void) const                                { return d.name; }
  void                          setCodec(const QByteArray &, SAudioFormat::Channels = SAudioFormat::Channel_None, int sampleRate = 0, int streamId = -1, int bitRate = 0, int frameSize = 0);

  inline SAudioFormat::Channels channelSetup(void) const                        { return d.channels; }
  inline void                   setChannelSetup(SAudioFormat::Channels c)       { d.channels = c; }
  inline int                    sampleRate(void) const                          { return d.sampleRate; }
  inline void                   setSampleRate(int s)                            { d.sampleRate = s; }
  inline int                    streamId(void) const                            { return d.streamId; }
  inline void                   setStreamId(int i)                              { d.streamId = i; }
  inline int                    bitRate(void) const                             { return d.bitRate; }
  inline void                   setBitRate(int b)                               { d.bitRate = b; }
  inline int                    frameSize(void) const                           { return d.frameSize; }
  inline void                   setFrameSize(int f)                             { d.frameSize = f; }

  inline int                    numChannels(void) const                         { return SAudioFormat::numChannels(channelSetup()); }

  void                          serialize(QXmlStreamWriter &) const;            //!< Serializes all information to the XML stream.
  bool                          deserialize(QXmlStreamReader &);                //!< Deserializes all information from the XML stream, make sure readNextStartElement() is invoked on the reader.

private:
  struct
  {
    QByteArray                  name;
    SAudioFormat::Channels      channels;
    int                         sampleRate;
    int                         streamId;
    int                         bitRate;
    int                         frameSize;
  }                             d;
};

} // End of namespace

#endif
