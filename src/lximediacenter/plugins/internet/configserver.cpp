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

#include "configserver.h"
#include "module.h"
#include "sitedatabase.h"

namespace LXiMediaCenter {
namespace InternetBackend {

const char  ConfigServer::dirSplit =
#if defined(Q_OS_UNIX)
    ':';
#elif  defined(Q_OS_WIN)
    ';';
#else
#error Not implemented.
#endif

ConfigServer::ConfigServer(const QString &, QObject *parent)
  : BackendServer(parent),
    masterServer(NULL),
    siteDatabase(NULL)
{
}

ConfigServer::~ConfigServer()
{
}

void ConfigServer::initialize(MasterServer *masterServer)
{
  this->masterServer = masterServer;
  this->siteDatabase = SiteDatabase::createInstance();

  BackendServer::initialize(masterServer);

  masterServer->httpServer()->registerCallback(serverPath(), this);
}

void ConfigServer::close(void)
{
  BackendServer::close();

  masterServer->httpServer()->unregisterCallback(this);
}

QString ConfigServer::pluginName(void) const
{
  return Module::pluginName;
}

QString ConfigServer::serverName(void) const
{
  return QT_TR_NOOP("Settings");
}

QString ConfigServer::serverIconPath(void) const
{
  return "/img/control.png";
}

SHttpServer::ResponseMessage ConfigServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *)
{
  if (request.isGet() || request.isPost())
  {
    const MediaServer::File file(request);

    if (file.fileName().isEmpty() || file.fileName().endsWith(".html", Qt::CaseInsensitive))
    {
      return handleHtmlRequest(request, file);
    }
    else if (file.fileName() == "README")
    {
      QFile txtFile(":/internet/sites/" + file.fileName());
      if (txtFile.open(QFile::ReadOnly))
      {
        SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
        response.setField("Cache-Control", "no-cache");
        response.setContentType("text/plain;charset=utf-8");
        response.setContent(txtFile.readAll());

        return response;
      }
    }
    else if (file.fileName().endsWith(".js", Qt::CaseInsensitive))
    {
      SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
      response.setField("Cache-Control", "no-cache");
      response.setContentType("text/javascript;charset=utf-8");
      response.setContent(siteDatabase->script(file.fileName().left(file.fileName().length() - 3)).toUtf8());

      return response;
    }
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

} } // End of namespaces
