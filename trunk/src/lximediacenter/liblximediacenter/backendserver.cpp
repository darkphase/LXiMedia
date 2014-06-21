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
  return '/' + serverName() + '/';
}

QByteArray BackendServer::frontPageContent(void)
{
  return QByteArray();
}

QByteArray BackendServer::settingsContent(void)
{
  return QByteArray();
}

HttpStatus BackendServer::makeResponse(const QByteArray &data, QByteArray &contentType, QIODevice *&response) const
{
  if (contentType.isEmpty())
    contentType = UPnP::mimeAppOctet;

  QBuffer * const buffer = new QBuffer();
  buffer->setData(data);
  response = buffer;
  return HttpStatus_Ok;
}

HttpStatus BackendServer::makeResponse(const QString &data, QByteArray &contentType, QIODevice *&response) const
{
  if (contentType.isEmpty())
    contentType = UPnP::mimeTextPlain;

  return makeResponse(data.toUtf8(), contentType, response);
}

HttpStatus BackendServer::makeHtmlContent(const QUrl &request, const QByteArray &content, QByteArray &contentType, QIODevice *&response, const QByteArray &head) const
{
  return d->masterServer->parseHtmlContent(request, content, head, contentType, response);
}

BackendServer::MasterServer * BackendServer::masterServer(void)
{
  return d->masterServer;
}

BackendServer::SearchResult::SearchResult(void)
  : relevance(0.0)
{
}

BackendServer::SearchResult::~SearchResult()
{
}

} // End of namespace
