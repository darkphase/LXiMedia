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

#include "televisionbackend.h"
#include "cameraserver.h"
//#include "scan.h"
//#include "teletextserver.h"
//#include "televisionserver.h"
//#include "configserver.h"

Q_EXPORT_PLUGIN2(PLUGIN_NAME, LXiMediaCenter::TelevisionBackend);

namespace LXiMediaCenter {

TelevisionBackend::TelevisionBackend(QObject *parent)
  : BackendPlugin(parent)//,
    //epgDatabase(NULL)
{
}

TelevisionBackend::~TelevisionBackend()
{
  //delete epgDatabase;
}

QString TelevisionBackend::pluginName(void) const
{
  return "Television";
}

QString TelevisionBackend::pluginVersion(void) const
{
  return "1.0";
}

QString TelevisionBackend::authorName(void) const
{
  return "A.J. Admiraal";
}

QList<BackendServer *> TelevisionBackend::createServers(BackendServer::MasterServer *server)
{
  //if (epgDatabase == NULL)
  //  epgDatabase = new EpgDatabase(this);

  QList<BackendServer *> servers;
  //TeletextServer * const teletextServer = new TeletextServer(epgDatabase, server, this);
  //TelevisionServer * const televisionServer = new TelevisionServer(epgDatabase, teletextServer, server, this);
  //ConfigServer * configServer = NULL;

//  if (televisionServer->hasTuners())
//  {
//    servers += televisionServer;
//    servers += teletextServer;
//    configServer = new ConfigServer(televisionServer, server, this);
//  }
//  else
//  {
//    delete televisionServer;
//    delete teletextServer;
//  }

  CameraServer * const cameraServer = new CameraServer(this, server, SAudioVideoInputNode::devices());
  if (cameraServer->hasCameras())
    servers += cameraServer;
  else
    delete cameraServer;

//  if (configServer)
//    servers += configServer;

  return servers;
}

} // End of namespace
