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
#include "screeninput.h"

namespace LXiStreamDevice {
namespace GdiCapture {

bool Module::registerClasses(void)
{
  if (qobject_cast<QApplication *>(QCoreApplication::instance()) == NULL)
  {
    qWarning() <<
        "Not loading GdiCapture because this is a non-gui application (i.e. "
        "a QCoreApplication is used instead of a QApplication)";

    return false; // Non-gui application
  }

  int result = false;
  foreach (const SFactory::Scheme &scheme, ScreenInput::listDevices())
  {
    ScreenInput::registerClass<ScreenInput>(scheme);

    result = true;
  }

  return result;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return "GDI screen capture plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  return QByteArray();
}

} } // End of namespaces
