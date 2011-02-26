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

#include "sgraph.h"
#include "sapplication.h"
#include "sdebug.h"
#include "stimer.h"

namespace LXiStream {


struct SGraph::Private
{
  QVector<Node *>               nodes;
  QVector<SourceNode *>         sourceNodes;
  QVector<SinkNode *>           sinkNodes;

  QThread                     * parentThread;
  STimer                        timer;
  int                           minTaskCount;
  bool                          started;
  bool                          stopping;
  bool                          stopped;
  int                           stopTimer;
  int                           stopCounter;

  static const QEvent::Type     scheduleSourceEventType;
  static const QEvent::Type     stopEventloopEventType;
};

const QEvent::Type SGraph::Private::scheduleSourceEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type SGraph::Private::stopEventloopEventType = QEvent::Type(QEvent::registerEventType());

SGraph::SGraph(void)
       :QThread(NULL),
        p(new Private())
{
  p->parentThread = QThread::thread();
  p->minTaskCount = QThread::idealThreadCount();
  p->started = false;
  p->stopping = false;
  p->stopped = true;
  p->stopTimer = -1;
  p->stopCounter = 0;
}

SGraph::~SGraph()
{
  if (QThread::currentThread() == this)
    qFatal("An SGraph can not delete itself");

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

bool SGraph::isRunning(void) const
{
  return p->started;
}

void SGraph::addNode(Node *node)
{
  p->nodes += node;
}

void SGraph::addNode(SourceNode *node)
{
  p->sourceNodes += node;
}

void SGraph::addNode(SinkNode *node)
{
  p->sinkNodes += node;
}

bool SGraph::start(void)
{
  if (!p->started)
  {
    p->stopping = false;
    p->stopped = false;
    p->timer.reset();

    QVector<SourceNode *> startedSources;
    QVector<SinkNode *> startedSinks;

    foreach (SourceNode *source, p->sourceNodes)
    if (source->start())
    {
      startedSources += source;
    }
    else
    {
      foreach (SourceNode *source, startedSources)
        source->stop();

      return false;
    }

    foreach (SinkNode *sink, p->sinkNodes)
    if (sink->start(&(p->timer)))
    {
      startedSinks += sink;
    }
    else
    {
      foreach (SourceNode *source, startedSources)
        source->stop();

      foreach (SinkNode *sink, startedSinks)
        sink->stop();

      return false;
    }

    // Ensure event handling occurs on the running thread.
    QThread::moveToThread(this);

    QThread::start();
    p->started = true;
    return true;
  }
  else
  {
    qWarning() << "Graph already started.";
    return false;
  }
}

void SGraph::stop(void)
{
  if (p->started)
    p->stopping = true;

  if (p->stopping && (QThread::currentThread() != this))
    QThread::wait();
}

void SGraph::queueSchedule(Dependency *depends)
{
  QCoreApplication::postEvent(this, new ScheduleEvent(depends));
}

void SGraph::run(void)
{
  QCoreApplication::postEvent(this, new QEvent(p->scheduleSourceEventType));

  QThread::exec();

  foreach (SourceNode *source, p->sourceNodes)
    source->stop();

  foreach (SinkNode *sink, p->sinkNodes)
    sink->stop();

  p->stopped = true;

  // Ensure event handling occurs on the parent thread again.
  QThread::moveToThread(p->parentThread);

  p->started = false;
}

void SGraph::customEvent(QEvent *e)
{
  if (e->type() == scheduleEventType)
  {
    schedule(static_cast<ScheduleEvent *>(e)->depends);

    if (numTasksQueued() < p->minTaskCount)
      QCoreApplication::postEvent(this, new QEvent(p->scheduleSourceEventType), INT_MIN);
  }
  else if (e->type() == p->scheduleSourceEventType)
  {
    if (!p->stopped)
    {
      if (!p->stopping)
      {
        if (numTasksQueued() < p->minTaskCount)
        {
          foreach (SourceNode *source, p->sourceNodes)
            source->process();

          QCoreApplication::postEvent(this, new QEvent(p->scheduleSourceEventType), INT_MIN);
          return;
        }
      }
      else if (numTasksQueued() == 0) // Graph is stopping and no more tasks queued
      {
        if (p->stopTimer == -1)
        {
          p->stopCounter = 2;
          p->stopTimer = startTimer(125);
        }
      }
      else
        p->stopCounter = 2;
    }
  }
  else
    QThread::customEvent(e);
}

void SGraph::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == p->stopTimer)
  {
    if (--p->stopCounter <= 0)
    {
      killTimer(p->stopTimer);
      p->stopTimer = -1;

      QThread::exit(0);
      p->stopped = true;
    }
  }
  else
    QThread::timerEvent(e);
}


SGraph::Node::Node(SGraph *graph)
  : SScheduler::Proxy(graph)
{
  if (graph)
    graph->addNode(this);
}

SGraph::Node::~Node()
{
}


SGraph::SinkNode::SinkNode(SGraph *graph)
  : SScheduler::Proxy(graph)
{
  if (graph)
    graph->addNode(this);
}

SGraph::SinkNode::~SinkNode()
{
}


SGraph::SourceNode::SourceNode(SGraph *graph)
  : SScheduler::Proxy(graph)
{
  if (graph)
    graph->addNode(this);
}

SGraph::SourceNode::~SourceNode()
{
}

} // End of namespace
