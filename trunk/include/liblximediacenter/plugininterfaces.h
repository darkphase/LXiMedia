/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef LXMEDIACENTER_PLUGININTERFACES_H
#define LXMEDIACENTER_PLUGININTERFACES_H

#include <QtCore>
#include <QtPlugin>
#include "backendserver.h"

class QGLWidget;
class QTabWidget;

namespace LXiMediaCenter {

/*! This is the main plugin interface, every plugin implements this. This
    interface is not directly implemented by any plugin though, the
    subinterfaces FrontendPlugin, BackendPlugin, ConfigurationPlugin and
    FrontendTheme should be implemented. Note that many plugins implement
    multiple interfaces at once (usually backend, frontend and configuration).
    Therefore subinterfaces should always inherit "virtual" from this one.
 */
class Plugin : public QObject
{
Q_OBJECT
protected:
  static QStringList            getPluginPaths(void);                           //!< Returns all paths where plugins can be placed.
  static QList<Plugin *>        loadPlugins(void);                              //!< Loads all plugins.

public:
  inline explicit               Plugin(QObject *parent = NULL) : QObject(parent) { }
  virtual                       ~Plugin();

  virtual void                  setPluginDate(const QDateTime &);               //!< Can be optionally implemented if the date of the binary is required.

  virtual QString               pluginName(void) const = 0;                     //!< Should return the human readable name of the plugin.
  virtual QString               pluginVersion(void) const = 0;                  //!< Should return the human readable version string of the plugin.
  virtual QString               authorName(void) const = 0;                     //!< Should return the human readable author name of the plugin.

protected:
                                Plugin();
};

/*! This is the main interface for backend plugins.
 */
class BackendPlugin : public Plugin
{
Q_OBJECT
public:
  static QList<BackendPlugin *> loadPlugins(void);

public:
  inline explicit               BackendPlugin(QObject *parent = NULL) : Plugin(parent) { }
  virtual                       ~BackendPlugin();

  virtual QList<BackendServer *> createServers(BackendServer::MasterServer *) = 0; //!< Should create one or more servers that handles requests.
  virtual void                  registerSandbox(SandboxServer *) = 0;           //!< should register zero or more sandbox commands.
};


} // End of namespace

#endif
