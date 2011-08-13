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

#include "backendserver.h"

namespace LXiMediaCenter {

S_FACTORIZABLE_INSTANCE(BackendServer)

struct BackendServer::Data
{
  MasterServer                * masterServer;
};

const int          BackendServer::maxRequestTime = 1000;
const qreal        BackendServer::minSearchRelevance = 0.1;
const char         BackendServer::searchDateFormat[] = "ddd d MMM yyyy";
const char         BackendServer::searchTimeFormat[] = "hh:mm";
const char         BackendServer::searchDateTimeFormat[] = "ddd, MMM d yyyy hh:mm";
const char         BackendServer::dataMime[] = "application/octet-stream";
const char         BackendServer::textMime[] = "text/plain;charset=utf-8";

QList<BackendServer *> BackendServer::create(QObject *parent)
{
  return factory().createObjects<BackendServer>(parent);
}

BackendServer::BackendServer(QObject *parent)
  : QObject(parent),
    d(new Data())
{
  d->masterServer = NULL;
}

BackendServer::~BackendServer()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void BackendServer::initialize(MasterServer *masterServer)
{
  d->masterServer = masterServer;
}

void BackendServer::close(void)
{
}

QString BackendServer::serverPath(void) const
{
  return '/' + pluginName() + '/' + serverName() + '/';
}

QByteArray BackendServer::frontPageWidget(void) const
{
  return QByteArray("");
}

BackendServer::SearchResultList BackendServer::search(const QStringList &) const
{
  return SearchResultList();
}

SHttpServer::ResponseMessage BackendServer::makeResponse(const SHttpServer::RequestHeader &request, const QByteArray &data, const char *mime, bool allowCache) const
{
  SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
  response.setField("Cache-Control", allowCache ? "max-age=1800" : "no-cache");

  if (mime != NULL)
    response.setContentType(mime);

  if (!request.isHead())
    response.setContent(data);

  return response;
}

SHttpServer::ResponseMessage BackendServer::makeResponse(const SHttpServer::RequestHeader &request, const QString &data, const char *mime, bool allowCache) const
{
  return makeResponse(request, data.toUtf8(), mime, allowCache);
}

SHttpServer::ResponseMessage BackendServer::makeHtmlContent(const SHttpServer::RequestHeader &request, const QUrl &url, const QByteArray &content, const QByteArray &head) const
{
  SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  if (!request.isHead())
    response.setContent(d->masterServer->parseHtmlContent(url, content, head));

  return response;
}

QString BackendServer::basePath(const QString &path) const
{
  QString result = path.mid(serverPath().length());
  result = result.startsWith('/') ? result : ('/' + result);
  
  return result;
}

QString BackendServer::dirName(const QString &path)
{
  QString result = path;
  while (result.endsWith('/'))
    result = result.left(result.length() - 1);

  int ls = result.lastIndexOf('/');
  if (result >= 0)
    result = result.mid(ls + 1);
  
  return result;
}


BackendServer::SearchResult::SearchResult(void)
  : relevance(0.0)
{
}

BackendServer::SearchResult::~SearchResult()
{
}

} // End of namespace
