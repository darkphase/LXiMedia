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

#ifndef __PULSEAUDIOINPUT_H
#define __PULSEAUDIOINPUT_H

#include <pulse/simple.h>
#include <QtCore>
#include <LXiStream>

namespace LXiStream {
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
  virtual void                  process(void);

private:
  const QString                 server;
  pa_simple                   * handle;

  STimer                        timer;
  SAudioFormat                  outFormat;
};


} } // End of namespaces

#endif
