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
  int                           processTimer;
  volatile bool                 running;
};

SGraph::SGraph(void)
       :QThread(NULL),
        d(new Data())
{
  d->parentThread = QThread::thread();
  d->running = false;
  d->processTimer = -1;
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
  return QThread::connect(sender, signal, receiver, member, Qt::DirectConnection);
}

bool SGraph::connect(const QObject *sender, const char *signal, const char *member) const
{
  return QThread::connect(sender, signal, member, Qt::DirectConnection);
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
  if (!d->running)
  {
    // Start nodes
    QVector<SInterfaces::SourceNode *> startedSources;
    QVector<SInterfaces::Node *> startedNodes;
    QVector<SInterfaces::SinkNode *> startedSinks;

    d->running = true;

    foreach (SInterfaces::SourceNode *source, d->sourceNodes)
    if (!source->start())
    {
      qWarning() << "Failed to start source:" << source->metaObject()->className();

      d->running = false;
      break;
    }
    else
      startedSources += source;

    if (d->running)
    foreach (SInterfaces::Node *node, d->nodes)
    if (!node->start())
    {
      qWarning() << "Failed to start node:" << node->metaObject()->className();

      d->running = false;
      break;
    }
    else
      startedNodes += node;

    if (d->running)
    foreach (SInterfaces::SinkNode *sink, d->sinkNodes)
    if (!sink->start(&(d->timer)))
    {
      qWarning() << "Failed to start sink node:" << sink->metaObject()->className();

      d->running = false;
      break;
    }
    else
      startedSinks += sink;

    if (d->running)
    {
      d->timer.reset();

      // Ensure event handling occurs on the running thread.
      QThread::moveToThread(this);
      QThread::start();

      return true;
    }

    foreach (SInterfaces::SourceNode *source, startedSources)
      source->stop();

    foreach (SInterfaces::Node *node, startedNodes)
      node->stop();

    foreach (SInterfaces::SinkNode *sink, startedSinks)
      sink->stop();

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
      QThread::wait();
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

  foreach (SInterfaces::SourceNode *source, d->sourceNodes)
    source->stop();

  foreach (SInterfaces::Node *node, d->nodes)
    node->stop();

  foreach (SInterfaces::SinkNode *sink, d->sinkNodes)
    sink->stop();

  // Ensure event handling occurs on the parent thread again.
  QThread::moveToThread(d->parentThread);
  d->running = false;
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
        QThread::msleep(10);
    }
    else if (QThread::currentThread() == this)
      QThread::exit(0);
  }
  else
    QThread::timerEvent(e);
}

} // End of namespace
