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

SHttpServer::SocketOp ConfigServer::handleHttpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if ((request.method() == "GET") || (request.method() == "POST") || (request.method() == "HEAD"))
  {
    const MediaServer::File file(request);

    if (file.baseName().isEmpty() || (file.suffix() == "html"))
    {
      return handleHtmlRequest(request, socket, file);
    }
    else if (file.suffix() == "txt")
    {
      QFile txtFile(":/internet/sites/" + file.baseName().toUpper());
      if (txtFile.open(QFile::ReadOnly))
      {
        SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
        response.setContentType("text/plain;charset=utf-8");
        response.setField("Cache-Control", "no-cache");

        socket->write(response);
        socket->write(txtFile.readAll());
        return SHttpServer::SocketOp_Close;
      }
    }
    else if (file.suffix() == "js")
    {
      SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
      response.setContentType("text/javascript;charset=utf-8");
      response.setField("Cache-Control", "no-cache");

      socket->write(response);
      socket->write(siteDatabase->script(file.baseName()).toUtf8());
      return SHttpServer::SocketOp_Close;
    }
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

void ConfigServer::handleHttpOptions(SHttpServer::ResponseHeader &response)
{
  response.setField("Allow", response.field("Allow") + ",GET,POST,HEAD");
}

} } // End of namespaces
