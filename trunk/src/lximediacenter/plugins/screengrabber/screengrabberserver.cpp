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

#include "screengrabberserver.h"
#include "module.h"

namespace LXiMediaCenter {
namespace ScreenGrabberBackend {

ScreenGrabberServer::ScreenGrabberServer(const QString &, QObject *parent)
  : MediaServer(parent),
    sandbox(NULL)
{
}

void ScreenGrabberServer::initialize(MasterServer *masterServer)
{
  sandbox = new SSandboxClient("lximc-screengrabber", this);

  MediaServer::initialize(masterServer);
}

void ScreenGrabberServer::close(void)
{
  delete sandbox;
  sandbox = NULL;

  MediaServer::close();
}

QString ScreenGrabberServer::serverName(void) const
{
  return Module::pluginName;
}

QString ScreenGrabberServer::serverIconPath(void) const
{
  return "/img/camera-photo.png";
}

ScreenGrabberServer::Stream * ScreenGrabberServer::streamVideo(const SHttpServer::RequestMessage &request)
{
  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  SSandboxClient * const sandbox = new SSandboxClient("lximc-screengrabber", this);

  QUrl rurl;
  rurl.setPath("/" + request.file());
  rurl.addQueryItem("opendesktop", QString::null);
  rurl.addQueryItem("desktop", request.fileName());
  typedef QPair<QString, QString> QStringPair;
  foreach (const QStringPair &queryItem, url.queryItems())
    rurl.addQueryItem(queryItem.first, queryItem.second);

  Stream *stream = new Stream(this, sandbox, request.path());
  if (stream->setup(rurl))
    return stream; // The graph owns the socket now.

  delete stream;

  return NULL;
}

SHttpServer::ResponseMessage ScreenGrabberServer::sendPhoto(const SHttpServer::RequestMessage &request)
{
  return SHttpServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}

QList<ScreenGrabberServer::Item> ScreenGrabberServer::listItems(const QString &, int start, int &count)
{
  const bool returnAll = count == 0;
  QList<Item> result;

  SSandboxClient::RequestMessage request(sandbox);
  request.setRequest("GET", "/?listdesktops=");

  QStringList cameras;
  const SHttpEngine::ResponseMessage response = sandbox->blockingRequest(request, 250);
  if (response.status() == SHttpEngine::Status_Ok)
  {
    QXmlStreamReader reader(response.content());
    if (reader.readNextStartElement() && (reader.name() == "desktops"))
    while (reader.readNextStartElement())
    if (reader.name() == "desktop")
      cameras += reader.readElementText();
  }

  for (int i=start, n=0; (i<cameras.count()) && (returnAll || (n<int(count))); i++, n++)
  {
    Item item;
    item.type = SUPnPContentDirectory::Item::Type_Video;
    item.played = false;
    item.url = serverPath() + cameras[i].toUtf8().toHex();
    item.iconUrl = "/img/camera-photo.png";
    item.title = cameras[i];

    item.audioFormat.setChannelSetup(SAudioFormat::Channels_Stereo);

    result += item;
  }

  count = cameras.count();

  return result;
}

ScreenGrabberServer::Item ScreenGrabberServer::getItem(const QString &path)
{
  const QString file = path.mid(path.lastIndexOf('/') + 1);

  Item item;
  item.type = SUPnPContentDirectory::Item::Type_Video;
  item.played = false;
  item.url = serverPath() + file;
  item.iconUrl = "/img/camera-photo.png";
  item.title = QString::fromUtf8(QByteArray::fromHex(file.toAscii()));

  item.audioFormat.setChannelSetup(SAudioFormat::Channels_Stereo);

  return item;
}

ScreenGrabberServer::Stream::Stream(ScreenGrabberServer *parent, SSandboxClient *sandbox, const QString &url)
  : MediaServer::Stream(parent, url),
    sandbox(sandbox)
{
}

ScreenGrabberServer::Stream::~Stream()
{
  delete sandbox;
}

bool ScreenGrabberServer::Stream::setup(const QUrl &url)
{
  SHttpEngine::RequestMessage message(sandbox);
  message.setRequest("GET", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));

  sandbox->openRequest(message, &proxy, SLOT(setSource(QIODevice *, SHttpEngine *)), Qt::DirectConnection);

  return true;
}

} } // End of namespaces
