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

#include "ssystem.h"
#include "sdebug.h"
#include "sterminal.h"
#include "saudiobuffer.h"
#include "sencodedaudiobuffer.h"
#include "sencodeddatabuffer.h"
#include "sencodedvideobuffer.h"
#include "ssubtitlebuffer.h"
#include "svideobuffer.h"

#include "private/exceptionhandler.h"
#include "private/log.h"
#include "common/module.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace LXiStream {

const SSystem::InitializeFlag SSystem::Initialize_Default =
    SSystem::InitializeFlag(
      int(SSystem::Initialize_Modules) |
      int(SSystem::Initialize_Devices) |
      int(SSystem::Initialize_LogToFile) |
      int(SSystem::Initialize_LogToConsole) |
      int(SSystem::Initialize_HandleFaults));

SSystem::InitializeFlags  SSystem::flags;

/*! Returns a name string for LXiStream.
 */
const char * SSystem::name(void)
{
  return "LXiStream";
}

/*! Returns the version identifier for the active build of LXiStream.
 */
const char * SSystem::version(void)
{
  return
#include "version.h"
      " (" __DATE__ " " __TIME__")";
}

/*! This is the main initialization method for LXiStream. When invoked without
    arguments; it tries to load all available plugins. If a specific module is
    provided; it will only load this module, the provided mopdule object may be
    destroyed after this method returns. This method may be invoked multiple
    times if needed.

    \returns True if initialization succeeded.
    \sa shutdown()
 */
bool SSystem::initialize(InitializeFlags f, const QString &preferredLogDir)
{
  // Ensure static initializers have been initialized.
  SDebug::Locker::initialize();

  flags = f;

  Private::Log::initialize(preferredLogDir);

  if ((flags & Initialize_HandleFaults) == Initialize_HandleFaults)
    Private::ExceptionHandler::initialize();

  // Register metatypes.
  qRegisterMetaType<SEncodedAudioBuffer>("SEncodedAudioBuffer");
  qRegisterMetaType<SAudioBuffer>("SAudioBuffer");
  qRegisterMetaType<SEncodedVideoBuffer>("SEncodedVideoBuffer");
  qRegisterMetaType<SVideoBuffer>("SVideoBuffer");
  qRegisterMetaType<SEncodedDataBuffer>("SEncodedDataBuffer");
  qRegisterMetaType<SSubtitleBuffer>("SSubtitleBuffer");

  qRegisterMetaType<SAudioCodec>("SAudioCodec");
  qRegisterMetaType<SVideoCodec>("SVideoCodec");
  qRegisterMetaType<SDataCodec>("SDataCodec");

  // Always load the Common module.
  loadModule<LXiStream::Common::Module>();

  if ((flags & Initialize_Modules) == Initialize_Modules)
    loadModule(NULL); // Loads all modules

  return true;
}

/*! \fn void SSystem::shutdown(void)
    This waits for all graphs to finish and cleans up any terminals allocated by
    LXiStream.

    \note Do not invoke initialize() again after shutting down.
 */
void SSystem::shutdown(void)
{
  foreach (const Module &module, moduleList())
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

  moduleList().clear();
}

/*! When invoked with NULL; it will attempt to load all available plugins. If a
    specific module is provided; it will only load this module. The ownership of
    the provided module will be transferred to SSystem, the module will be
    deleted when shutdown() is invoked.

    \returns True if loading succeeded.
 */
bool SSystem::loadModule(SInterfaces::Module *module)
{
  static bool probedPlugins = false;

  if (module)
  {
    module->registerClasses();
    moduleList() += Module(NULL, module);
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
        moduleList() += Module(loader, module);
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

QList<SSystem::Module> & SSystem::moduleList(void)
{
  static QList<Module> m;

  return m;
}

/*! Returns the about text with minimal XHTML markup.
 */
QByteArray SSystem::about(void)
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

  foreach (const SSystem::Module &module, moduleList())
    text += module.module->about();

  return text;
}



} // End of namespace


