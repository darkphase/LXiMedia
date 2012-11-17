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

#include "module.h"
#include "winmmaudioinput.h"
#include "winmmaudiooutput.h"

namespace LXiStreamDevice {
namespace WinMMBackend {

bool Module::registerClasses(void)
{
  WinMMAudioOutput::registerClass<WinMMAudioOutput>(1);

  foreach (const SFactory::Scheme &scheme, WinMMAudioInput::listDevices())
    WinMMAudioInput::registerClass<WinMMAudioInput>(scheme);

  return true;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return "WinMM plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  return QByteArray();
}

} } // End of namespaces
