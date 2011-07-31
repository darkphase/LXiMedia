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
#include "configserver.h"
#include "internetsandbox.h"
#include "internetserver.h"
#include "sitedatabase.h"

namespace LXiMediaCenter {
namespace InternetBackend {

template <const char * _name, const char * _icon>
class Server : public InternetServer
{
public:
  explicit Server(const QString &, QObject *parent)
    : InternetServer(_name, parent)
  {
  }

  virtual QString serverName(void) const
  {
    return _name;
  }

  virtual QString serverIconPath(void) const
  {
    return _icon;
  }
};

const char Module::pluginName[]     = QT_TR_NOOP("Internet");

const char Module::radioName[]      = QT_TR_NOOP("Radio"),        Module::radioIcon[] = "/img/audio-headset.png";
const char Module::tvName[]         = QT_TR_NOOP("TV"),           Module::tvIcon[]    = "/img/video-television.png";
const char Module::sitesName[]      = QT_TR_NOOP("Sites"),        Module::sitesIcon[] = "/img/homepage.png";

bool Module::registerClasses(void)
{
  InternetServer::registerClass< Server<radioName,  radioIcon> >(0);
  InternetServer::registerClass< Server<tvName,     tvIcon> >(-1);
  InternetServer::registerClass< Server<sitesName,  sitesIcon> >(-2);
  ConfigServer::registerClass<ConfigServer>(-6);

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
