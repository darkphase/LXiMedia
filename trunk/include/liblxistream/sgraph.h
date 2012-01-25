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

#ifndef LXSTREAM_SGRAPH_H
#define LXSTREAM_SGRAPH_H

#include <QtCore>
#include <LXiCore>
#include "export.h"

namespace LXiStream {

class STimer;

namespace SInterfaces {
  class Node;
  class SourceNode;
  class SinkNode;
}

class LXISTREAM_PUBLIC SGraph : public QThread
{
Q_OBJECT
public:
  explicit                      SGraph(void);
  virtual                       ~SGraph();

  bool                          isRunning(void) const;

  void                          addNode(SInterfaces::Node *);
  void                          addNode(SInterfaces::SourceNode *);
  void                          addNode(SInterfaces::SinkNode *);

public slots:
  virtual bool                  start(void);
  virtual void                  stop(void);

protected: // From QThread and QObject
  virtual void                  run(void);
  virtual void                  timerEvent(QTimerEvent *);
  virtual void                  customEvent(QEvent *);

private:
  bool                          startNodes(void);
  void                          stopNodes(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
