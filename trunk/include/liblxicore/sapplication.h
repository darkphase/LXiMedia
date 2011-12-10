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
#include "export.h"

#define sApp (::LXiCore::SApplication::instance())

#if defined(__GNUC__)
# define LXI_PROFILE_WAIT(x) do { ::LXiCore::SApplication::Profiler _prf(__PRETTY_FUNCTION__, #x); x; } while(false)
# define LXI_PROFILE_FUNCTION(t) ::LXiCore::SApplication::Profiler _prf(__PRETTY_FUNCTION__, ::LXiCore::SApplication:: t)
#elif defined(_MSC_VER)
# define LXI_PROFILE_WAIT(x) do { ::LXiCore::SApplication::Profiler _prf(__FUNCSIG__, #x); x; } while(false)
# define LXI_PROFILE_FUNCTION(t) ::LXiCore::SApplication::Profiler _prf(__FUNCSIG__, ::LXiCore::SApplication:: t)
#else
# define LXI_PROFILE_WAIT(x) do { ::LXiCore::SApplication::Profiler _prf(__FUNCTION__, #x); x; } while(false)
# define LXI_PROFILE_FUNCTION(t) ::LXiCore::SApplication::Profiler _prf(__FUNCTION__, ::LXiCore::SApplication:: t)
#endif

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

    The SApplication instance will also provide a logging system, everyting
    logged with qDebug(), qWarning(), qCritical() and qFatal() will be witten to
    a log in memory.
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

  /*! Enumerates different task types for use with profiling. They determine the
      color of the block in the profile.
   */
  enum TaskType
  {
    TaskType_Blocked,
    TaskType_AudioProcessing,
    TaskType_VideoProcessing,
    TaskType_MiscProcessing
  };

  /*! A profiler helper class that profiles the creation and destruction times
      of the object, much like a QMutexLocker.
   */
  class LXICORE_PUBLIC Profiler
  {
  public:
    explicit                    Profiler(const char *taskName, TaskType);
                                Profiler(const char *taskName, const char *waitName, TaskType = TaskType_Blocked);
                                ~Profiler();

  private:
    const char          * const taskName;
    const char          * const waitName;
    const TaskType              taskType;
    const int                   startTime;
  };

public:
  explicit                      SApplication(bool useLogFile = false, const QStringList &skipModules = QStringList(), QObject * = NULL);
  virtual                       ~SApplication();

  static const char           * name(void);
  static const char           * version(void);
  static SApplication         * instance(void);

  static QStringList            pluginPaths(void);
  void                          addModuleFilter(const QString &);
  bool                          loadModule(const QString &);
  bool                          loadModule(SModule *, QPluginLoader * = NULL);
  QMap<QString, SModule *>      modules(void) const;
  QByteArray                    about(void) const;

public: // Logging
  QByteArray                    log(void);
  void                          logLine(const QByteArray &line);

private:
  static void                   logMessage(QtMsgType, const char *);

public: // Profiling
  bool                          enableProfiling(const QString &fileName);
  void                          disableProfiling(void);
  int                           profileTimeStamp(void);
  void                          profileTask(int, int, const QByteArray &, TaskType);

public:
  static SApplication         * createForQTest(QObject *);

private:
  explicit                      SApplication(QObject *);

  static QList<SFactory *>    & factories(void);

private:
  static Initializer          * initializers;
  static SApplication         * self;

  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
