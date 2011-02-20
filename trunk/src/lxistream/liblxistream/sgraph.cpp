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
  inline Private(void) : mutex(QMutex::Recursive) { }

  QVector<SInterfaces::Node *>       nodes;
  QVector<SInterfaces::SourceNode *> sourceNodes;
  QVector<SInterfaces::SinkNode *>   sinkNodes;

  SThreadPool                 * threadPool;
  QThread                     * parentThread;
  int                           processTimer;
  STimer                        timer;
  bool                          started;
  bool                          stopping;
  bool                          stopped;
  int                           stopTimer;
  int                           stopCounter;
  QAtomicInt                    taskCount;

  QMutex                        mutex;
  int                           priority;

  static const QEvent::Type     scheduleSourceEventType;
  static const QEvent::Type     stopEventloopEventType;
};

const QEvent::Type SGraph::Private::scheduleSourceEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type SGraph::Private::stopEventloopEventType = QEvent::Type(QEvent::registerEventType());

SGraph::SGraph(void)
       :QThread(NULL),
        p(new Private())
{
  p->threadPool = SThreadPool::globalInstance();
  p->parentThread = QThread::thread();
  p->processTimer = -1;
  p->started = false;
  p->stopping = false;
  p->stopped = true;
  p->stopTimer = -1;
  p->stopCounter = 0;
  p->taskCount = 0;
  p->priority = 0;
}

SGraph::~SGraph()
{
  if (QThread::currentThread() == this)
    qFatal("An SGraph can not delete itself");

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SGraph::addNode(SInterfaces::Node *node)
{
  p->nodes += node;
}

void SGraph::addNode(SInterfaces::SourceNode *node)
{
  p->sourceNodes += node;
}

void SGraph::addNode(SInterfaces::SinkNode *node)
{
  p->sinkNodes += node;
}

void SGraph::setPriority(Priority priority)
{
  p->priority = priority;
}

SGraph::Priority SGraph::priority(void) const
{
  return Priority(p->priority);
}

bool SGraph::isRunning(void) const
{
  return p->started;
}

bool SGraph::start(void)
{
  if (!p->started)
  {
    p->stopping = false;
    p->stopped = false;
    p->timer.reset();

    QVector<SInterfaces::SourceNode *> startedSources;
    QVector<SInterfaces::SinkNode *> startedSinks;

    foreach (SInterfaces::SourceNode *source, p->sourceNodes)
    if (source->start())
    {
      startedSources += source;
    }
    else
    {
      foreach (SInterfaces::SourceNode *source, startedSources)
        source->stop();

      return false;
    }

    foreach (SInterfaces::SinkNode *sink, p->sinkNodes)
    if (sink->start(&(p->timer)))
    {
      startedSinks += sink;
    }
    else
    {
      foreach (SInterfaces::SourceNode *source, startedSources)
        source->stop();

      foreach (SInterfaces::SinkNode *sink, startedSinks)
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

void SGraph::start(SRunnable *runnable, int priority)
{
  class Runnable : public SRunnable
  {
  public:
    Runnable(SRunnable *parent, SGraph *graph)
      : SRunnable(parent), parent(parent), graph(graph)
    {
      graph->p->taskCount.ref();
    }

    virtual ~Runnable()
    {
      graph->p->taskCount.deref();
      delete parent;

      QCoreApplication::postEvent(graph, new QEvent(graph->p->scheduleSourceEventType), INT_MIN);
    }

  protected:
    virtual void run(SThreadPool *pool)
    {
      parent->run(pool);
    }

  private:
    SRunnable             * const parent;
    SGraph                * const graph;
  };

  if (p->started && !p->stopping)
    p->threadPool->start(new Runnable(runnable, this), priority == 0 ? p->priority : priority);
}

void SGraph::run(void)
{
  QCoreApplication::postEvent(this, new QEvent(p->scheduleSourceEventType));

  QThread::exec();

  foreach (SInterfaces::SourceNode *source, p->sourceNodes)
    source->stop();

  foreach (SInterfaces::SinkNode *sink, p->sinkNodes)
    sink->stop();

  p->stopped = true;

  // Ensure event handling occurs on the parent thread again.
  QThread::moveToThread(p->parentThread);

  p->started = false;
}

void SGraph::customEvent(QEvent *e)
{
  if (e->type() == p->scheduleSourceEventType)
  {
    SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

    if (!p->stopped)
    {
      if (!p->stopping)
      {
        if (p->taskCount < 3)
        {
          l.unlock();

          foreach (SInterfaces::SourceNode *source, p->sourceNodes)
            source->process();

          QCoreApplication::postEvent(this, new QEvent(p->scheduleSourceEventType), INT_MIN);
          return;
        }
      }
      else if (p->taskCount == 0) // Graph is stopping and no more tasks queued
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

} // End of namespace
