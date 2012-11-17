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

#ifndef WINMMAUDIOINPUT_H
#define WINMMAUDIOINPUT_H

#include <windows.h>
#include <mmsystem.h>
#include <QtCore>
#include <LXiStreamDevice>

namespace LXiStreamDevice {
namespace WinMMBackend {

class WinMMAudioInput : public SInterfaces::AudioInput
{
Q_OBJECT
public:
                                WinMMAudioInput(const QString &, QObject *);
  virtual                       ~WinMMAudioInput();

  virtual void                  setFormat(const SAudioFormat &);
  virtual SAudioFormat          format(void);

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

private:
  void                          queueHeaders(void);
  void                          flushHeaders(void);

private:
  static const unsigned         maxDelay = 1000; // ms

  HWAVEIN                       waveIn;
  QByteArray                    inputName;
  SAudioFormat                  inFormat;

  STimer                        timer;
  QQueue< QPair<WAVEHDR *, SAudioBuffer> > headers;
};

} } // End of namespaces

#endif
