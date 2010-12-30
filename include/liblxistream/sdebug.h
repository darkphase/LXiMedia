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

#ifndef LXSTREAM_SDEBUG_H
#define LXSTREAM_SDEBUG_H

#include <QtCore>
#include "stime.h"
#include "stimer.h"

namespace LXiStream {
namespace SDebug {

/*! The exception handler can provide a stack frame pointer.
 */
class ExceptionHandler
{
public:
  struct StackFrame
  {
    StackFrame(void) : stackPointer(NULL), framePointer(NULL)  { }

    void                      * stackPointer;
    void                      * framePointer;
  };

public:
  inline static StackFrame      currentStackFrame(void);
};


/*! A log file contains all messages logged by qDebug(), qWarning(), qCritical()
    and qFatal().
 */
class LogFile : public QFile
{
public:
  struct Message
  {
    QDateTime                   date;
    QString                     type;
    unsigned                    pid;
    unsigned                    tid;
    QString                     headline;
    QString                     message;
  };

public:
  explicit                      LogFile(const QString &, QObject * = NULL);
                                ~LogFile();

  Message                       readMessage(void);

public:
  static QStringList            errorLogFiles(void);
  static QStringList            allLogFiles(void);
  static QString                activeLogFile(void);
};


class Locker
{
friend class SSystem;
private:
  struct LockDesc
  {
    inline LockDesc(void)
        : stackFrame(),
          file(""),
          line(0),
          type(nullType),
          locked(false)
    {
    }

    inline LockDesc(const ExceptionHandler::StackFrame &stackFrame,
                    const char * file,
                    unsigned line,
                    const char * type,
                    bool locked)
        : stackFrame(stackFrame),
          file(file),
          line(line),
          type(type),
          locked(locked)
    {
    }

    ExceptionHandler::StackFrame stackFrame;
    const char                * file;
    unsigned                    line;
    const char                * type;
    bool                        locked;
  };

  typedef QMap<QThread *, QStack<LockDesc> > ThreadMap;
  typedef QMap<void *, ThreadMap> LockMap;

public:
  static void                   initialize(void);

protected:
  static void                   attemptLock(void *, const char *, const char *, unsigned);
  static void                   gotLock(void *);
  static void                   releasedLock(void *);

  static void                   logPotentialDeadlock(void *, const char *, unsigned);

private:
  static QMutex               & mutex(void);
  static LockMap              & lockMap(void) __attribute__((pure));
  static QTime                & lastLog(void) __attribute__((pure));

protected:
  static const int              maxLockTime;
  static const char     * const nullType;
};


class MutexLocker : private Locker
{
public:
  inline explicit MutexLocker(QMutex *m, const char *file, unsigned line)
      : mtx(m)
  {
    Q_ASSERT_X((val & quintptr(1u)) == quintptr(0),
               type, "QMutex pointer is misaligned");
    relock(file, line);
  }

  inline ~MutexLocker()
  {
    unlock();
  }

  inline void unlock()
  {
    if (mtx)
    {
      if ((val & quintptr(1u)) == quintptr(1u))
      {
        val &= ~quintptr(1u);
        mtx->unlock();
        releasedLock(mtx);
      }
    }
  }

  inline void relock(const char *file, unsigned line)
  {
    if (mtx)
    {
      if ((val & quintptr(1u)) == quintptr(0u))
      {
        attemptLock(mtx, type, file, line);

        if (!mtx->tryLock(maxLockTime))
        {
          logPotentialDeadlock(mtx, file, line);
          mtx->lock();
        }

        gotLock(mtx);
        val |= quintptr(1u);
      }
    }
  }

  inline QMutex *mutex() const
  {
    return reinterpret_cast<QMutex *>(val & ~quintptr(1u));
  }

private:
  Q_DISABLE_COPY(MutexLocker)

  static const char * const type;

  union
  {
    QMutex *mtx;
    quintptr val;
  };
};


class ReadLocker : private Locker
{
public:
  inline ReadLocker(QReadWriteLock *areadWriteLock, const char *file, unsigned line)
    : q_lock(areadWriteLock)
  {
    Q_ASSERT_X((q_val & quintptr(1u)) == quintptr(0),
               type, "QReadWriteLock pointer is misaligned");
    relock(file, line);
  }

  inline ~ReadLocker()
  {
    unlock();
  }

  inline void unlock()
  {
    if (q_lock)
    {
      if ((q_val & quintptr(1u)) == quintptr(1u))
      {
        q_val &= ~quintptr(1u);
        q_lock->unlock();
        releasedLock(q_lock);
      }
    }
  }

  inline void relock(const char *file, unsigned line)
  {
    if (q_lock)
    {
      if ((q_val & quintptr(1u)) == quintptr(0u))
      {
        attemptLock(q_lock, type, file, line);

        if (!q_lock->tryLockForRead(maxLockTime))
        {
          logPotentialDeadlock(q_lock, file, line);
          q_lock->lockForRead();
        }

        gotLock(q_lock);
        q_val |= quintptr(1u);
      }
    }
  }

  inline QReadWriteLock *readWriteLock() const
  { return reinterpret_cast<QReadWriteLock *>(q_val & ~quintptr(1u)); }

private:
  Q_DISABLE_COPY(ReadLocker)

  static const char * const type;

  union
  {
    QReadWriteLock *q_lock;
    quintptr q_val;
  };
};


class WriteLocker : private Locker
{
public:
  inline WriteLocker(QReadWriteLock *areadWriteLock, const char *file, unsigned line)
    : q_lock(areadWriteLock)
  {
    Q_ASSERT_X((q_val & quintptr(1u)) == quintptr(0),
               type, "QReadWriteLock pointer is misaligned");
    relock(file, line);
  }

  inline ~WriteLocker()
  {
    unlock();
  }

  inline void unlock()
  {
    if (q_lock)
    {
      if ((q_val & quintptr(1u)) == quintptr(1u))
      {
        q_val &= ~quintptr(1u);
        q_lock->unlock();
        releasedLock(q_lock);
      }
    }
  }

  inline void relock(const char *file, unsigned line)
  {
    if (q_lock)
    {
      if ((q_val & quintptr(1u)) == quintptr(0u))
      {
        attemptLock(q_lock, type, file, line);

        if (!q_lock->tryLockForWrite(maxLockTime))
        {
          logPotentialDeadlock(q_lock, file, line);
          q_lock->lockForWrite();
        }

        gotLock(q_lock);
        q_val |= quintptr(1u);
      }
    }
  }

  inline QReadWriteLock *readWriteLock() const
  {
    return reinterpret_cast<QReadWriteLock *>(q_val & ~quintptr(1u));
  }

private:
  Q_DISABLE_COPY(WriteLocker)

  static const char * const type;

  union
  {
    QReadWriteLock *q_lock;
    quintptr q_val;
  };
};


inline ExceptionHandler::StackFrame ExceptionHandler::currentStackFrame(void)
{
  StackFrame stackFrame;

#if (defined(QT_ARCH_I386) || defined(QT_ARCH_WINDOWS))
  asm volatile("movl %%esp, %0;"
               "movl %%ebp, %1;"
               : "=r"(stackFrame.stackPointer),
                 "=r"(stackFrame.framePointer));
#elif defined(QT_ARCH_X86_64)
  asm volatile("movq %%rsp, %0;"
               "movq %%rbp, %1;"
               : "=r"(stackFrame.stackPointer),
                 "=r"(stackFrame.framePointer));
#else
#error "Not implemented"
#endif

  return stackFrame;
}

} } // End of namespaces

#endif
