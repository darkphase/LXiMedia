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
#include "mediadatabase.h"
#include "mediaplayerserver.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char Module::pluginName[]     = QT_TR_NOOP("Media Player");

bool Module::registerClasses(void)
{
  MediaPlayerServer::registerClass<MediaPlayerServer>(0);

  return true;
}

void Module::unload(void)
{
  MediaDatabase::destroyInstance();
}

QByteArray Module::about(void)
{
  return QByteArray(pluginName) + " by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  const QByteArray text =
      " <h3>KDE monochromatic icons theme</h3>\n"
      " <p>Website: <a href=\"http://www.kde.org/\">www.kde.org</a></p>\n"
      " <p>Used under the terms of the GNU General Public License version 3\n"
      " as published by the Free Software Foundation.</p>\n";

  return text;
}

} } // End of namespaces
