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
#include "stimer.h"

namespace LXiStream {


struct SGraph::Private
{
  QVector<Node *>               nodes;
  QVector<SourceNode *>         sourceNodes;
  QVector<SinkNode *>           sinkNodes;

  QThread                     * parentThread;
  STimer                        timer;
  bool                          started;
  bool                          stopping;
  bool                          stopped;

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
  p->started = false;
  p->stopping = false;
  p->stopped = true;
}

SGraph::~SGraph()
{
  if (QThread::currentThread() == this)
    qFatal("An SGraph can not delete itself");

  delete p;
  *const_cast<Private **>(&p) = NULL;
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
    QVector<Node *> startedNodes;
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

    foreach (Node *node, p->nodes)
    if (node->start())
    {
      startedNodes += node;
    }
    else
    {
      foreach (SourceNode *source, startedSources)
        source->stop();

      foreach (Node *node, startedNodes)
        node->stop();

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

      foreach (Node *node, startedNodes)
        node->stop();

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

  if (p->stopping)
  {
    QCoreApplication::postEvent(this, new QEvent(p->stopEventloopEventType), INT_MIN);

    if (QThread::currentThread() != this)
      QThread::wait();
  }
}

void SGraph::run(void)
{
  QCoreApplication::postEvent(this, new QEvent(p->scheduleSourceEventType));

  QThread::exec();

  foreach (SourceNode *source, p->sourceNodes)
    source->stop();

  for (int i=0; i<p->nodes.count(); i++)
  foreach (Node *node, p->nodes)
    node->stop();

  foreach (SinkNode *sink, p->sinkNodes)
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
    if (!p->stopped && !p->stopping)
    {
      foreach (SourceNode *source, p->sourceNodes)
        source->process();

      QCoreApplication::postEvent(this, new QEvent(p->scheduleSourceEventType), INT_MIN + 1);
    }
  }
  if (e->type() == p->stopEventloopEventType)
    QThread::exit(0);
  else
    QThread::customEvent(e);
}


SGraph::Node::Node(SGraph *graph)
{
  if (graph)
    graph->addNode(this);
}

SGraph::Node::~Node()
{
}


SGraph::SinkNode::SinkNode(SGraph *graph)
{
  if (graph)
    graph->addNode(this);
}

SGraph::SinkNode::~SinkNode()
{
}


SGraph::SourceNode::SourceNode(SGraph *graph)
{
  if (graph)
    graph->addNode(this);
}

SGraph::SourceNode::~SourceNode()
{
}

} // End of namespace
