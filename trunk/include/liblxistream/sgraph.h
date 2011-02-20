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

#ifndef LXSTREAM_SGRAPH_H
#define LXSTREAM_SGRAPH_H

#include <QtCore>
#include "sinterfaces.h"
#include "sthreadpool.h"

namespace LXiStream {

class SGraph : public QThread,
               public SFunctionRunner
{
Q_OBJECT
Q_DISABLE_COPY(SGraph)
public:
  enum Priority
  {
    Priority_Idle             = INT_MIN,
    Priority_Lowest           = -100,
    Priority_Low              = -10,
    Priority_Normal           = 0,
    Priority_High             = 10,
    Priority_Highest          = 100,
    Priority_TimeCritical     = INT_MAX
  };

public:
  explicit                      SGraph(void);
  virtual                       ~SGraph();

  void                          addNode(SInterfaces::Node *);
  void                          addNode(SInterfaces::SourceNode *);
  void                          addNode(SInterfaces::SinkNode *);

  void                          setPriority(Priority);
  Priority                      priority(void) const;

  bool                          isRunning(void) const;

public slots:
  virtual bool                  start(void);
  virtual void                  stop(void);

protected: // From SFunctionRunner
  virtual void                  start(SRunnable *runnable, int priority = 0);

protected: // From QThread and QObject
  virtual void                  run(void);
  virtual void                  customEvent(QEvent *);
  virtual void                  timerEvent(QTimerEvent *);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
