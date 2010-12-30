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

#ifndef LXSTREAM_STASKRUNNER_H
#define LXSTREAM_STASKRUNNER_H

#include <QtCore>
#include "sinterfaces.h"

namespace LXiStream {

class STaskRunner
{
Q_DISABLE_COPY(STaskRunner)
protected:
  class Task : public QRunnable
  {
  public:
    inline                      Task(STaskRunner *parent, QObject *object) : parent(parent), object(object) { }

  public:
    STaskRunner         * const parent;
    QObject             * const object;

    STime                       startTime;
  };

protected:
  explicit                      STaskRunner(void);
  virtual                       ~STaskRunner();

public:
  template<class _base>
  inline void                   runTask(_base *, void(_base::*)(void));
  template<class _base, typename _type1>
  inline void                   runTask(_base *, void(_base::*)(const _type1 &), const _type1 &);
  template<class _base, typename _type1>
  inline void                   runTask(_base *, void(_base::*)(_type1 *), _type1 *);
  template<class _base, typename _type1, typename _type2>
  inline void                   runTask(_base *, void(_base::*)(const _type1 &, const _type2 &), const _type1 &, const _type2 &);
  template<class _base, typename _type1, typename _type2>
  inline void                   runTask(_base *, void(_base::*)(const _type1 &, _type2 *), const _type1 &, _type2 *);

protected:
  virtual void                  run(Task *) = 0;
  virtual void                  start(Task *) = 0;
  virtual void                  finish(Task *) = 0;
};


template<class _base>
void STaskRunner::runTask(_base *object, void(_base::* func)(void))
{
  class T : public Task
  {
  public:
    inline T(STaskRunner *parent, _base *object, void(_base::* func)(void))
      : Task(parent, object), func(func)
    {
    }

    virtual void run(void)
    {
      parent->start(this);
      (static_cast<_base *>(object)->*func)();
      parent->finish(this);
    }

  private:
    void(_base::* const func)(void);
  };

  run(new T(this, object, func));
}

template<class _base, typename _type1>
void STaskRunner::runTask(_base *object, void(_base::* func)(const _type1 &), const _type1 &arg1)
{
  class T : public Task
  {
  public:
    inline T(STaskRunner *parent, _base *object, void(_base::* func)(const _type1 &), const _type1 &arg1)
      : Task(parent, object), func(func), arg1(arg1)
    {
    }

    virtual void run(void)
    {
      parent->start(this);
      (static_cast<_base *>(object)->*func)(arg1);
      parent->finish(this);
    }

  private:
    void(_base::* const func)(const _type1 &);
    const _type1 arg1;
  };

  run(new T(this, object, func, arg1));
}

template<class _base, typename _type1>
void STaskRunner::runTask(_base *object, void(_base::* func)(_type1 *), _type1 *arg1)
{
  class T : public Task
  {
  public:
    inline T(STaskRunner *parent, _base *object, void(_base::* func)(_type1 *), _type1 *arg1)
      : Task(parent, object), func(func), arg1(arg1)
    {
    }

    virtual void run(void)
    {
      parent->start(this);
      (static_cast<_base *>(object)->*func)(arg1);
      parent->finish(this);
    }

  private:
    void(_base::* const func)(_type1 *);
    _type1 * const arg1;
  };

  run(new T(this, object, func, arg1));
}

template<class _base, typename _type1, typename _type2>
void STaskRunner::runTask(_base *object, void(_base::* func)(const _type1 &, const _type2 &), const _type1 &arg1, const _type2 &arg2)
{
  class T : public Task
  {
  public:
    inline T(STaskRunner *parent, _base *object, void(_base::* func)(const _type1 &, const _type2 &), const _type1 &arg1, const _type2 &arg2)
      : Task(parent, object), func(func), arg1(arg1), arg2(arg2)
    {
    }

    virtual void run(void)
    {
      parent->start(this);
      (static_cast<_base *>(object)->*func)(arg1, arg2);
      parent->finish(this);
    }

  private:
    void(_base::* const func)(const _type1 &, const _type2 &);
    const _type1 arg1;
    const _type2 arg2;
  };

  run(new T(this, object, func, arg1, arg2));
}

template<class _base, typename _type1, typename _type2>
void STaskRunner::runTask(_base *object, void(_base::* func)(const _type1 &, _type2 *), const _type1 &arg1, _type2 *arg2)
{
  class T : public Task
  {
  public:
    inline T(STaskRunner *parent, _base *object, void(_base::* func)(const _type1 &, _type2 *), const _type1 &arg1, _type2 *arg2)
      : Task(parent, object), func(func), arg1(arg1), arg2(arg2)
    {
    }

    virtual void run(void)
    {
      parent->start(this);
      (static_cast<_base *>(object)->*func)(arg1, arg2);
      parent->finish(this);
    }

  private:
    void(_base::* const func)(const _type1 &, _type2 *);
    const _type1 arg1;
    _type2 * const arg2;
  };

  run(new T(this, object, func, arg1, arg2));
}

} // End of namespace

#endif
