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

#ifndef __AUDIORESAMPLER_H
#define __AUDIORESAMPLER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {


/*! This audio filter can be used to resample an audio stream.
 */
class AudioResampler : public SInterfaces::AudioResampler
{
Q_OBJECT
private:
  struct Channel
  {
    inline Channel(void) : channel(SAudioFormat::Channel_None), bufferOffset(0) { }
    inline explicit Channel(SAudioFormat::Channel channel, ::AVResampleContext *context = NULL)
      : channel(channel), context(context), bufferOffset(0)
    {
    }

    SAudioFormat::Channel       channel;
    ::AVResampleContext       * context;
    SAudioBuffer                channelBuffer;
    unsigned                    bufferOffset;
  };

public:
  explicit                      AudioResampler(const QString &, QObject *);
  virtual                       ~AudioResampler();

public: // From SInterfaces::AudioResampler
  virtual void                  setSampleRate(unsigned);
  virtual unsigned              sampleRate(void);

  virtual SAudioBuffer          processBuffer(const SAudioBuffer &);
  virtual void                  compensate(float);

private:
  SAudioBuffer                  resampleChannel(Channel *, const SAudioBuffer &) const;

private:
  unsigned                      outSampleRate;
  QVector<Channel>              channels;
  SAudioFormat                  inFormat;
  volatile bool                 reopen;
};


} } // End of namespaces

#endif