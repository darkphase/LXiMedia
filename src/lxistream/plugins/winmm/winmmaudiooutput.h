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

#ifndef WINMMAUDIOOUTPUT_H
#define WINMMAUDIOOUTPUT_H

#include <windows.h>
#include <mmsystem.h>
#include <QtCore>
#include <LXiStreamDevice>

namespace LXiStreamDevice {
namespace WinMMBackend {

class WinMMAudioOutput : public SInterfaces::AudioOutput
{
Q_OBJECT
public:
                                WinMMAudioOutput(const QString &, QObject *);
  virtual                       ~WinMMAudioOutput();

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual STime                 latency(void) const;

  virtual void                  consume(const SAudioBuffer &);

private:
  void                          openCodec(const SAudioFormat &);
  void                          writeHeader(const qint16 *data, size_t size, const SAudioBuffer &buffer);
  void                          flushHeaders(void);

private:
  static const unsigned         maxDelay = 250; // ms

  HWAVEOUT                      waveOut;
  bool                          errorProduced;

  STime                         outLatency;
  SAudioFormat                  inFormat;

  QQueue< QPair<WAVEHDR *, SAudioBuffer> > headers;
};

} } // End of namespaces

#endif
