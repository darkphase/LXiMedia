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
#include "sbuffer.h"
#include "sinterfaces.h"
#include "stimer.h"

namespace LXiStream {


struct SGraph::Data
{
  QVector<SInterfaces::Node *> nodes;
  QVector<SInterfaces::SourceNode *> sourceNodes;
  QVector<SInterfaces::SinkNode *> sinkNodes;

  QThread                     * parentThread;
  STimer                        timer;
  bool                          started;
  bool                          stopping;
  bool                          stopped;

  static QAtomicInt             graphsRunning;

  static const QEvent::Type     scheduleSourceEventType;
  static const QEvent::Type     stopEventloopEventType;
};

QAtomicInt         SGraph::Data::graphsRunning(0);

const QEvent::Type SGraph::Data::scheduleSourceEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type SGraph::Data::stopEventloopEventType = QEvent::Type(QEvent::registerEventType());

SGraph::SGraph(void)
       :QThread(NULL),
        d(new Data())
{
  d->parentThread = QThread::thread();
  d->started = false;
  d->stopping = false;
  d->stopped = true;
}

SGraph::~SGraph()
{
  if (QThread::currentThread() == this)
    qFatal("An SGraph can not delete itself");

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SGraph::connect(const QObject *sender, const char *signal, const QObject *receiver, const char *member)
{
  return QThread::connect(sender, signal, receiver, member, Qt::QueuedConnection);
}

bool SGraph::connect(const QObject *sender, const char *signal, const char *member) const
{
  return QThread::connect(sender, signal, member, Qt::QueuedConnection);
}

bool SGraph::isRunning(void) const
{
  return d->started;
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
  if (!d->started)
  {
    d->stopping = false;
    d->stopped = false;
    d->timer.reset();

    QVector<SInterfaces::SourceNode *> startedSources;
    QVector<SInterfaces::Node *> startedNodes;
    QVector<SInterfaces::SinkNode *> startedSinks;

    foreach (SInterfaces::SourceNode *source, d->sourceNodes)
    if (source->start())
    {
      startedSources += source;
    }
    else
    {
      qWarning() << "Failed to start source node:" << source->metaObject()->className();

      foreach (SInterfaces::SourceNode *source, startedSources)
        source->stop();

      return false;
    }

    foreach (SInterfaces::Node *node, d->nodes)
    if (node->start())
    {
      startedNodes += node;
    }
    else
    {
      qWarning() << "Failed to start node:" << node->metaObject()->className();

      foreach (SInterfaces::SourceNode *source, startedSources)
        source->stop();

      foreach (SInterfaces::Node *node, startedNodes)
        node->stop();

      return false;
    }

    foreach (SInterfaces::SinkNode *sink, d->sinkNodes)
    if (sink->start(&(d->timer)))
    {
      startedSinks += sink;
    }
    else
    {
      qWarning() << "Failed to start sink node:" << sink->metaObject()->className();

      foreach (SInterfaces::SourceNode *source, startedSources)
        source->stop();

      foreach (SInterfaces::Node *node, startedNodes)
        node->stop();

      foreach (SInterfaces::SinkNode *sink, startedSinks)
        sink->stop();

      return false;
    }

    // Ensure event handling occurs on the running thread.
    QThread::moveToThread(this);

    QThread::start();
    d->started = true;
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
  if (d->started)
    d->stopping = true;

  if (d->stopping)
  {
    QCoreApplication::postEvent(this, new QEvent(d->stopEventloopEventType), INT_MIN);

    if (QThread::currentThread() != this)
      QThread::wait();
  }
}

void SGraph::run(void)
{
  QCoreApplication::postEvent(this, new QEvent(d->scheduleSourceEventType));

  if (d->graphsRunning.fetchAndAddRelaxed(1) == 0)
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount() * 2);

  QThread::exec();

  foreach (SInterfaces::SourceNode *source, d->sourceNodes)
    source->stop();

  for (int i=0; i<d->nodes.count(); i++)
  foreach (SInterfaces::Node *node, d->nodes)
    node->stop();

  foreach (SInterfaces::SinkNode *sink, d->sinkNodes)
    sink->stop();

  d->stopped = true;

  if (d->graphsRunning.fetchAndAddRelaxed(-1) == 1)
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());

  // Ensure event handling occurs on the parent thread again.
  QThread::moveToThread(d->parentThread);

  d->started = false;
}

void SGraph::customEvent(QEvent *e)
{
  if (e->type() == d->scheduleSourceEventType)
  {
    if (!d->stopped && !d->stopping)
    {
      foreach (SInterfaces::SourceNode *source, d->sourceNodes)
        source->process();

      QCoreApplication::postEvent(this, new QEvent(d->scheduleSourceEventType), INT_MIN + 1);
    }
  }
  if (e->type() == d->stopEventloopEventType)
    QThread::exit(0);
  else
    QThread::customEvent(e);
}

} // End of namespace
