/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

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
    bool                        inilialized;
    Initializer               * next;
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
  /*! Initializes all LXiMedia components.

      \note Only one active instance is allowed at any time.
      \param useLogFile         When true a log file is created.
      \param skipModules        A list of modules for which no plugins should be
                                loaded.
      \param parent             The parent QObject.
   */
  explicit                      SApplication(bool useLogFile = false, const QStringList &skipModules = QStringList(), QObject * = NULL);

  /*! Uninitializes all LXiMedia components.

      \note Only one active instance is allowed at any time.
   */
  virtual                       ~SApplication();

  /*! Returns a name string for LXiMedia.
   */
  static const char           * name(void);

  /*! Returns the version identifier for the active build of LXiStream.
   */
  static const char           * version(void);

  /*! Returns a pointer to the active SApplication instance or NULL if none
      exists.

      \note Only one active instance is allowed at any time.
   */
  static SApplication         * instance(void);

  /*! Returns the base filename fo be used for temporary files, excluding the
      path to the temporary dir.
   */
  static QString                tempFileBase(void);

  /*! Returns the paths that are searched for plugins.
   */
  static QStringList            pluginPaths(void);

  /*! Adds a module filter string whoich is use to determine which of the plugins
      in the plugin directory is actually loaded.

      \note This method should rarely be useful.
   */
  void                          addModuleFilter(const QString &);

  /*! Loads a module.

      \note This method should rarely be useful.
   */
  bool                          loadModule(const QString &);

  /*! Loads a module.

      \note This method should rarely be useful.
   */
  bool                          loadModule(SModule *, QPluginLoader * = NULL);

  QMap<QString, SModule *>      modules(void) const;

  /*! Returns the about text with minimal XHTML markup.
   */
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
  /*! Creates a new SApplication instance for use within a QTest environment. No
      modules will be loaded automatically and they have to be loaded using
      loadModule().
   */
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
