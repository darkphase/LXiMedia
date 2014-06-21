/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include "sgraph.h"
#include "sbuffer.h"
#include "sinterfaces.h"
#include "stimer.h"

namespace LXiStream {

struct SGraph::Data
{
  QVector<SInterfaces::SourceNode *> sourceNodes;
  QVector<SInterfaces::Node *>  nodes;
  QVector<SInterfaces::SinkNode *> sinkNodes;

  QList<SInterfaces::SourceNode *> startedSources;
  QList<SInterfaces::Node *>    startedNodes;
  QList<SInterfaces::SinkNode *> startedSinks;

  QThread                     * parentThread;
  QThreadPool                 * threadPool;
  STimer                        timer;
  int                           processTimer;
  volatile bool                 running;

  static const QEvent::Type     stopEventType;
};

const QEvent::Type  SGraph::Data::stopEventType = QEvent::Type(QEvent::registerEventType());

SGraph::SGraph(void)
       :QThread(NULL),
        d(new Data())
{
  d->parentThread = QThread::thread();
  d->threadPool = QThreadPool::globalInstance();
  d->running = false;
  d->processTimer = -1;
}

SGraph::~SGraph()
{
  if (QThread::currentThread() != this)
  {
    if (!QThread::wait(1000))
      QThread::terminate();
  }
  else
    qFatal("An SGraph can not delete itself");

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SGraph::isRunning(void) const
{
  return d->running;
}

void SGraph::addNode(SInterfaces::Node *node)
{
  d->nodes += node;
}

void SGraph::addNode(SInterfaces::SourceNode *node)
{
  d->sourceNodes += node;
}

void SGraph::addNode(SInterfaces::SinkNode *node)
{
  d->sinkNodes += node;
}

bool SGraph::start(void)
{
  QThread::setTerminationEnabled();

  if (!d->running)
  {
    if (startNodes())
    {
      d->running = true;
      d->timer.reset();

      // Ensure event handling occurs on the running thread.
      QThread::moveToThread(this);
      QThread::start();

      return true;
    }
    else
      return false;
  }
  else
  {
    qWarning() << "Graph already started.";
    return false;
  }
}

void SGraph::stop(void)
{
  if (d->running)
  {
    d->running = false;

    if (QThread::currentThread() != this)
    {
      QThread::wait();

      stopNodes();
    }
  }
}

void SGraph::run(void)
{
  if (d->running)
  {
    d->processTimer = startTimer(0);

    QThread::exec();

    killTimer(d->processTimer);
    d->processTimer = -1;
  }

  // Ensure event handling occurs on the parent thread again.
  QThread::moveToThread(d->parentThread);
  d->running = false;

  qApp->postEvent(this, new QEvent(d->stopEventType));
}

void SGraph::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == d->processTimer)
  {
    if (d->running)
    {
      bool produced = false;
      foreach (SInterfaces::SourceNode *source, d->sourceNodes)
        produced |= source->process();

      if (!produced)
        QThread::msleep(40);
    }
    else if (QThread::currentThread() == this)
      QThread::exit(0);
  }
  else
    QThread::timerEvent(e);
}

void SGraph::customEvent(QEvent *e)
{
  if (e->type() == d->stopEventType)
    stopNodes();
  else
    SGraph::customEvent(e);
}

bool SGraph::startNodes(void)
{
  bool result = true;

  foreach (SInterfaces::SourceNode *source, d->sourceNodes)
  if (!source->start())
  {
    qWarning() << "Failed to start source:" << source->metaObject()->className();

    result = false;
    break;
  }
  else
    d->startedSources += source;

  if (result)
  foreach (SInterfaces::Node *node, d->nodes)
  {
    if (!node->start())
    {
      qWarning() << "Failed to start node:" << node->metaObject()->className();

      result = false;
      break;
    }
    else
      d->startedNodes += node;
  }

  if (result)
  foreach (SInterfaces::SinkNode *sink, d->sinkNodes)
  {
    if (!sink->start(&(d->timer)))
    {
      qWarning() << "Failed to start sink node:" << sink->metaObject()->className();

      result = false;
      break;
    }
    else
      d->startedSinks += sink;
  }

  if (!result)
    stopNodes();

  return result;
}

void SGraph::stopNodes(void)
{
  while (!d->startedSources.isEmpty())
    d->startedSources.takeFirst()->stop();

  while (!d->startedNodes.isEmpty())
    d->startedNodes.takeFirst()->stop();

  while (!d->startedSinks.isEmpty())
    d->startedSinks.takeFirst()->stop();
}

} // End of namespace
