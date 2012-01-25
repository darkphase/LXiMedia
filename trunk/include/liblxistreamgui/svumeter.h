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

#ifndef LXSTREAM_SVUMETER_H
#define LXSTREAM_SVUMETER_H

#include <QtCore>
#include <QtGui>
#include <LXiCore>
#include <LXiStream>
#include "export.h"

namespace LXiStreamGui {

class LXISTREAMGUI_PUBLIC SVuMeter : public QFrame
{
Q_OBJECT
public:
                                SVuMeter(QWidget *);
  virtual                       ~SVuMeter();

  inline bool                   slow(void) const                                { return slowUpdate; }
  inline void                   setSlow(bool s)                                 { slowUpdate = s; }

  LXiStream::SAudioFormat       inputFormat(void) const;

public slots:
  void                          input(const SAudioBuffer &);
  
protected:
  virtual void                  paintEvent(QPaintEvent *);
  virtual void                  timerEvent(QTimerEvent *);

private:
  QVector<int>                  determinePos(const QVector<qreal> &, const QVector< QQueue<qreal> > &) const;

private:
  static const qreal            maxRms;

  mutable QMutex                mutex;
  bool                          slowUpdate;
  int                           updateTimer;
  volatile bool                 needsUpdate;
  QRect                         myRect;
  QVector<qreal>                rms;
  QVector<qreal>                lastRms;
  QVector< QQueue<qreal> >      peaks;
  QVector< QQueue<qreal> >      lastPeaks;
  LXiStream::SAudioFormat       format;
};

} // End of namespace

#endif
