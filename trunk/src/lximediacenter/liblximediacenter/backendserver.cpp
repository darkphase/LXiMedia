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

SHttpServer::SocketOp BackendServer::sendResponse(const SHttpServer::RequestHeader &request, QAbstractSocket *socket, const QByteArray &data, const char *mime, bool allowCache, const QString &redir) const
{
  SHttpServer::ResponseHeader response(request, (redir.length() == 0) ? SHttpServer::Status_Ok : SHttpServer::Status_MovedPermanently);
  if (!allowCache)
    response.setField("Cache-Control", "no-cache");
  else
    response.setField("Cache-Control", "max-age=1800");

  if (redir.length() == 0)
  {
    if (mime != NULL)
      response.setContentType(mime);

    if (data.length() > 0)
      response.setContentLength(data.length());
  }
  else
    response.setField("Location", redir);

  socket->write(response);
  if (request.method() != "HEAD")
    socket->write(data);

  return SHttpServer::SocketOp_Close;
}

SHttpServer::SocketOp BackendServer::sendResponse(const SHttpServer::RequestHeader &request, QAbstractSocket *socket, const QString &data, const char *mime, bool allowCache, const QString &redir) const
{
  return sendResponse(request, socket, data.toUtf8(), mime, allowCache, redir);
}

SHttpServer::SocketOp BackendServer::sendHtmlContent(const SHttpServer::RequestHeader &request, QAbstractSocket *socket, const QUrl &url, const SHttpServer::ResponseHeader &response, const QByteArray &content, const QByteArray &head) const
{
  socket->write(response);
  if (request.method() != "HEAD")
    socket->write(d->masterServer->parseHtmlContent(url, content, head));

  return SHttpServer::SocketOp_Close;
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
