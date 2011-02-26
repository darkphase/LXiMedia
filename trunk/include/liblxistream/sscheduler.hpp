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
#include "sscheduler.h" // For QtCreator autocompletion only.
#endif

#define LOOPTYPES1X(X, A2, C2, A3, C3, A4, C4) \
  X(_type1,         const _type1,         A2, C2, A3, C3, A4, C4) \
  X(const _type1 &, const _type1,         A2, C2, A3, C3, A4, C4) \
  X(_type1 *,       _type1 * const,       A2, C2, A3, C3, A4, C4) \
  X(const _type1 *, const _type1 *const,  A2, C2, A3, C3, A4, C4)

#define LOOPTYPES1(X) LOOPTYPES1X(X, NULL, NULL, NULL, NULL, NULL, NULL)

#define LOOPTYPES2X(X, A3, C3, A4, C4) \
  LOOPTYPES1X(X, _type2,         const _type2,         A3, C3, A4, C4) \
  LOOPTYPES1X(X, const _type2 &, const _type2,         A3, C3, A4, C4) \
  LOOPTYPES1X(X, _type2 *,      _type2 * const,        A3, C3, A4, C4) \
  LOOPTYPES1X(X, const _type2 *, const _type2 * const, A3, C3, A4, C4)

#define LOOPTYPES2(X) LOOPTYPES2X(X, NULL, NULL, NULL, NULL)

#define LOOPTYPES3X(X, A4, C4) \
  LOOPTYPES2X(X, _type3,         const _type3,         A4, C4) \
  LOOPTYPES2X(X, const _type3 &, const _type3,         A4, C4) \
  LOOPTYPES2X(X, _type3 *,       _type3 * const,       A4, C4) \
  LOOPTYPES2X(X, const _type3 *, const _type3 * const, A4, C4)

#define LOOPTYPES3(X) LOOPTYPES3X(X, NULL, NULL)

#define LOOPTYPES4(X) \
  LOOPTYPES3X(X, _type4,         const _type4) \
  LOOPTYPES3X(X, const _type4 &, const _type4) \
  LOOPTYPES3X(X, _type4 *,       _type4 * const) \
  LOOPTYPES3X(X, const _type4 *, const _type4 * const)


#ifdef PROXY_METHODS

  inline void schedule(void(* func)(void),
           Dependency *depends = NULL, int priority = 0)
  {
    if (scheduler == NULL)
    {
      if (depends) depends->lock();
      func();
      if (depends) depends->unlock();
    }
    else
      scheduler->schedule(func, depends, priority);
  }

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1> \
  inline void schedule(void(* func)(A1), \
           A1 arg1, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      func(arg1); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(func, arg1, depends, priority); \
  }

LOOPTYPES1(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1, typename _type2> \
  inline void schedule(void(* func)(A1, A2), \
           A1 arg1, A2 arg2, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      func(arg1, arg2); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(func, arg1, arg2, depends, priority); \
  }

LOOPTYPES2(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1, typename _type2, typename _type3> \
  inline void schedule(void(* func)(A1, A2, A3), \
           A1 arg1, A2 arg2, A3 arg3, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      func(arg1, arg2, arg3); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(func, arg1, arg2, arg3, depends, priority); \
  }

LOOPTYPES3(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1, typename _type2, typename _type3, typename _type4> \
  inline void schedule(void(* func)(A1, A2, A3, A4), \
           A1 arg1, A2 arg2, A3 arg3, A4 arg4, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      func(arg1, arg2, arg3, arg4); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(func, arg1, arg2, arg3, arg4, depends, priority); \
  }

LOOPTYPES4(FUNC)
#undef FUNC

  template<class _base>
  inline void schedule(void(_base::* func)(void),
                  Dependency *depends = NULL, int priority = 0)
  {
    if (scheduler == NULL)
    {
      if (depends) depends->lock();
      (static_cast<_base *>(this)->*func)();
      if (depends) depends->unlock();
    }
    else
      scheduler->schedule(static_cast<_base *>(this), func, depends, priority);
  }

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1> \
  inline void schedule(void(_base::* func)(A1), \
           A1 arg1, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      (static_cast<_base *>(this)->*func)(arg1); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(static_cast<_base *>(this), func, arg1, depends, priority); \
  }

LOOPTYPES1(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2> \
  inline void schedule(void(_base::* func)(A1, A2), \
           A1 arg1, A2 arg2, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      (static_cast<_base *>(this)->*func)(arg1, arg2); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(static_cast<_base *>(this), func, arg1, arg2, depends, priority); \
  }

LOOPTYPES2(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2, typename _type3> \
  inline void schedule(void(_base::* func)(A1, A2, A3), \
           A1 arg1, A2 arg2, A3 arg3, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      (static_cast<_base *>(this)->*func)(arg1, arg2, arg3); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(static_cast<_base *>(this), func, arg1, arg2, arg3, depends, priority); \
  }

LOOPTYPES3(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2, typename _type3, typename _type4> \
  inline void schedule(void(_base::* func)(A1, A2, A3, A4), \
           A1 arg1, A2 arg2, A3 arg3, A4 arg4, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      (static_cast<_base *>(this)->*func)(arg1, arg2, arg3, arg4); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(static_cast<_base *>(this), func, arg1, arg2, arg3, arg4, depends, priority); \
  }

LOOPTYPES4(FUNC)
#undef FUNC


  template<class _base>
  inline void schedule(_base *object, void(_base::* func)(void),
                  Dependency *depends = NULL, int priority = 0)
  {
    if (scheduler == NULL)
    {
      if (depends) depends->lock();
      (object->*func)();
      if (depends) depends->unlock();
    }
    else
      scheduler->schedule(object, func, depends, priority);
  }

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1> \
  inline void schedule(_base *object, void(_base::* func)(A1), \
           A1 arg1, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      (object->*func)(arg1); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(object, func, arg1, depends, priority); \
  }

LOOPTYPES1(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2> \
  inline void schedule(_base *object, void(_base::* func)(A1, A2), \
           A1 arg1, A2 arg2, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      (object->*func)(arg1, arg2); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(object, func, arg1, arg2, depends, priority); \
  }

LOOPTYPES2(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2, typename _type3> \
  inline void schedule(_base *object, void(_base::* func)(A1, A2, A3), \
           A1 arg1, A2 arg2, A3 arg3, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      (object->*func)(arg1, arg2, arg3); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(object, func, arg1, arg2, arg3, depends, priority); \
  }

LOOPTYPES3(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2, typename _type3, typename _type4> \
  inline void schedule(_base *object, void(_base::* func)(A1, A2, A3, A4), \
           A1 arg1, A2 arg2, A3 arg3, A4 arg4, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    if (scheduler == NULL) \
    { \
      if (depends) depends->lock(); \
      (object->*func)(arg1, arg2, arg3, arg4); \
      if (depends) depends->unlock(); \
    } \
    else \
      scheduler->schedule(object, func, arg1, arg2, arg3, arg4, depends, priority); \
  }

  LOOPTYPES4(FUNC)
  #undef FUNC

#endif

#ifdef SCHEDULE_METHODS

  inline void schedule(void(* func)(void),
           Dependency *depends = NULL, int priority = 0)
  {
    struct T : Runnable
    {
      inline T(void(* func)(void), Dependency *depends)
        : Runnable(depends), func(func) { }

      virtual void run(SScheduler *) { func(); }

      void(* const func)(void);
    };

    start(new T(func, depends), priority);
  }


#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1> \
  inline void schedule(void(* func)(A1), \
           A1 arg1, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    struct T : Runnable  \
    { \
      inline T(void(* func)(A1), \
               A1 arg1, \
               Dependency *depends) \
        : Runnable(depends, typeid(func).name()), func(func), \
          arg1(arg1) { } \
  \
      virtual void run(SScheduler *) { func(arg1); } \
  \
      void(* const func)(A1); \
      C1 arg1; \
    }; \
  \
    start(new T(func, arg1, depends), priority); \
  }

LOOPTYPES1(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1, typename _type2> \
  inline void schedule(void(* func)(A1, A2), \
           A1 arg1, A2 arg2, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    struct T : Runnable  \
    { \
      inline T(void(* func)(A1, A2), \
               A1 arg1, A2 arg2, \
               Dependency *depends) \
        : Runnable(depends, typeid(func).name()), func(func), \
          arg1(arg1), arg2(arg2) { } \
  \
      virtual void run(SScheduler *) { func(arg1, arg2); } \
  \
      void(* const func)(A1, A2); \
      C1 arg1; C2 arg2; \
    }; \
  \
    start(new T(func, arg1, arg2, depends), priority); \
  }

LOOPTYPES2(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1, typename _type2, typename _type3> \
  inline void schedule(void(* func)(A1, A2, A3), \
           A1 arg1, A2 arg2, A3 arg3, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    struct T : Runnable  \
    { \
      inline T(void(* func)(A1, A2, A3), \
               A1 arg1, A2 arg2, A3 arg3, \
               Dependency *depends) \
        : Runnable(depends, typeid(func).name()), func(func), \
          arg1(arg1), arg2(arg2), arg3(arg3) { } \
  \
      virtual void run(SScheduler *) { func(arg1, arg2, arg3); } \
  \
      void(* const func)(A1, A2, A3); \
      C1 arg1; C2 arg2; C3 arg3; \
    }; \
  \
    start(new T(func, arg1, arg2, arg3, depends), priority); \
  }

LOOPTYPES3(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1, typename _type2, typename _type3, typename _type4> \
  inline void schedule(void(* func)(A1, A2, A3, A4), \
           A1 arg1, A2 arg2, A3 arg3, A4 arg4, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    struct T : Runnable  \
    { \
      inline T(void(* func)(A1, A2, A3, A4), \
               A1 arg1, A2 arg2, A3 arg3, A4 arg4, \
               Dependency *depends) \
        : Runnable(depends, typeid(func).name()), func(func), \
          arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4) { } \
  \
      virtual void run(SScheduler *) { func(arg1, arg2, arg3, arg4); } \
  \
      void(* const func)(A1, A2, A3, A4); \
      C1 arg1; C2 arg2; C3 arg3; C4 arg4; \
    }; \
  \
    start(new T(func, arg1, arg2, arg3, arg4, depends), priority); \
  }

LOOPTYPES4(FUNC)
#undef FUNC

  template<class _base>
  inline void schedule(_base *object, void(_base::* func)(void),
           Dependency *depends = NULL, int priority = 0)
  {
    struct T : Runnable
    {
      inline T(_base *object, void(_base::* func)(void), Dependency *depends)
        : Runnable(depends), object(object), func(func) { }

      virtual void run(SScheduler *) { (object->*func)(); }

      _base * const object;
      void(_base::* const func)(void);
    };

    start(new T(object, func, depends), priority);
  }

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1> \
  inline void schedule(_base *object, void(_base::* func)(A1), \
           A1 arg1, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    struct T : Runnable  \
    { \
      inline T(_base *object, void(_base::* func)(A1), \
               A1 arg1, \
               Dependency *depends) \
        : Runnable(depends, typeid(func).name()), object(object), func(func), \
          arg1(arg1) { } \
  \
      virtual void run(SScheduler *) { (object->*func)(arg1); } \
  \
      _base * const object; \
      void(_base::* const func)(A1); \
      C1 arg1; \
    }; \
  \
    start(new T(object, func, arg1, depends), priority); \
  }

LOOPTYPES1(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2> \
  inline void schedule(_base *object, void(_base::* func)(A1, A2), \
           A1 arg1, A2 arg2, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    struct T : Runnable  \
    { \
      inline T(_base *object, void(_base::* func)(A1, A2), \
               A1 arg1, A2 arg2, \
               Dependency *depends) \
        : Runnable(depends, typeid(func).name()), object(object), func(func), \
          arg1(arg1), arg2(arg2) { } \
  \
      virtual void run(SScheduler *) { (object->*func)(arg1, arg2); } \
  \
      _base * const object; \
      void(_base::* const func)(A1, A2); \
      C1 arg1; C2 arg2; \
    }; \
  \
    start(new T(object, func, arg1, arg2, depends), priority); \
  }

LOOPTYPES2(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2, typename _type3> \
  inline void schedule(_base *object, void(_base::* func)(A1, A2, A3), \
           A1 arg1, A2 arg2, A3 arg3, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    struct T : Runnable  \
    { \
      inline T(_base *object, void(_base::* func)(A1, A2, A3), \
               A1 arg1, A2 arg2, A3 arg3, \
               Dependency *depends) \
        : Runnable(depends, typeid(func).name()), object(object), func(func), \
          arg1(arg1), arg2(arg2), arg3(arg3) { } \
  \
      virtual void run(SScheduler *) { (object->*func)(arg1, arg2, arg3); } \
  \
      _base * const object; \
      void(_base::* const func)(A1, A2, A3); \
      C1 arg1; C2 arg2; C3 arg3; \
    }; \
  \
    start(new T(object, func, arg1, arg2, arg3, depends), priority); \
  }

LOOPTYPES3(FUNC)
#undef FUNC

#define FUNC(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2, typename _type3, typename _type4> \
  inline void schedule(_base *object, void(_base::* func)(A1, A2, A3, A4), \
           A1 arg1, A2 arg2, A3 arg3, A4 arg4, \
           Dependency *depends = NULL, int priority = 0) \
  { \
    struct T : Runnable  \
    { \
      inline T(_base *object, void(_base::* func)(A1, A2, A3, A4), \
               A1 arg1, A2 arg2, A3 arg3, A4 arg4, \
               Dependency *depends) \
        : Runnable(depends, typeid(func).name()), object(object), func(func), \
          arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4) { } \
  \
      virtual void run(SScheduler *) { (object->*func)(arg1, arg2, arg3, arg4); } \
  \
      _base * const object; \
      void(_base::* const func)(A1, A2, A3, A4); \
      C1 arg1; C2 arg2; C3 arg3; C4 arg4; \
    }; \
  \
    start(new T(object, func, arg1, arg2, arg3, arg4, depends), priority); \
  }

LOOPTYPES4(FUNC)
#undef FUNC

#endif

#undef LOOPTYPES1
#undef LOOPTYPES2
#undef LOOPTYPES3
#undef LOOPTYPES4
