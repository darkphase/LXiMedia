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
#include "export.h"

#define sApp (SApplication::instance())

namespace LXiCore {

class SFactory;
class SModule;

/*! The SApplication class initializes LXiMedia. An instance of SApplication is
    required to use most of the LXiMedia classes (unless documented otherwise).
    Once the instance is destroyed LXiMedia is uninitialized.

    The constructor of SApplication will automatically load all relevant modules
    from plugin files. On Unix, these plugin files have to be located in either
    /usr/lib/lximedia0/ or /usr/local/lib/lximedia0/. On Windows these files
    need to be located in a directory called "lximedia" that is located in the
    directory that contains LXiCore0.dll.

    The SApplication instance will also provide a logging system if a log
    directory is provided to its constructor. Everyting logged with qDebug(),
    qWarning(), qCritical() and qFatal() will be witten to a log file in that
    directory.
 */
class LXICORE_PUBLIC SApplication : public QObject
{
Q_OBJECT
friend class SFactory;
public:
  /*! This initializer can be used to perform custom actions on creation and
      destruction of an SApplication. To use it, create a subclass of
      SApplication::Initializer and implement the startup() and shutdown()
      methods and create a static instance of the subclass.

      \code
        class MyInitializer : SApplication::Initializer
        {
          virtual void startup(void)
          {
            // Startup actions ...
          }

          virtual void shutdown(void)
          {
            // Shutdown actions ...
          }

          static MyInitializer instance;
        }

        MyInitializer MyInitializer::instance;
      \endcode
   */
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

  /*! This is a helper class to read log messages from a log file. It can be
      used as a QFile, read messages from the file with the readMessage()
      method, similar to QFile::readLine().
   */
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
  explicit                      SApplication(const QString &logDir = QString::null, QObject * = NULL);
  virtual                       ~SApplication();

  _lxi_pure static const char * name(void);
  _lxi_pure static const char * version(void);
  _lxi_pure static SApplication * instance(void);

  void                          addModuleFilter(const QString &);
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
  _lxi_internal explicit        SApplication(QObject *);

  _lxi_internal static QList<SFactory *> & factories(void);

private:
  _lxi_internal static Initializer * initializers;
  _lxi_internal static SApplication * self;

  struct Data;
  Data                  * const d;
};


/*! Returns a stack-frame pointer for the calling function. This stack-frame
    pointer can then be used with the SApplication::logStackFrame() method.
 */
inline SApplication::StackFrame SApplication::currentStackFrame(void)
{
  StackFrame stackFrame;

#if ((defined(QT_ARCH_I386) || defined(QT_ARCH_WINDOWS)) && defined(__GNUC__))
  asm volatile("movl %%esp, %0;"
               "movl %%ebp, %1;"
               : "=r"(stackFrame.stackPointer),
                 "=r"(stackFrame.framePointer));
#elif (defined(QT_ARCH_X86_64) && defined(__GNUC__))
  asm volatile("movq %%rsp, %0;"
               "movq %%rbp, %1;"
               : "=r"(stackFrame.stackPointer),
                 "=r"(stackFrame.framePointer));
#elif ((defined(QT_ARCH_I386) || defined(QT_ARCH_WINDOWS)) && defined(_MSC_VER))
  __asm {
    mov stackFrame.stackPointer, esp
    mov stackFrame.framePointer, ebp
  }
#else
#error "Not implemented"
#endif

  return stackFrame;
}

} // End of namespace

#endif
