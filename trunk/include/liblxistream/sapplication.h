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

#ifndef LXISTREAM_SAPPLICATION_H
#define LXISTREAM_SAPPLICATION_H

#include <QtCore>
#include "sfactory.h"
#include "sinterfaces.h"

namespace LXiStream {

class SCodec;
class SGraph;
class SNode;
class SProber;
class STerminal;

class SApplication
{
friend class SGraph;
public:
  enum InitializeFlag
  {
    Initialize_NoFlags        = 0x0000,

    Initialize_Modules        = 0x0001,                                         //!< All modules are loaded automatically.
    Initialize_Devices        = 0x0002,                                         //!< All devices are loaded automatically.

    Initialize_UseOpenGL      = 0x0010,                                         //!< Enable OpenGL acceleration.

    Initialize_LogToFile      = 0x1000,                                         //!< Enables logging of events to a file.
    Initialize_LogToConsole   = 0x2000,                                         //!< Enables logging of events to the console.
    Initialize_HandleFaults   = 0x8000                                          //!< Enables handling of crashes (SegFault, etc.) internally.
  };
  Q_DECLARE_FLAGS(InitializeFlags, InitializeFlag)

  // Generic configurations
  static const InitializeFlag   Initialize_Default;

private:
  struct Module
  {
    inline Module(QPluginLoader *loader, SInterfaces::Module *module)
        : loader(loader), module(module)
    {
    }

    QPluginLoader             * loader;
    SInterfaces::Module       * module;
  };

public:
  explicit                      SApplication(InitializeFlags = Initialize_Default, const QString &preferredLogDir = QString::null);
                                ~SApplication();

  static const char           * name(void) __attribute__((pure));
  static const char           * version(void) __attribute__((pure));

  bool                          loadModule(SInterfaces::Module *);
  QByteArray                    about(void) const;

  inline InitializeFlags        initializeFlags(void)                           { return flags; }
  inline static SApplication  * instance(void)                                  { return self; }

private:
  template <class _module>
  inline void                   loadModule(void);

private:
  static SApplication         * self;

  InitializeFlags               flags;
  QList<Module>                 moduleList;
};


/*! Loads the classes from the specified module.
 */
template <class _module>
void SApplication::loadModule(void)
{
  _module * const module = new _module();
  module->registerClasses();

  moduleList += Module(NULL, module);
}

} // End of namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SApplication::InitializeFlags)

#endif
