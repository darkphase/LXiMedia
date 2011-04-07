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

#ifndef LXSTREAMFRONTEND_STUNER_H
#define LXSTREAMFRONTEND_STUNER_H

#include <QtCore>
#include <LXiCore>

namespace LXiStream {

class S_DSO_PUBLIC STuner : public QObject
{
Q_OBJECT
Q_PROPERTY(quint64 frequency READ frequency WRITE setFrequency)
public:
  struct Status
  {
    bool                        hasSignal;
    bool                        hasCarrier;
    bool                        hasSync;
    bool                        hasLock;
    quint32                     bitErrorRate;
    qreal                       signalStrength;
    qreal                       signalNoiseRatio;
  };

public:
  explicit                      STuner(QObject *parent = NULL);

  virtual quint64               frequency(void) const = 0;
  virtual bool                  setFrequency(quint64) = 0;
  virtual bool                  frequencyInfo(quint64 &low, quint64 &high, quint64 &step) const = 0;
  virtual Status                signalStatus(void) const = 0;

signals:
  void                          statusChanged(const Status &);
};


} // End of namespace

#endif
