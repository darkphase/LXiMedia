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
#include "sscheduler.h"

namespace LXiStream {

class SGraph : public QThread,
               public SScheduler
{
Q_OBJECT
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

  /*! The Node abstract class is used for processing nodes.
   */
  class Node : public SScheduler::Proxy
  {
  public:
    explicit                      Node(SGraph *);
    virtual                       ~Node();
  };

  /*! The SinkNode abstract class is used for sink nodes.
   */
  class SinkNode : public SScheduler::Proxy
  {
  public:
    explicit                      SinkNode(SGraph *);
    virtual                       ~SinkNode();

    virtual bool                  start(STimer *) = 0;
    virtual void                  stop(void) = 0;
  };

  /*! The SourceNode abstract class is used for source nodes.
   */
  class SourceNode : public SScheduler::Proxy
  {
  public:
    explicit                      SourceNode(SGraph *);
    virtual                       ~SourceNode();

    virtual bool                  start(void) = 0;
    virtual void                  stop(void) = 0;
    virtual void                  process(void) = 0;

  protected:
    mutable QMutex                mutex;
  };

public:
  explicit                      SGraph(void);
  virtual                       ~SGraph();

  void                          setPriority(Priority);
  Priority                      priority(void) const;

  bool                          isRunning(void) const;

  void                          addNode(Node *);
  void                          addNode(SourceNode *);
  void                          addNode(SinkNode *);

public slots:
  virtual bool                  start(void);
  virtual void                  stop(void);

protected: // From SScheduler
  virtual void                  queueSchedule(Dependency *);

protected: // From QThread and QObject
  virtual void                  run(void);
  virtual void                  customEvent(QEvent *);
  virtual void                  timerEvent(QTimerEvent *);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

Q_DECLARE_INTERFACE(LXiStream::SGraph::Node, "nl.dds.admiraal.www.LXiStream.SGraph.Node/1.0")
Q_DECLARE_INTERFACE(LXiStream::SGraph::SinkNode, "nl.dds.admiraal.www.LXiStream.SGraph.SinkNode/1.0")
Q_DECLARE_INTERFACE(LXiStream::SGraph::SourceNode, "nl.dds.admiraal.www.LXiStream.SGraph.SourceNode/1.0")

#endif
