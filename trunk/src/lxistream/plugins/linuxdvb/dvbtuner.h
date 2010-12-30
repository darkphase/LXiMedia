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

#ifndef LINUXDVBBACKEND_DVBTUNER_H
#define LINUXDVBBACKEND_DVBTUNER_H

#include <QtCore>
#include <LXiStream>
#include "dvbdevice.h"

namespace LXiStream {
namespace LinuxDvbBackend {

class DVBDevice;


class DVBTuner : public SDigitalTuner
{
Q_OBJECT
public:
  explicit                      DVBTuner(DVBDevice *);

  virtual quint64               frequency(void) const;
  virtual bool                  setFrequency(quint64);
  virtual bool                  frequencyInfo(quint64 &low, quint64 &high, quint64 &step) const;
  virtual Status                signalStatus(void) const;

  bool                          hasLock(void) const;

private:
  DVBDevice             * const parent;
};


} } // End of namespaces

#endif
