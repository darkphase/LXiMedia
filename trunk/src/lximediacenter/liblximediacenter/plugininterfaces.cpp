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

#include "plugininterfaces.h"

#include "globalsettings.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace LXiMediaCenter {

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
}

void Plugin::setPluginDate(const QDateTime &)
{
}

QStringList Plugin::getPluginPaths(void)
{
  QStringList paths;
#ifndef Q_OS_WIN
  paths += QCoreApplication::applicationDirPath() + "/lximediacenter/";
  paths += "/usr/lib/lximediacenter/";
  paths += "/usr/local/lib/lximediacenter/";
#else
  const QByteArray myDll = "LXiMediaCenter" + QByteArray(GlobalSettings::version()).split('.').first() + ".dll";
  HMODULE myModule = ::GetModuleHandleA(myDll.data());
  char fileName[MAX_PATH];
  if (::GetModuleFileNameA(myModule, fileName, MAX_PATH) > 0)
  {
    QByteArray path = fileName;
    path = path.left(path.lastIndexOf('\\') + 1);
    paths += path + "lximediacenter\\";
  }
  else
    qCritical("Failed to locate %s", myDll.data());
#endif

  return paths;
}

QList<Plugin *> Plugin::loadPlugins(void)
{
  QList<Plugin *> plugins;

  QSet<QString> modules;
  foreach (QDir dir, getPluginPaths())
  {
#ifndef Q_OS_WIN
    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QDir::Files))
#else
    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QStringList() << "*.dll", QDir::Files))
#endif
    if (!modules.contains(fileInfo.fileName()))
    {
      modules.insert(fileInfo.fileName());

      QPluginLoader loader(fileInfo.absoluteFilePath());
      Plugin * const plugin = qobject_cast<Plugin *>(loader.instance());

      if (plugin)
      {
        plugin->setPluginDate(fileInfo.lastModified());
        plugins.append(plugin);
      }
      else
        qWarning() << "Error loading plugin" << fileInfo.fileName()
                   << ": " << loader.errorString();
    }
  }

  return plugins;
}

QList<BackendPlugin *> BackendPlugin::loadPlugins()
{
  QList<BackendPlugin *> result;
  foreach (Plugin *plugin, Plugin::loadPlugins())
    result += qobject_cast<BackendPlugin *>(plugin);

  return result;
}

BackendPlugin::~BackendPlugin()
{
}

} // End of namespace
