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

#ifndef LXISTREAM_SSCHEDULER_H
#define LXISTREAM_SSCHEDULER_H

#include <QtCore>
#include <typeinfo>
#include "stimer.h"

namespace LXiStream {

class SScheduler
{
public:
  enum Priority
  {
    Priority_Idle             = -1000,
    Priority_Lowest           = -100,
    Priority_Low              = -10,
    Priority_Normal           = 0,
    Priority_High             = 10,
    Priority_Highest          = 100,
    Priority_TimeCritical     = 1000
  };

  /*! A task dependency ensures only one task of a set sharing a dependency can
      run simultaneously. It also ensures that tasks sharing a dependency are
      scheduled in the order that they were presented to the scheduler.
   */
  class Dependency
  {
  friend class SScheduler;
  public:
    explicit                    Dependency(SScheduler *scheduler);

    void                        lock(void);
    bool                        tryLock(void);
    bool                        tryLock(int timeout);
    void                        unlock(void);

  private:
    SScheduler          * const scheduler;
    QMutex                      mutex;
  };

  /*! A proxy class that can be used to add the schedule() methods to another
      class.
   */
  class Proxy
  {
  public:
    inline explicit             Proxy(SScheduler *scheduler) : scheduler(scheduler) { }

#ifdef qdoc
    /*! Runs the specified class member method with the specified arguments on a
        thread in the threadpool. Two extra arguments can be provided: a pointer
        to a SDependency instance indicating the task dependency and an int
        indicating the priority. The function is executed immediately if no,
        threadpool is available.
     */
    template<class _base, typename _func>
    inline void                 schedule(_base *object, _func method, ...);
#else
  // sscheduler.hpp provides all template methods that implement the feature
  // specified above.
#define PROXY_METHODS
#include "sscheduler.hpp"
#undef PROXY_METHODS
#endif

  private:
    SScheduler          * const scheduler;
  };

private:
  class Runnable : public QRunnable
  {
  friend class SScheduler;
  public:
    explicit                    Runnable(const char *name = NULL);
    explicit                    Runnable(Dependency *depends, const char *name = NULL);
    virtual                     ~Runnable();

    virtual void                run(SScheduler *) = 0;

  private:
    virtual void                run(void);

  private:
    const char          * const name;
    Dependency                * depends;
    SScheduler                * scheduler;
  };

  friend bool ::LXiStream::operator<(int, const QPair<Runnable *, int> &);
  friend bool ::LXiStream::operator<(const QPair<Runnable *, int> &, int);

  typedef QList< QPair<Runnable *, int> > TaskQueue;

public:
  void                          setPriority(Priority);
  Priority                      priority(void) const;

  bool                          enableTrace(const QString &fileName);
  void                          stopTrace(void);

  int                           numTasksQueued(void) const;
  int                           activeThreadCount(void) const;
  void                          waitForDone(void);

#ifdef qdoc
  /*! Runs the specified class member method with the specified arguments on a
      thread in the threadpool. Two extra arguments can be provided: a pointer
      to a SDependency instance indicating the task dependency and an int
      indicating the priority.
   */
  template<class _base, typename _func>
  inline void                   schedule(_base *object, _func method, ...);
#else
  // sscheduler.hpp provides all template methods that implement the feature
  // specified above.
#define SCHEDULE_METHODS
#include "sscheduler.hpp"
#undef SCHEDULE_METHODS
#endif

protected:
  explicit                      SScheduler(QThreadPool * = QThreadPool::globalInstance());
  virtual                       ~SScheduler();

  virtual void                  queueSchedule(Dependency *) = 0;
  void                          schedule(Dependency *);

private:
  void                          start(Runnable *runnable, Priority = Priority_Normal);
  void                          traceTask(STime, STime, const QByteArray &);

protected:
  static const QEvent::Type     scheduleEventType;
  struct ScheduleEvent : QEvent
  {
    ScheduleEvent(Dependency *depends) : QEvent(scheduleEventType), depends(depends) { };
    Dependency * const depends;
  };

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
