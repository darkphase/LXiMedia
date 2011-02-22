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

#include "sthreadpool.h"
#include "sdebug.h"
#include <cxxabi.h>

namespace LXiStream {

struct SThreadPool::Data
{
  inline Data(void) : mutex(QMutex::Recursive) { }

  QMutex                        mutex;
  QMap<SDependency *, TaskQueue> taskQueues;
  int                           scheduleTimer;

  QFile                       * traceFile;
  QMap<QThread *, int>          traceThreadMap;
  STimer                        traceTimer;
  qint64                        traceWidth;
  static const int              traceLineHeight = 20;
  static const int              traceSecWidth = 10000;

  static const QEvent::Type     scheduleEventType;
  static const QEvent::Type     startTimerEventType;
};

const QEvent::Type  SThreadPool::Data::startTimerEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SThreadPool::Data::scheduleEventType = QEvent::Type(QEvent::registerEventType());

class SThreadPool::ScheduleEvent : public QEvent
{
public:
  inline ScheduleEvent(SDependency *depends)
    : QEvent(Data::scheduleEventType), depends(depends)
  {
  }

  SDependency * const depends;
};

SThreadPool::SThreadPool(QObject *parent)
  : QThreadPool(parent),
    d(new Data())
{
  d->scheduleTimer = -1;
  d->traceFile = NULL;
  d->traceWidth = 0;
}

SThreadPool::~SThreadPool()
{
  stopTrace();

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SThreadPool::enableTrace(const QString &fileName)
{
  d->traceFile = new QFile(fileName, this);
  if (!d->traceFile->open(QFile::WriteOnly))
  {
    delete d->traceFile;
    d->traceFile = NULL;
    return false;
  }

  // Reserve space for SVG header
  d->traceFile->write("<svg>" + QByteArray(250, ' ') + '\n');

  d->traceTimer.reset();
  d->traceWidth = 0;
  traceTask(STime::null, STime::null, "Start");

  return true;
}

void SThreadPool::stopTrace(void)
{
  if (d->traceFile)
  {
    // Write SVG trailer
    d->traceFile->write("</svg>\n");

    // Write SVG header
    d->traceFile->seek(0);
    d->traceFile->write(
        "<!-- Trace file created by LXiStream -->\n"
        "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" "
             "xmlns=\"http://www.w3.org/2000/svg\" "
             "version=\"1.1\" id=\"svg2\" "
             "width=\"" + QByteArray::number(d->traceWidth + (d->traceSecWidth / 10)) + "\" "
             "height=\"" + QByteArray::number(d->traceThreadMap.count() * d->traceLineHeight) + "\">");

    delete d->traceFile;
    d->traceFile = NULL;

    d->traceThreadMap.clear();
  }
}

inline bool operator<(int priority, const QPair<SRunnable *, int> &p)
{
    return p.second < priority;
}

inline bool operator<(const QPair<SRunnable *, int> &p, int priority)
{
    return priority < p.second;
}

void SThreadPool::start(SRunnable *runnable, int priority)
{
  runnable->threadPool = this;

  if (runnable->depends != NULL)
  {
    SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

    QMap<SDependency *, TaskQueue>::Iterator i = d->taskQueues.find(runnable->depends);
    if (i == d->taskQueues.end())
    {
      if (runnable->depends->tryLock())
      {
        QThreadPool::start(runnable, priority);
        return;
      }
      else
        i = d->taskQueues.insert(runnable->depends, TaskQueue());
    }

    TaskQueue::Iterator at = qUpperBound(i->begin(), i->end(), priority);
    i->insert(at, qMakePair(runnable, priority));

    if (d->scheduleTimer == -1)
      QCoreApplication::postEvent(this, new QEvent(d->startTimerEventType));
  }
  else
    QThreadPool::start(runnable, priority);
}

SThreadPool * SThreadPool::globalInstance(void)
{
  static SThreadPool globalPool;

  return &globalPool;
}

void SThreadPool::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == d->scheduleTimer)
  {
    if (d->mutex.tryLock())
    {
      schedule();
      d->mutex.unlock();
    }
  }
  else
    QObject::timerEvent(e);
}

void SThreadPool::customEvent(QEvent *e)
{
  if (e->type() == d->scheduleEventType)
  {
    schedule(static_cast<ScheduleEvent *>(e)->depends);
  }
  else if (e->type() == d->startTimerEventType)
  {
    SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

    if (d->scheduleTimer == -1)
      d->scheduleTimer = startTimer(250);
  }
  else
    QObject::customEvent(e);
}

bool SThreadPool::tryStart(SRunnable *)
{
  return false;
}

void SThreadPool::schedule(void)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  for (QMap<SDependency *, TaskQueue>::Iterator i=d->taskQueues.begin(); i!=d->taskQueues.end(); )
  if (!i->isEmpty())
  {
    schedule(i);
    i++;
  }
  else
    i = d->taskQueues.erase(i);

  if (d->taskQueues.isEmpty())
  {
    killTimer(d->scheduleTimer);
    d->scheduleTimer = -1;
  }
}

void SThreadPool::schedule(SDependency *depends)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  QMap<SDependency *, TaskQueue>::Iterator i=d->taskQueues.find(depends);
  if (i!=d->taskQueues.end())
  {
    if (!i->isEmpty())
      schedule(i);
    else
      d->taskQueues.erase(i);
  }
}

void SThreadPool::schedule(const QMap<SDependency *, TaskQueue>::Iterator &queue)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  if (queue.key()->tryLock())
  {
    TaskQueue::Iterator task = queue->begin();
    QThreadPool::start(task->first, 1); // Priority 1 is used as the mutex is already locked.
    queue->erase(task);
  }
}

void SThreadPool::traceTask(STime startTime, STime stopTime, const QByteArray &taskName)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  QMap<QThread *, int>::Iterator threadId = d->traceThreadMap.find(QThread::currentThread());
  if (threadId == d->traceThreadMap.end())
  {
    threadId = d->traceThreadMap.insert(QThread::currentThread(), d->traceThreadMap.count());

    d->traceFile->write(
        "<text x=\"0\" "
              "y=\"" + QByteArray::number((*threadId * d->traceLineHeight) + 10) + "\" "
              "style=\"font-size:8px\">"
          "Thread " + QByteArray::number(*threadId) + "</text>\n");
  }

  const qint64 duration = (stopTime - startTime).toUSec();
  const qint64 taskStart = (startTime.toUSec() * d->traceSecWidth / 1000000) + 40;
  const qint64 taskWidth = qMax(Q_INT64_C(1), duration * d->traceSecWidth / 1000000);

  d->traceWidth = qMax(d->traceWidth, taskStart + taskWidth);

  d->traceFile->write(
      "<rect x=\"" + QByteArray::number(taskStart) + "\" "
            "y=\"" + QByteArray::number(*threadId * d->traceLineHeight) + "\" "
            "width=\"" + QByteArray::number(taskWidth) + "\" "
            "height=\"" + QByteArray::number(d->traceLineHeight) + "\" "
            "style=\"fill:#E0E0FF;stroke:#000000;stroke-width:1\" />\n"
      "<text x=\"" + QByteArray::number(taskStart) + "\" "
            "y=\"" + QByteArray::number((*threadId * d->traceLineHeight) + d->traceLineHeight - 7) + "\" "
            "style=\"font-size:6px\">" + QByteArray::number(duration) + " us</text>\n"
      "<text x=\"" + QByteArray::number(taskStart) + "\" "
            "y=\"" + QByteArray::number((*threadId * d->traceLineHeight) + d->traceLineHeight - 1) + "\" "
            "style=\"font-size:6px\">" + taskName + "</text>\n");

  d->traceFile->flush();
}

SRunnable::SRunnable(const char *name)
  : QRunnable(),
    name(name ? name : typeid(*this).name()),
    depends(NULL),
    threadPool(NULL)
{
}

SRunnable::SRunnable(SDependency *depends, const char *name)
  : QRunnable(),
    name(name ? name : typeid(*this).name()),
    depends(depends),
    threadPool(NULL)
{
}

SRunnable::SRunnable(SRunnable *from)
  : QRunnable(),
    name(from->name),
    depends(from->depends),
    threadPool(from->threadPool)
{
  from->depends = NULL;
}

SRunnable::~SRunnable()
{
  if (depends)
  {
    depends->unlock();
    QCoreApplication::postEvent(threadPool, new SThreadPool::ScheduleEvent(depends));
  }
}

void SRunnable::run(void)
{
  if (threadPool->d->traceFile)
  {
    const STime startTime = threadPool->d->traceTimer.timeStamp();

    run(threadPool);

    const STime endTime = threadPool->d->traceTimer.timeStamp();

    char demangled[256];
    size_t demangledSize = sizeof(demangled);
    if (abi::__cxa_demangle(name, demangled, &demangledSize, NULL) != NULL)
      threadPool->traceTask(startTime, endTime, demangled);
    else
      threadPool->traceTask(startTime, endTime, name);
  }
  else
    run(threadPool);
}

} // End of namespace
