/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#ifndef CAMERA_MODULE_H
#define CAMERA_MODULE_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStreamDevice>

namespace LXiMediaCenter {
namespace CameraBackend {

class Module : public QObject, public SModule
{
Q_OBJECT
Q_INTERFACES(LXiCore::SModule)
Q_PLUGIN_METADATA(IID LXiCore_SModule_iid)
public:
  virtual bool                  registerClasses(void);
  virtual void                  unload(void);
  virtual QByteArray            about(void);
  virtual QByteArray            licenses(void);

public:
  static const char             pluginName[];
};

} } // End of namespaces

#endif