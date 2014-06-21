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

#ifndef PULSEAUDIOINPUT_H
#define PULSEAUDIOINPUT_H

#include <pulse/simple.h>
#include <QtCore>
#include <LXiStreamDevice>

namespace LXiStreamDevice {
namespace PulseAudioBackend {

class PulseAudioInput : public SInterfaces::AudioInput
{
Q_OBJECT
public:
                                PulseAudioInput(const QString &, QObject *);
  virtual                       ~PulseAudioInput();

  virtual void                  setFormat(const SAudioFormat &);
  virtual SAudioFormat          format(void);

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

private:
  bool                          openHandle();

private:
  pa_simple                   * handle;
  QByteArray                    inputName;
  SAudioFormat                  outFormat;
  SAudioBuffer                  silentBuffer;

  STimer                        timer;
  SInterval                     bufferRate;
  STime                         lastBuffer;
  STime                         lastTryOpen;
};

} } // End of namespaces

#endif
