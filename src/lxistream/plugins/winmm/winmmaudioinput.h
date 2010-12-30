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

#ifndef __WINMMAUDIOINPUT_H
#define __WINMMAUDIOINPUT_H

#include <windows.h>
#include <mmsystem.h>
#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace WinMMBackend {


class WinMMAudioInput : public SNodes::Audio::Source
{
Q_OBJECT
public:
                                WinMMAudioInput(int, QObject *);

  virtual inline bool           finished(void) const                            { return false; }
  virtual inline SAudioCodec::Channels channels(void) const                     { return SAudioCodec::guessChannels(outNumChannels); }
  virtual inline void           setChannels(SAudioCodec::Channels c)            { outNumChannels = SAudioCodec::numChannels(c); }
  virtual inline unsigned       sampleRate(void) const                          { return outSampleRate; }
  virtual inline void           setSampleRate(unsigned s)                       { outSampleRate = s; }

public: // From SNode
  virtual bool                  prepare(const SCodecList &);
  virtual bool                  unprepare(void);
  virtual Result                processBuffer(const SBuffer &, SBufferList &);

private:
  static const unsigned         maxBuffers = 256;

  volatile bool                 running;
  const int                     dev;
  HWAVEOUT                      waveIn;

  STimer                        timer;
  SAudioCodec                   format;
  unsigned                      outSampleSize;
  unsigned                      outSampleRate;
  unsigned                      outNumChannels;

  SBufferQueue                  producedBuffers;
};


} } // End of namespaces

#endif
