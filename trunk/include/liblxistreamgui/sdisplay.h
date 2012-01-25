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

#ifndef LXSTREAM_SDISPLAY_H
#define LXSTREAM_SDISPLAY_H

#include <QtCore>
#include <QtGui>
#include <LXiCore>
#include "export.h"

namespace LXiStreamGui {

class LXISTREAMGUI_PUBLIC SDisplay : public QObject
{
Q_OBJECT
public:
  struct Mode
  {
    QSize                       size;
    qreal                       rate;
  };

public:
                                SDisplay(void);
  virtual                       ~SDisplay();

  void                          blockScreenSaver(bool);

  QList<Mode>                   allModes(void) const;
  Mode                          mode(void) const;
  bool                          setMode(const Mode &);

  bool                          setSize(const QSize &);
  inline bool                   setSize(int w, int h) { return setSize(QSize(w, h)); }

  QList<qreal>                  allRates(void) const;
  bool                          setRate(qreal);

protected:
  virtual void                  timerEvent(QTimerEvent *);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
