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

#ifndef LXSTREAM_STIMER_H
#define LXSTREAM_STIMER_H

#include <QtGlobal>
#include <LXiCore>
#include "stime.h"
#include "export.h"

namespace LXiStream {

class LXISTREAM_PUBLIC STimer
{
Q_DISABLE_COPY(STimer)
public:
                                STimer(void);
                                ~STimer(void);

  void                          sync(STimer &);

  STime                         timeStamp(void) const;
  void                          setTimeStamp(STime);
  void                          reset(void);
  STime                         correctTimeStamp(STime ts, STime maxOffset);
  STime                         offset(STime) const;
  STime                         correctOffset(STime ts, STime maxOffset = STime::fromMSec(50));
  void                          pause(bool);

  STime                         smoothTimeStamp(STime interval = STime(), STime delay = STime::null);
  STime                         absoluteTimeStamp(void) const;

private:
  class Init;

  struct Data;
  Data                        * d;

  struct IntervalData;
  IntervalData                * id;
};

} // End of namespace

#endif
