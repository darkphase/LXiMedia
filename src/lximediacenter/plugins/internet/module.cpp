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
#include "internetsandbox.h"
#include "internetserver.h"
#include "sitedatabase.h"

namespace LXiMediaCenter {
namespace InternetBackend {

const char Module::pluginName[]     = QT_TR_NOOP("Internet");

bool Module::registerClasses(void)
{
  InternetServer::registerClass<InternetServer>(0);
  InternetSandbox::registerClass<InternetSandbox>(0);

  return true;
}

void Module::unload(void)
{
  SiteDatabase::destroyInstance();
}

QByteArray Module::about(void)
{
  return QByteArray(pluginName) + " by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  const QByteArray text;

  return text;
}

} } // End of namespaces

#include <QtPlugin>
Q_EXPORT_PLUGIN2(lximediacenter_internet, LXiMediaCenter::InternetBackend::Module);
