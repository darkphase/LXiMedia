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

#include "sscheduler.h"
#include "sdebug.h"
#include <cxxabi.h>

namespace LXiStream {

const QEvent::Type  SScheduler::scheduleEventType = QEvent::Type(QEvent::registerEventType());

struct SScheduler::Data
{
  inline Data(void) : mutex(QMutex::Recursive) { }

  QThreadPool                 * threadPool;
  QMutex                        mutex;
  QMap<Dependency *, TaskQueue> taskQueues;
  QAtomicInt                    taskCount;

  QFile                       * traceFile;
  QMap<QThread *, int>          traceThreadMap;
  STimer                        traceTimer;
  qint64                        traceWidth;
  static const int              traceLineHeight = 20;
  static const int              traceSecWidth = 10000;
};

SScheduler::SScheduler(QThreadPool *threadPool)
  : d(new Data())
{
  d->threadPool = threadPool;
  d->taskCount = 0;
  d->traceFile = NULL;
  d->traceWidth = 0;
}

SScheduler::~SScheduler()
{
  stopTrace();

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SScheduler::enableTrace(const QString &fileName)
{
  d->traceFile = new QFile(fileName);
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

void SScheduler::stopTrace(void)
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

int SScheduler::numTasksQueued(void) const
{
  return d->taskCount;
}

int SScheduler::activeThreadCount(void) const
{
  return d->threadPool->activeThreadCount();
}

void SScheduler::waitForDone(void)
{
  // Wait for all tasks to finish
  do
  {
    d->threadPool->waitForDone();
    QCoreApplication::processEvents();
  } while (d->threadPool->activeThreadCount() > 0);
}

inline bool operator<(int priority, const QPair<SScheduler::Runnable *, int> &p)
{
    return p.second < priority;
}

inline bool operator<(const QPair<SScheduler::Runnable *, int> &p, int priority)
{
    return priority < p.second;
}

void SScheduler::start(Runnable *runnable, int priority)
{
  d->taskCount.ref();
  runnable->scheduler = this;

  if (runnable->depends != NULL)
  {
    if (__builtin_expect(runnable->depends->scheduler != this, false))
      qFatal("The specified task dependency is not created for this scheduler.");

    SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

    QMap<Dependency *, TaskQueue>::Iterator i = d->taskQueues.find(runnable->depends);
    if (i == d->taskQueues.end())
    {
      if (runnable->depends->tryLock())
      {
        d->threadPool->start(runnable, priority);
        return;
      }

      i = d->taskQueues.insert(runnable->depends, TaskQueue());
    }
    else if (i->isEmpty())
    {
      if (runnable->depends->tryLock())
      {
        d->threadPool->start(runnable, priority);
        return;
      }
    }

    TaskQueue::Iterator at = qUpperBound(i->begin(), i->end(), priority);
    i->insert(at, qMakePair(runnable, priority));

    queueSchedule(runnable->depends);
  }
  else
    d->threadPool->start(runnable, priority);
}

void SScheduler::schedule(Dependency *depends)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  QMap<Dependency *, TaskQueue>::Iterator i=d->taskQueues.find(depends);
  if (i!=d->taskQueues.end())
  {
    if (!i->isEmpty())
    {
      if (i.key()->tryLock())
      {
        TaskQueue::Iterator task = i->begin();
        d->threadPool->start(task->first, task->second);
        i->erase(task);
      }
    }
    else
      d->taskQueues.erase(i);
  }
}

void SScheduler::traceTask(STime startTime, STime stopTime, const QByteArray &taskName)
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


SScheduler::Dependency::Dependency(SScheduler *scheduler)
  : scheduler(scheduler),
    mutex(QMutex::NonRecursive)
{
}

void SScheduler::Dependency::lock(void)
{
  mutex.lock();
}

bool SScheduler::Dependency::tryLock(void)
{
  return mutex.tryLock();
}

bool SScheduler::Dependency::tryLock(int timeout)
{
  return mutex.tryLock(timeout);
}

void SScheduler::Dependency::unlock(void)
{
  mutex.unlock();
  if (scheduler)
    scheduler->queueSchedule(this);
}


SScheduler::Runnable::Runnable(const char *name)
  : QRunnable(),
    name(name ? name : typeid(*this).name()),
    depends(NULL),
    scheduler(NULL)
{
}

SScheduler::Runnable::Runnable(Dependency *depends, const char *name)
  : QRunnable(),
    name(name ? name : typeid(*this).name()),
    depends(depends),
    scheduler(NULL)
{
}

SScheduler::Runnable::~Runnable()
{
  if (depends)
    depends->unlock();

  if (scheduler)
    scheduler->d->taskCount.deref();
}

void SScheduler::Runnable::run(void)
{
  if (scheduler->d->traceFile)
  {
    const STime startTime = scheduler->d->traceTimer.timeStamp();

    run(scheduler);

    const STime endTime = scheduler->d->traceTimer.timeStamp();

    char demangled[256];
    size_t demangledSize = sizeof(demangled);
    if (abi::__cxa_demangle(name, demangled, &demangledSize, NULL) != NULL)
      scheduler->traceTask(startTime, endTime, demangled);
    else
      scheduler->traceTask(startTime, endTime, name);
  }
  else
    run(scheduler);
}

} // End of namespace
