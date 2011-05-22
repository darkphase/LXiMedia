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

bool Module::registerClasses(void)
{
  int result = false;
  if (loadDevices)
  {
    foreach (const SFactory::Scheme &scheme, V4l2Input::listDevices())
    {
      V4l2Input::registerClass<V4l2Input>(scheme);

      result = true;
    }
  }

  return result;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return "Video 4 Linux plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  return QByteArray();
}

} } // End of namespaces

#include <QtPlugin>
Q_EXPORT_PLUGIN2(lxistream_v4l, LXiStream::V4lBackend::Module);
