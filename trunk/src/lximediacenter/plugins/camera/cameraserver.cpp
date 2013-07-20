/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#include "cameraserver.h"
#include "module.h"
#include "camerasandbox.h"

namespace LXiMediaCenter {
namespace CameraBackend {

CameraServer::CameraServer(const QString &, QObject *parent)
  : MediaServer(parent),
    masterServer(NULL),
    probeSandbox(NULL)
{
}

void CameraServer::initialize(MasterServer *masterServer)
{
  this->masterServer = masterServer;
  probeSandbox = masterServer->createSandbox(SSandboxClient::Priority_Low);

  MediaServer::initialize(masterServer);
}

void CameraServer::close(void)
{
  delete probeSandbox;
  probeSandbox = NULL;

  MediaServer::close();
}

QString CameraServer::serverName(void) const
{
  return Module::pluginName;
}

QString CameraServer::serverIconPath(void) const
{
  return "/img/camera-photo.png";
}

CameraServer::Stream * CameraServer::streamVideo(const SHttpServer::RequestMessage &request)
{
  QUrl url(request.path());
  if (request.query().hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(request.query().queryItemValue("query").toLatin1());

  SSandboxClient * const sandbox = masterServer->createSandbox(SSandboxClient::Priority_Normal);
  sandbox->ensureStarted();

  QUrlQuery rquery;
  rquery.addQueryItem("opencamera", QString::null);
  rquery.addQueryItem("device", request.fileName());
  typedef QPair<QString, QString> QStringPair;
  foreach (const QStringPair &queryItem, QUrlQuery(url).queryItems())
    rquery.addQueryItem(queryItem.first, queryItem.second);

  QUrl rurl;
  rurl.setPath(CameraSandbox::path + request.file());
  rurl.setQuery(rquery);

  Stream *stream = new Stream(this, sandbox, request.path());
  if (stream->setup(rurl))
    return stream; // The graph owns the socket now.

  delete stream;
  delete sandbox;

  return NULL;
}

SHttpServer::ResponseMessage CameraServer::sendPhoto(const SHttpServer::RequestMessage &request)
{
  return SHttpServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}

QList<CameraServer::Item> CameraServer::listItems(const QString &virtualPath, int start, int &count)
{
  const bool returnAll = count == 0;
  QList<Item> result;

  SSandboxClient::RequestMessage request(probeSandbox);
  request.setRequest("GET", QByteArray(CameraSandbox::path) + "?listcameras=");

  QStringList cameras;
  const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request);
  if (response.status() == SHttpEngine::Status_Ok)
  {
    QXmlStreamReader reader(response.content());
    if (reader.readNextStartElement() && (reader.name() == "cameras"))
    while (reader.readNextStartElement())
    if (reader.name() == "camera")
      cameras += reader.readElementText();
  }

  for (int i=start, n=0; (i<cameras.count()) && (returnAll || (n<int(count))); i++, n++)
  {
    const QByteArray name = cameras[i].toUtf8().toHex();

    Item item;
    item.path = virtualPath + '/' + name;
    item.type = SUPnPContentDirectory::Item::Type_Video;
    item.played = false;
    item.url = serverPath() + name;
    item.iconUrl = "/img/camera-photo.png";
    item.title = cameras[i];

    item.audioFormat.setChannelSetup(SAudioFormat::Channels_Stereo);

    result += item;
  }

  count = cameras.count();

  return result;
}

CameraServer::Item CameraServer::getItem(const QString &path)
{
  const QString file = path.mid(path.lastIndexOf('/') + 1);

  Item item;
  item.type = SUPnPContentDirectory::Item::Type_Video;
  item.played = false;
  item.url = serverPath() + file;
  item.iconUrl = "/img/camera-photo.png";
  item.title = QString::fromUtf8(QByteArray::fromHex(file.toLatin1()));

  item.audioFormat.setChannelSetup(SAudioFormat::Channels_Stereo);

  return item;
}

CameraServer::Stream::Stream(CameraServer *parent, SSandboxClient *sandbox, const QString &url)
  : MediaServer::Stream(parent, url),
    sandbox(sandbox)
{
}

CameraServer::Stream::~Stream()
{
  delete sandbox;
}

bool CameraServer::Stream::setup(const QUrl &url)
{
  SHttpEngine::RequestMessage message(sandbox);
  message.setRequest("GET", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));

  sandbox->openRequest(message, &proxy, SLOT(setSource(QIODevice *, SHttpEngine *)), Qt::DirectConnection);

  return true;
}

} } // End of namespaces
