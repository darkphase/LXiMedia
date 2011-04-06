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
#include <liblxicore/sfactory.hpp>

template class LXiCore::SFactorizable<LXiMediaCenter::BackendServer>;

namespace LXiMediaCenter {

struct BackendServer::Data
{
  MasterServer                * masterServer;
};

const int          BackendServer::maxRequestTime = 1000;
const qreal        BackendServer::minSearchRelevance = 0.1;
const char * const BackendServer::searchDateFormat = "ddd d MMM yyyy";
const char * const BackendServer::searchTimeFormat = "hh:mm";
const char * const BackendServer::searchDateTimeFormat = "ddd, MMM d yyyy hh:mm";
const char * const BackendServer::dataMime = "application/octet-stream";
const char * const BackendServer::textMime = "text/plain;charset=utf-8";

QList<BackendServer *> BackendServer::create(QObject *parent)
{
  return factory().createObjects<BackendServer>(parent);
}

void BackendServer::test(void)
{
  qDebug() << "BackendServer::test" << (void *)&(factory());
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

SHttpServer::SocketOp BackendServer::sendResponse(const SHttpServer::RequestHeader &request, QIODevice *socket, const QByteArray &data, const char *mime, bool allowCache, const QString &redir) const
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
  socket->write(data);
  return SHttpServer::SocketOp_Close;
}

SHttpServer::SocketOp BackendServer::sendResponse(const SHttpServer::RequestHeader &request, QIODevice *socket, const QString &data, const char *mime, bool allowCache, const QString &redir) const
{
  return sendResponse(request, socket, data.toUtf8(), mime, allowCache, redir);
}

SHttpServer::SocketOp BackendServer::sendHtmlContent(QIODevice *socket, const QUrl &url, const SHttpServer::ResponseHeader &response, const QByteArray &content, const QByteArray &head) const
{
  socket->write(response);
  socket->write(d->masterServer->parseHtmlContent(url, content, head));

  return SHttpServer::SocketOp_Close;
}

} // End of namespace
