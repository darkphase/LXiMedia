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
#include "sthreadpool.h" // For QtCreator autocompletion only.
#endif

template<class _base>
inline void queue(_base *object, void(_base::* func)(void),
                SDependency *depends = NULL, int priority = 0)
{
  struct T : SRunnable
  {
    inline T(_base *object, void(_base::* func)(void), SDependency *depends)
      : SRunnable(depends), object(object), func(func) { }

    virtual void run(SThreadPool *) { (object->*func)(); }

    _base * const object;
    void(_base::* const func)(void);
  };

  start(new T(object, func, depends), priority);
}


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


#define RUNSTATICFUNC1(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1>                                                     \
  inline void queue(void(* func)(A1),                                           \
           A1 arg1,                                                             \
           SDependency *depends = NULL, int priority = 0)                       \
  {                                                                             \
    struct T : SRunnable                                                        \
    {                                                                           \
      inline T(void(* func)(A1),                                                \
               A1 arg1,                                                         \
               SDependency *depends)                                            \
        : SRunnable(depends, typeid(func).name()), func(func),                  \
          arg1(arg1) { }                                                        \
                                                                                \
      virtual void run(SThreadPool *) { func(arg1); }                           \
                                                                                \
      void(* const func)(A1);                                                   \
      C1 arg1;                                                                  \
    };                                                                          \
                                                                                \
    start(new T(func, arg1, depends), priority);                                \
  }

LOOPTYPES1(RUNSTATICFUNC1)


#define RUNSTATICFUNC2(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1, typename _type2>                                    \
  inline void queue(void(* func)(A1, A2),                                       \
           A1 arg1, A2 arg2,                                                    \
           SDependency *depends = NULL, int priority = 0)                       \
  {                                                                             \
    struct T : SRunnable                                                        \
    {                                                                           \
      inline T(void(* func)(A1, A2),                                            \
               A1 arg1, A2 arg2,                                                \
               SDependency *depends)                                            \
        : SRunnable(depends, typeid(func).name()), func(func),                  \
          arg1(arg1), arg2(arg2) { }                                            \
                                                                                \
      virtual void run(SThreadPool *) { func(arg1, arg2); }                     \
                                                                                \
      void(* const func)(A1, A2);                                               \
      C1 arg1; C2 arg2;                                                         \
    };                                                                          \
                                                                                \
    start(new T(func, arg1, arg2, depends), priority);                          \
  }

LOOPTYPES2(RUNSTATICFUNC2)


#define RUNSTATICFUNC3(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1, typename _type2, typename _type3>                   \
  inline void queue(void(* func)(A1, A2, A3),                                   \
           A1 arg1, A2 arg2, A3 arg3,                                           \
           SDependency *depends = NULL, int priority = 0)                       \
  {                                                                             \
    struct T : SRunnable                                                        \
    {                                                                           \
      inline T(void(* func)(A1, A2, A3),                                        \
               A1 arg1, A2 arg2, A3 arg3,                                       \
               SDependency *depends)                                            \
        : SRunnable(depends, typeid(func).name()), func(func),                  \
          arg1(arg1), arg2(arg2), arg3(arg3) { }                                \
                                                                                \
      virtual void run(SThreadPool *) { func(arg1, arg2, arg3); }               \
                                                                                \
      void(* const func)(A1, A2, A3);                                           \
      C1 arg1; C2 arg2; C3 arg3;                                                \
    };                                                                          \
                                                                                \
    start(new T(func, arg1, arg2, arg3, depends), priority);                    \
  }

LOOPTYPES3(RUNSTATICFUNC3)


#define RUNSTATICFUNC4(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<typename _type1, typename _type2, typename _type3, typename _type4>  \
  inline void queue(void(* func)(A1, A2, A3, A4),                               \
           A1 arg1, A2 arg2, A3 arg3, A4 arg4,                                  \
           SDependency *depends = NULL, int priority = 0)                       \
  {                                                                             \
    struct T : SRunnable                                                        \
    {                                                                           \
      inline T(void(* func)(A1, A2, A3, A4),                                    \
               A1 arg1, A2 arg2, A3 arg3, A4 arg4,                              \
               SDependency *depends)                                            \
        : SRunnable(depends, typeid(func).name()), func(func),                  \
          arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4) { }                    \
                                                                                \
      virtual void run(SThreadPool *) { func(arg1, arg2, arg3, arg4); }         \
                                                                                \
      void(* const func)(A1, A2, A3, A4);                                       \
      C1 arg1; C2 arg2; C3 arg3; C4 arg4;                                       \
    };                                                                          \
                                                                                \
    start(new T(func, arg1, arg2, arg3, arg4, depends), priority);              \
  }

LOOPTYPES4(RUNSTATICFUNC4)

#undef RUNSTATICFUNC1
#undef RUNSTATICFUNC2
#undef RUNSTATICFUNC3
#undef RUNSTATICFUNC4


#define RUNMEMBERFUNC1(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1>                                        \
  inline void queue(_base *object, void(_base::* func)(A1),                     \
           A1 arg1,                                                             \
           SDependency *depends = NULL, int priority = 0)                       \
  {                                                                             \
    struct T : SRunnable                                                        \
    {                                                                           \
      inline T(_base *object, void(_base::* func)(A1),                          \
               A1 arg1,                                                         \
               SDependency *depends)                                            \
        : SRunnable(depends, typeid(func).name()), object(object), func(func),  \
          arg1(arg1) { }                                                        \
                                                                                \
      virtual void run(SThreadPool *) { (object->*func)(arg1); }                \
                                                                                \
      _base * const object;                                                     \
      void(_base::* const func)(A1);                                            \
      C1 arg1;                                                                  \
    };                                                                          \
                                                                                \
    start(new T(object, func, arg1, depends), priority);                        \
  }

LOOPTYPES1(RUNMEMBERFUNC1)


#define RUNMEMBERFUNC2(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2>                       \
  inline void queue(_base *object, void(_base::* func)(A1, A2),                 \
           A1 arg1, A2 arg2,                                                    \
           SDependency *depends = NULL, int priority = 0)                       \
  {                                                                             \
    struct T : SRunnable                                                        \
    {                                                                           \
      inline T(_base *object, void(_base::* func)(A1, A2),                      \
               A1 arg1, A2 arg2,                                                \
               SDependency *depends)                                            \
        : SRunnable(depends, typeid(func).name()), object(object), func(func),  \
          arg1(arg1), arg2(arg2) { }                                            \
                                                                                \
      virtual void run(SThreadPool *) { (object->*func)(arg1, arg2); }          \
                                                                                \
      _base * const object;                                                     \
      void(_base::* const func)(A1, A2);                                        \
      C1 arg1; C2 arg2;                                                         \
    };                                                                          \
                                                                                \
    start(new T(object, func, arg1, arg2, depends), priority);                  \
  }

LOOPTYPES2(RUNMEMBERFUNC2)


#define RUNMEMBERFUNC3(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2, typename _type3>      \
  inline void queue(_base *object, void(_base::* func)(A1, A2, A3),             \
           A1 arg1, A2 arg2, A3 arg3,                                           \
           SDependency *depends = NULL, int priority = 0)                       \
  {                                                                             \
    struct T : SRunnable                                                        \
    {                                                                           \
      inline T(_base *object, void(_base::* func)(A1, A2, A3),                  \
               A1 arg1, A2 arg2, A3 arg3,                                       \
               SDependency *depends)                                            \
        : SRunnable(depends, typeid(func).name()), object(object), func(func),  \
          arg1(arg1), arg2(arg2), arg3(arg3) { }                                \
                                                                                \
      virtual void run(SThreadPool *) { (object->*func)(arg1, arg2, arg3); }    \
                                                                                \
      _base * const object;                                                     \
      void(_base::* const func)(A1, A2, A3);                                    \
      C1 arg1; C2 arg2; C3 arg3;                                                \
    };                                                                          \
                                                                                \
    start(new T(object, func, arg1, arg2, arg3, depends), priority);            \
  }

LOOPTYPES3(RUNMEMBERFUNC3)


#define RUNMEMBERFUNC4(A1, C1, A2, C2, A3, C3, A4, C4) \
  template<class _base, typename _type1, typename _type2, typename _type3, typename _type4> \
  inline void queue(_base *object, void(_base::* func)(A1, A2, A3, A4),         \
           A1 arg1, A2 arg2, A3 arg3, A4 arg4,                                  \
           SDependency *depends = NULL, int priority = 0)                       \
  {                                                                             \
    struct T : SRunnable                                                        \
    {                                                                           \
      inline T(_base *object, void(_base::* func)(A1, A2, A3, A4),              \
               A1 arg1, A2 arg2, A3 arg3, A4 arg4,                              \
               SDependency *depends)                                            \
        : SRunnable(depends, typeid(func).name()), object(object), func(func),  \
          arg1(arg1), arg2(arg2), arg3(arg3), arg4(arg4) { }                    \
                                                                                \
      virtual void run(SThreadPool *) { (object->*func)(arg1, arg2, arg3, arg4); } \
                                                                                \
      _base * const object;                                                     \
      void(_base::* const func)(A1, A2, A3, A4);                                \
      C1 arg1; C2 arg2; C3 arg3; C4 arg4;                                       \
    };                                                                          \
                                                                                \
    start(new T(object, func, arg1, arg2, arg3, arg4, depends), priority);      \
  }

LOOPTYPES4(RUNMEMBERFUNC4)

#undef RUNMEMBERFUNC1
#undef RUNMEMBERFUNC2
#undef RUNMEMBERFUNC3
#undef RUNMEMBERFUNC4

#undef LOOPTYPES1
#undef LOOPTYPES2
#undef LOOPTYPES3
#undef LOOPTYPES4
