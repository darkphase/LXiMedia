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

#include "sapplication.h"
#include "sdebug.h"
#include "saudiobuffer.h"
#include "sencodedaudiobuffer.h"
#include "sencodeddatabuffer.h"
#include "sencodedvideobuffer.h"
#include "ssubpicturebuffer.h"
#include "ssubtitlebuffer.h"
#include "svideobuffer.h"

#include "private/exceptionhandler.h"
#include "private/log.h"
#include "common/module.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace LXiStream {

const SApplication::InitializeFlag SApplication::Initialize_Default =
    SApplication::InitializeFlag(
      int(SApplication::Initialize_Modules) |
      int(SApplication::Initialize_Devices) |
      int(SApplication::Initialize_LogToFile) |
      int(SApplication::Initialize_LogToConsole) |
      int(SApplication::Initialize_HandleFaults));

SApplication * SApplication::self = NULL;

/*! This is the main initialization for LXiStream. When invoked without
    arguments; it tries to load all available plugins. If a specific module is
    provided; it will only load this module, the provided mopdule object may be
    destroyed after this method returns. This method may be invoked multiple
    times if needed.

    \returns True if initialization succeeded.
    \sa shutdown()
 */
SApplication::SApplication(InitializeFlags flags, const QString &preferredLogDir)
  : flags(flags)
{
  if (self != NULL)
    qFatal("Only one instance of the SApplication class is allowed.");

  self = this;

  static bool firsttime = true;
  if (firsttime)
  {
    firsttime = false;

    // Ensure static initializers have been initialized.
    SDebug::Locker::initialize();

    // Register metatypes.
    qRegisterMetaType<SEncodedAudioBuffer>("SEncodedAudioBuffer");
    qRegisterMetaType<SAudioBuffer>("SAudioBuffer");
    qRegisterMetaType<SEncodedVideoBuffer>("SEncodedVideoBuffer");
    qRegisterMetaType<SVideoBuffer>("SVideoBuffer");
    qRegisterMetaType<SEncodedDataBuffer>("SEncodedDataBuffer");
    qRegisterMetaType<SSubpictureBuffer>("SSubpictureBuffer");
    qRegisterMetaType<SSubtitleBuffer>("SSubtitleBuffer");

    qRegisterMetaType<SAudioCodec>("SAudioCodec");
    qRegisterMetaType<SVideoCodec>("SVideoCodec");
    qRegisterMetaType<SDataCodec>("SDataCodec");
  }

  Private::Log::initialize(preferredLogDir);

  if ((flags & Initialize_HandleFaults) == Initialize_HandleFaults)
    Private::ExceptionHandler::initialize();

  // Always load the Common module.
  loadModule<LXiStream::Common::Module>();

  if ((flags & Initialize_Modules) == Initialize_Modules)
    loadModule(NULL); // Loads all modules
}

/*! \fn void SApplication::shutdown(void)
    This waits for all graphs to finish and cleans up any terminals allocated by
    LXiStream.

    \note Do not invoke initialize() again after shutting down.
 */
SApplication::~SApplication(void)
{
  Q_ASSERT(QThread::currentThread() == thread());

  // Wait for all tasks to finish
  SScheduler::waitForDone();

  foreach (SFactory *factory, factories())
    factory->clear();

  foreach (const Module &module, moduleList)
  {
    module.module->unload();
    if (module.loader)
    {
      module.loader->unload();
      delete module.loader;
    }
    else
      delete module.module;
  }

  moduleList.clear();

  self = NULL;
}

/*! Returns a name string for LXiStream.
 */
const char * SApplication::name(void)
{
  return "LXiStream";
}

/*! Returns the version identifier for the active build of LXiStream.
 */
const char * SApplication::version(void)
{
  return
#include "_version.h"
      ;
}

/*! When invoked with NULL; it will attempt to load all available plugins. If a
    specific module is provided; it will only load this module. The ownership of
    the provided module will be transferred to SApplication, the module will be
    deleted when shutdown() is invoked.

    \returns True if loading succeeded.
 */
bool SApplication::loadModule(SInterfaces::Module *module)
{
  Q_ASSERT(QThread::currentThread() == thread());

  static bool probedPlugins = false;

  if (module)
  {
    module->registerClasses();
    moduleList += Module(NULL, module);
  }
  else if (!probedPlugins)
  {
    // And now load the plugins
    QStringList paths;
#ifndef Q_OS_WIN
    paths += QCoreApplication::applicationDirPath() + "/liblxistream/";
    paths += "/usr/lib/liblxistream/";
    paths += "/usr/local/lib/liblxistream/";
#else
    const QByteArray myDll = "LXiStream" + QByteArray(version()).split('.').first() + ".dll";
    HMODULE myModule = ::GetModuleHandleA(myDll.data());
    char fileName[MAX_PATH];
    if (::GetModuleFileNameA(myModule, fileName, MAX_PATH) > 0)
    {
      QByteArray path = fileName;
      path = path.left(path.lastIndexOf('\\') + 1);
      paths += path + "liblxistream\\";
    }
    else
      qCritical("Failed to locate %s", myDll.data());
#endif

    QSet<QString> modules;
    foreach (QDir dir, paths)
#ifndef Q_OS_WIN
    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QDir::Files))
#else
    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QStringList() << "*.dll", QDir::Files))
#endif
    if (!modules.contains(fileInfo.fileName()))
    {
      modules.insert(fileInfo.fileName());

      QPluginLoader *loader = new QPluginLoader(fileInfo.absoluteFilePath());
      SInterfaces::Module * const module = qobject_cast<SInterfaces::Module *>(loader->instance());
      if (module)
      {
        module->registerClasses();
        moduleList += Module(loader, module);
      }
      else
      {
        qWarning() << "Error loading plugin" << fileInfo.fileName()
                   << ": " << loader->errorString();

        loader->unload();
        delete loader;
      }
    }

    probedPlugins = true;
  }

  return true;
}

/*! Returns the about text with minimal XHTML markup.
 */
QByteArray SApplication::about(void) const
{
  QByteArray text =
      " <h1>" + QByteArray(name()) + "</h1>\n"
      " Version: " + QByteArray(version()) + "<br />\n"
      " Website: <a href=\"http://lximedia.sourceforge.net/\">lximedia.sourceforge.net</a><br />\n"
      " <br />\n"
      " <b>Copyright &copy; 2009-2010 A.J. Admiraal. All rights reserved.</b><br />\n"
      " <br />\n"
      " This program is free software; you can redistribute it and/or modify it\n"
      " under the terms of the GNU General Public License version 2 as published\n"
      " by the Free Software Foundation.<br />\n"
      " <br />\n"
      " This program is distributed in the hope that it will be useful, but WITHOUT\n"
      " ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
      " FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.<br />\n"
      " <br />\n";

  foreach (const SApplication::Module &module, moduleList)
    text += module.module->about();

  return text;
}

void SApplication::customEvent(QEvent *e)
{
  if (e->type() == scheduleEventType)
    schedule(static_cast<ScheduleEvent *>(e)->depends);
  else
    return QObject::customEvent(e);
}

void SApplication::queueSchedule(Dependency *depends)
{
  QCoreApplication::postEvent(this, new ScheduleEvent(depends));
}

QList<SFactory *> & SApplication::factories(void)
{
  Q_ASSERT((self == NULL) || (QThread::currentThread() == self->thread()));

  static QList<SFactory *> l;

  return l;
}

} // End of namespace


