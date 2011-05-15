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
                                SAudioCodec(const char *, SAudioFormat::Channels = SAudioFormat::Channel_None, int sampleRate = 0, int bitRate = 0, int frameSize = 0);
                                SAudioCodec(const QString &, SAudioFormat::Channels = SAudioFormat::Channel_None, int sampleRate = 0, int bitRate = 0, int frameSize = 0);

  inline                        operator const QString &() const                { return codec(); }

  bool                          operator==(const SAudioCodec &other) const;
  inline bool                   operator!=(const SAudioCodec &other) const      { return !operator==(other); }
  inline bool                   operator==(const char *other) const             { return d.codec == other; }
  inline bool                   operator!=(const char *other) const             { return !operator==(other); }
  inline bool                   operator==(const QString &other) const          { return d.codec == other; }
  inline bool                   operator!=(const QString &other) const          { return !operator==(other); }

  inline bool                   isNull(void) const                              { return d.codec.isEmpty(); }
  inline const QString        & codec(void) const                               { return d.codec; }
  void                          setCodec(const QString &, SAudioFormat::Channels = SAudioFormat::Channel_None, int sampleRate = 0, int bitRate = 0, int frameSize = 0);

  inline SAudioFormat::Channels channelSetup(void) const                        { return d.channels; }
  inline void                   setChannelSetup(SAudioFormat::Channels c)       { d.channels = c; }
  inline int                    sampleRate(void) const                          { return d.sampleRate; }
  inline void                   setSampleRate(int s)                            { d.sampleRate = s; }
  inline int                    bitRate(void) const                             { return d.bitRate; }
  inline void                   setBitRate(int b)                               { d.bitRate = b; }
  inline int                    frameSize(void) const                           { return d.frameSize; }
  inline void                   setFrameSize(int f)                             { d.frameSize = f; }

  inline int                    numChannels(void) const                         { return SAudioFormat::numChannels(channelSetup()); }

  inline const QByteArray     & extraData(void) const                           { return d.extraData; }
  inline void                   setExtraData(const QByteArray &data)            { d.extraData = data; }

  QString                       toString(bool addExtraData = true) const;
  static SAudioCodec            fromString(const QString &);

private:
  struct
  {
    QString                     codec;
    SAudioFormat::Channels      channels;
    int                         sampleRate;
    int                         bitRate;
    int                         frameSize;

    QByteArray                  extraData;
  }                             d;
};

} // End of namespace

#endif
