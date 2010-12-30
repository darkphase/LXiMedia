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

#ifndef __WINMMAUDIOOUTPUT_H
#define __WINMMAUDIOOUTPUT_H

#include <windows.h>
#include <mmsystem.h>
#include <QtCore>
#include <LXiStream>
#include <liblxistream/common/audiooutput.h>

namespace LXiStream {
namespace WinMMBackend {

class WinMMAudioOutputNode;

class WinMMAudioOutput : public Common::AudioOutput
{
Q_OBJECT
friend class WinMMAudioOutputNode;
public:
                                WinMMAudioOutput(int, bool, QObject *);
  virtual                       ~WinMMAudioOutput();

  virtual Common::AudioMixerNode * createNode(void);

public: // From SNode
  virtual STime                 latency(void) const;

  virtual bool                  prepare(const SCodecList &);
  virtual bool                  unprepare(void);

protected:
  virtual void                  writeAudio(const SAudioBuffer &);

private:
  bool                          openCodec(const SAudioCodec &);

  void                          writeHeader(const qint16 *, size_t, const SAudioBuffer &);
  void                          flushHeaders(void);

public:
  int                           nodeCount;

private:
  static const unsigned         maxDelay = 250; // ms

  STime                         outLatency;
  SAudioBuffer                  silentBuffer;
  SAudioBuffer                  lastBuffer;
  unsigned                      lastBufferRepeats;

  SCodec                        outCodec;
  const int                     dev;
  HWAVEOUT                      waveOut;
  bool                          errorProduced;
  QQueue< QPair<WAVEHDR *, SAudioBuffer> > headers;
};


class WinMMAudioOutputNode : public Common::AudioOutputNode
{
Q_OBJECT
public:
                                WinMMAudioOutputNode(WinMMAudioOutput *);
  virtual                       ~WinMMAudioOutputNode();

private:
  WinMMAudioOutput      * const parent;
};


} } // End of namespaces

#endif
