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

#ifndef LXICORE_SAPPLICATION_H
#define LXICORE_SAPPLICATION_H

#include <QtCore>
#include "splatform.h"
#include "sscheduler.h"
#include "export.h"

#define sApp (SApplication::instance())

namespace LXiCore {

class SFactory;
class SModule;

/*! The SApplication class initializes LXiMedia. An instance of SApplication is
    required to use most of the LXiMedia classes (unless documented otherwise).
    Once the instance is destroyed LXiMedia is uninitialized.
 */
class LXICORE_PUBLIC SApplication : public QObject
{
Q_OBJECT
friend class SFactory;
public:
  class LXICORE_PUBLIC Initializer
  {
  friend class SApplication;
  protected:
                                Initializer(void);
    /* Deliberately not virtual */ ~Initializer();

    virtual void                startup(void) = 0;
    virtual void                shutdown(void) = 0;

  private:
    Initializer               * next;
  };

  struct StackFrame
  {
    inline StackFrame(void) : stackPointer(NULL), framePointer(NULL)  { }

    void                      * stackPointer;
    void                      * framePointer;
  };

  class LXICORE_PUBLIC LogFile : public QFile
  {
  public:
    struct Message
    {
      QDateTime                 date;
      QString                   type;
      unsigned                  pid;
      unsigned                  tid;
      QString                   headline;
      QString                   message;
    };

  public:
    explicit                    LogFile(const QString &, QObject * = NULL);

    Message                     readMessage(void);
  };

public:
  explicit                      SApplication(const QStringList &moduleFilter = QStringList(), const QString &logDir = QString::null, QObject * = NULL);
  virtual                       ~SApplication();

  __pure static const char    * name(void);
  __pure static const char    * version(void);
  __pure static SApplication  * instance(void);

  bool                          loadModule(SModule *, QPluginLoader * = NULL);
  QByteArray                    about(void) const;

public: // Exception handler
  static void                   installExcpetionHandler(void);

  inline static StackFrame      currentStackFrame(void);
  static void                   logStackFrame(const StackFrame &);
  inline static void            logStackFrame(void)                             { return logStackFrame(currentStackFrame()); }

public: // Logging
  const QString               & logDir(void) const;
  const QStringList           & errorLogFiles(void) const;
  const QStringList           & allLogFiles(void) const;
  const QString               & activeLogFile(void) const;
  static void                   logLineToActiveLogFile(const QString &line);

public:
  static SApplication         * createForQTest(QObject *);

private:
  __internal explicit           SApplication(QObject *);

  __internal static QList<SFactory *> & factories(void);

private:
  __internal static Initializer * initializers;
  __internal static SApplication * self;

  struct Data;
  Data                  * const d;
};


inline SApplication::StackFrame SApplication::currentStackFrame(void)
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

} // End of namespace

#endif
