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

#include "module.h"
#include "v4l2input.h"

namespace LXiStream {
namespace V4lBackend {


void Module::registerClasses(void)
{
  if ((SApplication::instance()->initializeFlags() & SApplication::Initialize_Devices) == SApplication::Initialize_Devices)
  {
    foreach (const SFactory::Scheme &scheme, V4l2Input::listDevices())
      V4l2Input::registerClass<V4l2Input>(scheme);
  }
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return QByteArray();
}


} } // End of namespaces

#ifdef PLUGIN_NAME
#include <QtPlugin>
Q_EXPORT_PLUGIN2(PLUGIN_NAME, LXiStream::V4lBackend::Module);
#endif
