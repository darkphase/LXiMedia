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

#ifndef LXISTREAM_STHREADPOOL_H
#define LXISTREAM_STHREADPOOL_H

#include <QtCore>
#include <typeinfo>
#include "stimer.h"

namespace LXiStream {

class SThreadPool;
class SDependency;

class SRunnable : public QRunnable
{
friend class SThreadPool;
public:
  explicit                      SRunnable(const char *name = NULL);
  explicit                      SRunnable(SDependency *depends, const char *name = NULL);
  explicit                      SRunnable(SRunnable *from);
  virtual                       ~SRunnable();

  virtual void                  run(SThreadPool *) = 0;

private:
  virtual void                  run(void);

private:
  const char            * const name;
  SDependency                 * depends;
  SThreadPool                 * threadPool;
};

class SFunctionRunner
{
public:
#ifdef qdoc
  /*! Runs the specified class member method with the specified arguments on a
      thread in the threadpool. Two extra arguments can be provided: a pointer
      to a SDependency instance indicating the task dependency and an int
      indicating the priority.
   */
  template<class _base, typename _func>
  inline void                   queue(_base *object, _func method, ...);
#else
  // sthreadpool.hpp provides all template methods that implement the feature
  // specified above.
#include "sthreadpool.hpp"
#endif

protected:
  virtual void                  start(SRunnable *runnable, int priority = 0) = 0;
};

class SThreadPool : public QThreadPool,
                    public SFunctionRunner
{
Q_OBJECT
friend class SRunnable;
private:
  typedef QList< QPair<SRunnable *, int> > TaskQueue;
  class ScheduleEvent;

public:
                                SThreadPool(QObject *parent = NULL);
  virtual                       ~SThreadPool();

  bool                          enableTrace(const QString &fileName);
  void                          stopTrace(void);

  virtual void                  start(SRunnable *runnable, int priority = 0);

  static SThreadPool          * globalInstance(void);

protected:
  virtual void                  timerEvent(QTimerEvent *);
  virtual void                  customEvent(QEvent *);

private:
  bool                          tryStart(SRunnable *);
  void                          schedule(void);
  void                          schedule(SDependency *);
  void                          schedule(const QMap<SDependency *, TaskQueue>::Iterator &);
  void                          traceTask(STime, STime, const QByteArray &);

private:
  struct Data;
  Data                  * const d;
};

/*! Only non-recursive mutexes are allowed as task dependencies.
 */
class SDependency : public QMutex
{
public:
  inline                        SDependency(void) : QMutex(QMutex::NonRecursive) { }
};

} // End of namespace

#endif
