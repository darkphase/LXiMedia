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

#include "cameraserver.h"
#include "module.h"
#include "televisionsandbox.h"

namespace LXiMediaCenter {
namespace TelevisionBackend {

CameraServer::CameraServer(const QString &, QObject *parent)
  : MediaServer(parent),
    masterServer(NULL)
{
}

void CameraServer::initialize(MasterServer *masterServer)
{
  this->masterServer = masterServer;

  MediaServer::initialize(masterServer);
}

void CameraServer::close(void)
{
  MediaServer::close();
}

QString CameraServer::pluginName(void) const
{
  return Module::pluginName;
}

QString CameraServer::serverName(void) const
{
  return QT_TR_NOOP("Cameras");
}

QString CameraServer::serverIconPath(void) const
{
  return "/img/camera-photo.png";
}

CameraServer::SearchResultList CameraServer::search(const QStringList &rawQuery) const
{
  SearchResultList list;

  return list;
}


CameraServer::Stream * CameraServer::streamVideo(const SHttpServer::RequestMessage &request)
{
  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  SSandboxClient * const sandbox = masterServer->createSandbox(SSandboxClient::Mode_Normal);
  sandbox->ensureStarted();

  QUrl rurl;
  rurl.setPath(TelevisionSandbox::path + request.file());
  rurl.addQueryItem("opencamera", QString::null);
  typedef QPair<QString, QString> QStringPair;
  foreach (const QStringPair &queryItem, url.queryItems())
    rurl.addQueryItem(queryItem.first, queryItem.second);

  const QStringList file = request.file().split('.');
  if (file.count() >= 2)
    rurl.addQueryItem("device", file.first());

  Stream *stream = new Stream(this, sandbox, request.path());
  if (stream->setup(rurl))
    return stream; // The graph owns the socket now.

  delete stream;
  masterServer->recycleSandbox(sandbox);

  return NULL;
}

int CameraServer::countItems(const QString &path)
{
  return SAudioVideoInputNode::devices().count();
}

QList<CameraServer::Item> CameraServer::listItems(const QString &path, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;
  QList<Item> result;

  const QStringList cameras = SAudioVideoInputNode::devices();
  for (int i=start, n=0; (i<cameras.count()) && (returnAll || (n<int(count))); i++, n++)
  {
    Item item;
    item.type = SUPnPContentDirectory::Item::Type_Video;
    item.played = false;
    item.url = cameras[i].toUtf8().toBase64();
    item.iconUrl = "/img/camera-photo.png";
    item.title = cameras[i];

    result += item;
  }

  return result;
}

SHttpServer::SocketOp CameraServer::handleHttpRequest(const SHttpServer::RequestMessage &request, QAbstractSocket *socket)
{
  if ((request.method() == "GET") || (request.method() == "HEAD"))
  {
    const QUrl url(request.path());
    const QString file = request.file();

    if (file.endsWith(".html")) // Show player
    {
      const QByteArray camera = file.left(file.length() - 5).toAscii();
      const QString title = QString::fromUtf8(QByteArray::fromBase64(camera));

      SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
      response.setContentType("text/html;charset=utf-8");
      response.setField("Cache-Control", "no-cache");

      return sendHtmlContent(request, socket, url, response, buildVideoPlayer(camera, title, url), headPlayer);
    }
  }

  return MediaServer::handleHttpRequest(request, socket);
}

CameraServer::Stream::Stream(CameraServer *parent, SSandboxClient *sandbox, const QString &url)
  : MediaServer::Stream(parent, url),
    sandbox(sandbox)
{
}

CameraServer::Stream::~Stream()
{
  static_cast<CameraServer *>(parent)->masterServer->recycleSandbox(sandbox);
}

bool CameraServer::Stream::setup(const QUrl &url)
{
  SHttpEngine::RequestMessage message(sandbox);
  message.setRequest("GET", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));

  sandbox->openRequest(message, &proxy, SLOT(setSource(QAbstractSocket *)));

  return true;
}

} } // End of namespaces
