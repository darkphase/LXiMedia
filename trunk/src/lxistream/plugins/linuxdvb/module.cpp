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
#include "dvbdevice.h"
#include "dvbinput.h"

namespace LXiStream {
namespace LinuxDvbBackend {


void Module::registerClasses(void)
{
  if ((SSystem::initializeFlags() & SSystem::Initialize_Devices) == SSystem::Initialize_Devices)
  {
    SSystem::registerClass<DVBDevice>("lx-linuxdvb");
    SSystem::registerVideoCaptureDevices(DVBDevice::listDevices());
  }
}

void Module::unload(void)
{
}


} } // End of namespaces
