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
#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace LXiCore {

const QEvent::Type  SScheduler::scheduleEventType = QEvent::Type(QEvent::registerEventType());

struct SScheduler::Data
{
  inline Data(void) : mutex(QMutex::Recursive) { }

  QThreadPool                 * threadPool;
  int                           priority;
  QMutex                        mutex;
  QMap<Dependency *, TaskQueue> taskQueues;
  QAtomicInt                    taskCount;

  QFile                       * traceFile;
  QMap<QThread *, int>          traceThreadMap;
  QTime                         traceTimer;
  int                           traceWidth;
  static const int              traceLineHeight = 20;
  static const int              traceSecWidth = 10000;
};

SScheduler::SScheduler(QThreadPool *threadPool)
  : d(new Data())
{
  d->threadPool = threadPool;
  d->priority = 0;
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

void SScheduler::setPriority(Priority priority)
{
  d->priority = int(priority);
}

SScheduler::Priority SScheduler::priority(void) const
{
  return Priority(d->priority);
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

  d->traceTimer.start();
  d->traceWidth = 0;
  traceTask(-1, -1, "Start");

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

void SScheduler::start(Runnable *runnable, Priority priority)
{
  d->taskCount.ref();
  runnable->scheduler = this;

  const int prio = d->priority + int(priority);

  if (runnable->depends != NULL)
  {
    if (__expect(runnable->depends->scheduler != this, false))
      qFatal("The specified task dependency is not created for this scheduler.");

    QMutexLocker l(&d->mutex);

    QMap<Dependency *, TaskQueue>::Iterator i = d->taskQueues.find(runnable->depends);
    if (i == d->taskQueues.end())
    {
      if (runnable->depends->tryLock())
      {
        d->threadPool->start(runnable, prio);
        return;
      }

      i = d->taskQueues.insert(runnable->depends, TaskQueue());
    }
    else if (i->isEmpty())
    {
      if (runnable->depends->tryLock())
      {
        d->threadPool->start(runnable, prio);
        return;
      }
    }

    TaskQueue::Iterator at = qUpperBound(i->begin(), i->end(), prio);
    i->insert(at, qMakePair(runnable, prio));

    queueSchedule(runnable->depends);
  }
  else
    d->threadPool->start(runnable, prio);
}

void SScheduler::schedule(Dependency *depends)
{
  QMutexLocker l(&d->mutex);

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

void SScheduler::traceTask(int startTime, int stopTime, const QByteArray &taskName)
{
  QMutexLocker l(&d->mutex);

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

  const int duration = stopTime - startTime;
  const int taskStart = (startTime * d->traceSecWidth / 1000) + 40;
  const int taskWidth = qMax(1, duration * d->traceSecWidth / 1000);

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
    semaphore(1)
{
}

void SScheduler::Dependency::lock(void)
{
  semaphore.acquire(1);
}

bool SScheduler::Dependency::tryLock(void)
{
  return semaphore.tryAcquire(1);
}

bool SScheduler::Dependency::tryLock(int timeout)
{
  return semaphore.tryAcquire(1, timeout);
}

void SScheduler::Dependency::unlock(void)
{
  semaphore.release(1);
  if (scheduler)
    scheduler->queueSchedule(this);
}


SScheduler::DependencyLocker::DependencyLocker(Dependency *m)
{
  Q_ASSERT_X((reinterpret_cast<quintptr>(m) & quintptr(1u)) == quintptr(0),
             "DependencyLocker", "Dependency pointer is misaligned");
  if (m)
  {
    m->lock();
    val = reinterpret_cast<quintptr>(m) | quintptr(1u);
  }
  else
    val = 0;
}

SScheduler::DependencyLocker::~DependencyLocker()
{
  unlock();
}

void SScheduler::DependencyLocker::unlock()
{
  if ((val & quintptr(1u)) == quintptr(1u))
  {
    val &= ~quintptr(1u);
    dependency()->unlock();
  }
}

void SScheduler::DependencyLocker::relock()
{
  if (val)
  {
    if ((val & quintptr(1u)) == quintptr(0u))
    {
      dependency()->lock();
      val |= quintptr(1u);
    }
  }
}

SScheduler::Dependency * SScheduler::DependencyLocker::dependency() const
{
  return reinterpret_cast<Dependency *>(val & ~quintptr(1u));
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
    const int startTime = scheduler->d->traceTimer.elapsed();

    run(scheduler);

    const int endTime = scheduler->d->traceTimer.elapsed();

#ifdef __GNUC__
    char demangled[256];
    size_t demangledSize = sizeof(demangled);
    if (abi::__cxa_demangle(name, demangled, &demangledSize, NULL) != NULL)
      scheduler->traceTask(startTime, endTime, demangled);
    else
#endif
      scheduler->traceTask(startTime, endTime, name);
  }
  else
    run(scheduler);
}

} // End of namespace
