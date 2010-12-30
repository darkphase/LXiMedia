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

#ifndef __WINMMAUDIODEVICE_H
#define __WINMMAUDIODEVICE_H

#include <windows.h>
#include <mmsystem.h>
#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace WinMMBackend {

class WinMMAudioOutput;


class WinMMAudioDevice : public STerminals::AudioDevice
{
Q_OBJECT
public:
  static SSystem::DeviceEntryList listDevices(void);

public:
                                WinMMAudioDevice(QObject *);
  virtual                       ~WinMMAudioDevice();

public: // From STerminal
  virtual bool                  open(const QUrl &);

  virtual QString               friendlyName(void) const;
  virtual QString               longName(void) const;
  virtual Types                 terminalType(void) const;

  virtual QList<Stream>         inputStreams(void) const;
  virtual QList<Stream>         outputStreams(void) const;
  virtual SNode               * openStream(const Stream &);

private:
  static QMap<QString, int>     foundDevices;
  static QMutex                 mutex;
  static QMap<int, WinMMAudioOutput *> outputs;

  int                           dev;

  QList<WinMMAudioOutput *>     openedExclusive;
};


} } // End of namespaces

#endif
