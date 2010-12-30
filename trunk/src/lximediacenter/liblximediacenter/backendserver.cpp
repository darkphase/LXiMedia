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

#include <liblximediacenter/backendserver.h>
#include <liblximediacenter/plugininterfaces.h>

namespace LXiMediaCenter {

const int          BackendServer::maxRequestTime = 1000;
const qreal        BackendServer::minSearchRelevance = 0.1;
const char * const BackendServer::searchDateFormat = "ddd d MMM yyyy";
const char * const BackendServer::searchTimeFormat = "hh:mm";
const char * const BackendServer::searchDateTimeFormat = "ddd, MMM d yyyy hh:mm";
const char * const BackendServer::dataMime = "application/octet-stream";
const char * const BackendServer::textMime = "text/plain;charset=utf-8";

struct BackendServer::Private
{
  MasterServer                * server;
  QString                       name;
  QString                       httpPath;
  QString                       dlnaPath;

  QFuture<void>                 dlnaUpdateFuture;
};

BackendServer::BackendServer(const char *name, Plugin *plugin, MasterServer *server)
    : QObject(server),
      httpDir(server->httpServer(), this),
      dlnaDir(server->dlnaServer()),
      p(new Private())
{
  Q_ASSERT(name);

  p->server = server;
  p->name = tr(name);

  if (plugin)
    p->httpPath = "/" + plugin->pluginName().toLower() + "/" + SStringParser::toRawName(name).toLower() + "/";
  else
    p->httpPath = "/" + SStringParser::toRawName(name).toLower() + "/";

  p->httpPath.replace(" ", "");

  p->server->httpServer()->addDir(p->httpPath, &httpDir);
}

BackendServer::~BackendServer()
{
  p->dlnaUpdateFuture.waitForFinished();

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

QByteArray BackendServer::frontPageWidget(void) const
{
  return QByteArray("");
}

BackendServer::SearchResultList BackendServer::search(const QStringList &) const
{
  return SearchResultList();
}

BackendServer::MasterServer * BackendServer::masterServer(void) const
{
  return p->server;
}

const QString & BackendServer::name(void) const
{
  return p->name;
}

const QString & BackendServer::httpPath(void) const
{
  return p->httpPath;
}

const QString & BackendServer::dlnaPath(void) const
{
  return p->dlnaPath;
}

bool BackendServer::sendReply(QAbstractSocket *socket, const QByteArray &data, const char *mime, bool allowCache, const QString &redir) const
{
  QHttpResponseHeader response((redir.length() == 0) ? 200 : 301);

  if (!allowCache)
    response.setValue("Cache-Control", "no-cache");
  else
    response.setValue("Cache-Control", "max-age=1800");

  if (redir.length() == 0)
  {
    if (mime != NULL)
      response.setContentType(mime);

    if (data.length() > 0)
      response.setContentLength(data.length());
  }
  else
    response.setValue("Location", redir);

  socket->write(response.toString().toUtf8());
  socket->write(data);

  return false;
}

bool BackendServer::sendReply(QAbstractSocket *socket, const QString &data, const char *mime, bool allowCache, const QString &redir) const
{
  return sendReply(socket, data.toUtf8(), mime, allowCache, redir);
}

bool BackendServer::sendHtmlContent(QAbstractSocket *socket, const QUrl &url, const QHttpResponseHeader &response, const QByteArray &content, const QByteArray &head) const
{
  socket->write(response.toString().toUtf8());
  socket->write(p->server->parseHtmlContent(url, content, head));

  return false;
}

void BackendServer::startDlnaUpdate(void)
{
  class Update : public QRunnable
  {
  public:
    inline Update(BackendServer *backendServer) : backendServer(backendServer)  { }

    virtual void run(void)
    {
      backendServer->updateDlnaTask();
    }

  private:
    BackendServer * const backendServer;
  };

  if (!p->dlnaUpdateFuture.isStarted() || p->dlnaUpdateFuture.isFinished())
    masterServer()->ioThreadPool()->start(new Update(this), 0);
}

void BackendServer::enableDlna(void)
{
  p->dlnaPath = "/" + p->name + "/";
  p->server->dlnaServer()->addDir(p->dlnaPath, &dlnaDir);
}

void BackendServer::updateDlnaTask(void)
{
}

bool BackendServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  return httpDir.superHandleConnection(request, socket);
}


BackendServer::HttpDir::HttpDir(HttpServer *server, BackendServer *parent)
    : HttpServerDir(server),
      parent(parent)
{
}

bool BackendServer::HttpDir::handleConnection(const QHttpRequestHeader &h, QAbstractSocket *s)
{
  return parent->handleConnection(h, s);
}


} // End of namespace
