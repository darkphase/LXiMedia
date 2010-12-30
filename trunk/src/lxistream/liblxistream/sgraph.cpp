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
#include "sdebug.h"
#include "stimer.h"

namespace LXiStream {


struct SGraph::Private
{
  inline Private(void) : mutex(QMutex::Recursive) { }

  QVector<SInterfaces::Node *>       nodes;
  QVector<SInterfaces::SourceNode *> sourceNodes;
  QVector<SInterfaces::SinkNode *>   sinkNodes;

  QThread                     * parentThread;
  int                           processTimer;
  STimer                        timer;
  bool                          started;
  bool                          stopping;
  bool                          stopped;
  int                           stopTimer;
  int                           stopCounter;

  QMutex                        mutex;
  int                           priority;
  QMap<QObject *, QQueue<Task *> > tasks;

  QFile                       * traceFile;
  QMap<QThread *, int>          traceThreadMap;
  STimer                        traceTimer;
  qint64                        traceWidth;

  static const QEvent::Type     scheduleTaskEventType;
  static const QEvent::Type     stopEventloopEventType;
  static const int              traceLineHeight = 20;
  static const int              traceSecWidth = 10000;
};

const QEvent::Type SGraph::Private::scheduleTaskEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type SGraph::Private::stopEventloopEventType = QEvent::Type(QEvent::registerEventType());

SGraph::SGraph(void)
       :QThread(NULL),
        p(new Private())
{
  p->parentThread = QThread::thread();
  p->processTimer = -1;
  p->started = false;
  p->stopping = false;
  p->stopped = true;
  p->stopTimer = -1;
  p->stopCounter = 0;
  p->priority = 0;
  p->traceFile = NULL;
}

SGraph::~SGraph()
{
  if (QThread::currentThread() == this)
    qFatal("An SGraph can not delete itself");

  SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

  int count = 0;
  for (QMap<QObject *, QQueue<Task *> >::Iterator i=p->tasks.begin(); i!=p->tasks.end(); i++)
    count = qMax(count, i->count());

  l.unlock();

  if (count > 0)
    qFatal("Graph still running at destruction, please stop graph first.");

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

bool SGraph::enableTrace(const QString &fileName)
{
  p->traceFile = new QFile(fileName, this);
  if (!p->traceFile->open(QFile::WriteOnly))
  {
    delete p->traceFile;
    p->traceFile = NULL;
    return false;
  }

  return true;
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
      startedSources += source;
    else
    {
      foreach (SInterfaces::SourceNode *source, startedSources)
        source->stop();

      return false;
    }

    foreach (SInterfaces::SinkNode *sink, p->sinkNodes)
    if (sink->start(&(p->timer)))
      startedSinks += sink;
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

void SGraph::run(void)
{
  if (p->traceFile)
  {
    // Reserve space for SVG header
    p->traceFile->write("<svg>" + QByteArray(250, ' ') + '\n');

    p->traceTimer.reset();
    p->traceWidth = 0;
    traceTask(STime::null, STime::null, "Start");
  }

  QCoreApplication::postEvent(this, new QEvent(p->scheduleTaskEventType));

  QThread::exec();

  foreach (SInterfaces::SourceNode *source, p->sourceNodes)
    source->stop();

  foreach (SInterfaces::SinkNode *sink, p->sinkNodes)
    sink->stop();

  p->stopped = true;

  if (p->traceFile)
  {
    // Write SVG trailer
    p->traceFile->write("</svg>\n");

    // Write SVG header
    p->traceFile->seek(0);
    p->traceFile->write(
        "<!-- Trace file created by LXiStream -->\n"
        "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" "
             "xmlns=\"http://www.w3.org/2000/svg\" "
             "version=\"1.1\" id=\"svg2\" "
             "width=\"" + QByteArray::number(p->traceWidth + (p->traceSecWidth / 10)) + "\" "
             "height=\"" + QByteArray::number(p->traceThreadMap.count() * p->traceLineHeight) + "\">");

    delete p->traceFile;
    p->traceFile = NULL;

    p->traceThreadMap.clear();
  }

  // Ensure event handling occurs on the parent thread again.
  QThread::moveToThread(p->parentThread);

  p->started = false;
}

bool SGraph::event(QEvent *e)
{
  if ((p->traceFile == NULL) || !p->started)
  {
    return QThread::event(e);
  }
  else
  {
    const STime startTime = p->traceTimer.timeStamp();
    const bool result = QThread::event(e);
    traceTask(startTime, p->traceTimer.timeStamp(), "Event " + QByteArray::number(e->type()));
    return result;
  }
}

void SGraph::customEvent(QEvent *e)
{
  if (e->type() == p->scheduleTaskEventType)
  {
    SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

    if (!p->stopped)
    {
      int count = 0;
      for (QMap<QObject *, QQueue<Task *> >::Iterator i=p->tasks.begin(); i!=p->tasks.end(); i++)
        count = qMax(count, i->count());

      if (!p->stopping)
      {
        if (count < 3)
        {
          l.unlock();

          foreach (SInterfaces::SourceNode *source, p->sourceNodes)
            source->process();

          QCoreApplication::postEvent(this, new QEvent(p->scheduleTaskEventType), INT_MIN);
          return;
        }
      }
      else if (count == 0) // Graph is stopping and no more tasks queued
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

QThreadPool * SGraph::threadPool(void)
{
  static QThreadPool p;

  return &p;
}

void SGraph::run(Task *task)
{
  SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

  if (!p->stopped)
  {
    QMap<QObject *, QQueue<Task *> >::Iterator i = p->tasks.find(task->object);
    if (i == p->tasks.end())
      i = p->tasks.insert(task->object, QQueue<Task *>());

    if (i->isEmpty())
      threadPool()->start(task, p->priority);

    i->enqueue(task);
  }
}

void SGraph::start(Task *task)
{
  if (p->traceFile)
    task->startTime = p->traceTimer.timeStamp();
}

void SGraph::finish(Task *task)
{
  if (p->traceFile)
    traceTask(task->startTime, p->traceTimer.timeStamp(), task->object->metaObject()->className());

  SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

  QMap<QObject *, QQueue<Task *> >::Iterator i = p->tasks.find(task->object);
  if (i != p->tasks.end())
  {
    i->dequeue();

    if (!i->isEmpty())
      threadPool()->start(i->head(), p->priority);
  }

  QCoreApplication::postEvent(this, new QEvent(p->scheduleTaskEventType), INT_MIN);
}

void SGraph::traceTask(STime startTime, STime stopTime, const QByteArray &taskName)
{
  SDebug::MutexLocker l(&(p->mutex), __FILE__, __LINE__);

  QMap<QThread *, int>::Iterator threadId = p->traceThreadMap.find(QThread::currentThread());
  if (threadId == p->traceThreadMap.end())
  {
    threadId = p->traceThreadMap.insert(QThread::currentThread(), p->traceThreadMap.count());

    p->traceFile->write(
        "<text x=\"0\" "
              "y=\"" + QByteArray::number((*threadId * p->traceLineHeight) + 10) + "\" "
              "style=\"font-size:8px\">"
          "Thread " + QByteArray::number(*threadId) + "</text>\n");
  }

  const qint64 duration = (stopTime - startTime).toUSec();
  const qint64 taskStart = (startTime.toUSec() * p->traceSecWidth / 1000000) + 40;
  const qint64 taskWidth = qMax(Q_INT64_C(1), duration * p->traceSecWidth / 1000000);

  p->traceWidth = qMax(p->traceWidth, taskStart + taskWidth);

  p->traceFile->write(
      "<rect x=\"" + QByteArray::number(taskStart) + "\" "
            "y=\"" + QByteArray::number(*threadId * p->traceLineHeight) + "\" "
            "width=\"" + QByteArray::number(taskWidth) + "\" "
            "height=\"" + QByteArray::number(p->traceLineHeight) + "\" "
            "style=\"fill:#E0E0FF;stroke:#000000;stroke-width:1\" />\n"
      "<text x=\"" + QByteArray::number(taskStart) + "\" "
            "y=\"" + QByteArray::number((*threadId * p->traceLineHeight) + p->traceLineHeight - 7) + "\" "
            "style=\"font-size:6px\">" + QByteArray::number(duration) + " us</text>\n"
      "<text x=\"" + QByteArray::number(taskStart) + "\" "
            "y=\"" + QByteArray::number((*threadId * p->traceLineHeight) + p->traceLineHeight - 1) + "\" "
            "style=\"font-size:6px\">" + taskName + "</text>\n");

  p->traceFile->flush();
}

} // End of namespace
