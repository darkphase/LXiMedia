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

#include "screengrabberserver.h"
#include "module.h"

namespace LXiMediaCenter {
namespace ScreenGrabberBackend {

ScreenGrabberServer::ScreenGrabberServer(const QString &, QObject *parent)
  : MediaServer(parent),
    ssdpClient(NULL),
    searchTimer(),
    searchCounter(0)
{
  searchTimer.setInterval(500);
  connect(&searchTimer, SIGNAL(timeout()), SLOT(sendSearch()));
}

void ScreenGrabberServer::initialize(MasterServer *masterServer)
{
  ssdpClient = masterServer->ssdpServer();
  searchCounter = 0;
  searchTimer.start();

  MediaServer::initialize(masterServer);
}

void ScreenGrabberServer::close(void)
{
  MediaServer::close();

  searchTimer.stop();
  ssdpClient = NULL;
}

QString ScreenGrabberServer::serverName(void) const
{
  return Module::pluginName;
}

QString ScreenGrabberServer::serverIconPath(void) const
{
  return "/img/video-display.png";
}

ScreenGrabberServer::Stream * ScreenGrabberServer::streamVideo(const SHttpServer::RequestMessage &request)
{
  QUrl url(request.path());
  if (request.query().hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(request.query().queryItemValue("query").toLatin1());

  const QStringList path = QString(request.path()).split('/');
  if (path.count() >= 2)
  {
    updateNodes();

    QMap<QString, SSsdpClient::Node>::Iterator node =
        nodes.find(path[path.count() - 2]);

    if (node != nodes.end())
    {
      SHttpClient * const httpClient = new SHttpClient(this);

      QUrlQuery rquery;
      rquery.addQueryItem("opendesktop", QString::null);
      rquery.addQueryItem("desktop", request.fileName());
      typedef QPair<QString, QString> QStringPair;
      foreach (const QStringPair &queryItem, QUrlQuery(url).queryItems())
        rquery.addQueryItem(queryItem.first, queryItem.second);

      QUrl rurl(node->location);
      rurl.setPath("/desktop");
      rurl.setQuery(rquery);

      Stream *stream = new Stream(this, httpClient, request.path());
      if (stream->setup(rurl))
        return stream; // The graph owns the socket now.

      delete stream;
    }
  }

  return NULL;
}

SHttpServer::ResponseMessage ScreenGrabberServer::sendPhoto(const SHttpServer::RequestMessage &request)
{
  return SHttpServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}

QList<ScreenGrabberServer::Item> ScreenGrabberServer::listItems(const QString &path, int start, int &count)
{
  const bool returnAll = count == 0;
  QList<Item> result;

  if (path == serverPath())
  {
    updateNodes();

    const QList<QString> hostnames = nodes.keys();

    for (int i=start, n=0; (i<hostnames.count()) && (returnAll || (n<int(count))); i++, n++)
      result += makeItem(serverPath() + hostnames[i] + '/');

    count = hostnames.count();
  }
  else
  {
    QString filePath = path;
    while (filePath.endsWith('/'))
      filePath = filePath.left(filePath.length() - 1);

    QMap<QString, SSsdpClient::Node>::Iterator node =
        nodes.find(filePath.split('/').last());

    if (node != nodes.end())
    {
      SHttpClient httpClient;
      QStringList desktops;

      QUrl url(node->location);

      SHttpClient::RequestMessage request(&httpClient);
      request.setHost(url.host(), url.port());
      request.setRequest("GET", "/desktops");

      const SHttpEngine::ResponseMessage response = httpClient.blockingRequest(request, 250);
      if (response.status() == SHttpEngine::Status_Ok)
      {
        QXmlStreamReader reader(response.content());
        if (reader.readNextStartElement() && (reader.name() == "desktops"))
        while (reader.readNextStartElement())
        if (reader.name() == "desktop")
          desktops += reader.readElementText();
      }

      for (int i=start, n=0; (i<desktops.count()) && (returnAll || (n<int(count))); i++, n++)
        result += makeItem(path + desktops[i].toUtf8().toHex());

      count = desktops.count();
    }
  }

  return result;
}

ScreenGrabberServer::Item ScreenGrabberServer::getItem(const QString &path)
{
  return makeItem(path);
}

void ScreenGrabberServer::sendSearch()
{
  if (ssdpClient)
  {
    if (searchCounter++ < 3)
      ssdpClient->sendSearch("urn:XXX:service:ScreenGrabber:1");
    else
      searchTimer.stop();
  }
}

ScreenGrabberServer::Item ScreenGrabberServer::makeItem(const QString &path)
{
  QString virtualPath = path.mid(serverPath().length());
  while (virtualPath.endsWith('/'))
    virtualPath = virtualPath.left(virtualPath.length() - 1);

  const int ls = virtualPath.lastIndexOf('/');
  if (ls >= 0)
  {
    const QString file = virtualPath.mid(ls + 1);
    const QString dir = virtualPath.left(ls);
    if (!file.isEmpty() && !dir.isEmpty())
    {
      Item item;
      item.isDir = false;
      item.path = path;
      item.type = SUPnPContentDirectory::Item::Type_Video;
      item.played = false;
      item.url = path;
      item.iconUrl = "/img/video-display.png";
      item.title = QString::fromUtf8(QByteArray::fromHex(file.toLatin1()));

      item.audioFormat.setChannelSetup(SAudioFormat::Channels_Stereo);

      return item;
    }
  }

  Item item;
  item.isDir = true;
  item.path = path;
  item.played = false;
  item.url = path;
  item.iconUrl = "/img/screengrabber.png";
  item.title = virtualPath;

  return item;
}

void ScreenGrabberServer::updateNodes(void)
{
  SHttpClient httpClient;

  if (ssdpClient)
  {
    const QList<SSsdpClient::Node> activeNodes =
        ssdpClient->searchResults("urn:XXX:service:ScreenGrabber:1");

    QSet<QString> activeUuids;

    // Add new nodes.
    foreach (const SSsdpClient::Node &node, activeNodes)
    {
      QUrl url(node.location);

      activeUuids.insert(node.uuid);

      bool found = false;
      for (QMap<QString, SSsdpClient::Node>::Iterator i = nodes.begin();
           (i != nodes.end()) && !found;
           i++)
      {
        if (i->uuid == node.uuid)
        {
          *i = node;
          found = true;
        }
      }

      if (!found)
      {
        SHttpClient::RequestMessage request(&httpClient);
        request.setHost(url.host(), url.port());
        request.setRequest("GET", "/hostname");

        const SHttpEngine::ResponseMessage response = httpClient.blockingRequest(request, 250);
        if (response.status() == SHttpEngine::Status_Ok)
        {
          QXmlStreamReader reader(response.content());
          if (reader.readNextStartElement() && (reader.name() == "hostname"))
          {
            const QString hostname = reader.readElementText();

            QString name = hostname;
            for (int i=0;; i++)
            if (nodes.find(name) != nodes.end())
              name = hostname + QString::number(i + 1);
            else
              break;

            nodes[name] = node;
          }
        }
      }
    }

    // Remove deactivated nodes
    for (QMap<QString, SSsdpClient::Node>::Iterator i = nodes.begin();
         i != nodes.end(); )
    {
      if (!activeUuids.contains(i->uuid))
        i = nodes.erase(i);
      else
        i++;
    }
  }
}

ScreenGrabberServer::Stream::Stream(ScreenGrabberServer *parent, SHttpClient *httpClient, const QString &url)
  : MediaServer::Stream(parent, url),
    httpClient(httpClient)
{
}

ScreenGrabberServer::Stream::~Stream()
{
  delete httpClient;
}

bool ScreenGrabberServer::Stream::setup(const QUrl &url)
{
  SHttpEngine::RequestMessage message(httpClient);
  message.setHost(url.host(), url.port());
  message.setRequest("GET", url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority));

  httpClient->openRequest(message, &proxy, SLOT(setSource(QIODevice *, SHttpEngine *)), Qt::DirectConnection);

  return true;
}

} } // End of namespaces
