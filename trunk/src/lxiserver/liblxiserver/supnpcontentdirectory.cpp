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

#include "supnpcontentdirectory.h"
#include "supnpgenaserver.h"

#define USE_SMALL_OBJECTIDS
#define USE_SMALL_QUERYIDS

namespace LXiServer {

const char      SUPnPContentDirectory::contentDirectoryNS[] = "urn:schemas-upnp-org:service:ContentDirectory:1";
const unsigned  SUPnPContentDirectory::seekSec = 120;

struct SUPnPContentDirectory::Data : SUPnPContentDirectory::Callback
{
  virtual int                   countContentDirItems(const QString &peer, const QString &path);
  virtual QList<Item>           listContentDirItems(const QString &peer, const QString &path, unsigned start, unsigned count);

  SUPnPGenaServer             * genaServer;

  QMap<QString, Callback *>     callbacks;
  QAtomicInt                    systemUpdateId;

#ifdef USE_SMALL_OBJECTIDS
  QVector<QByteArray>           objectIdList;
  QHash<QByteArray, qint32>     objectIdHash;
#endif

#ifdef USE_SMALL_QUERYIDS
  static QVector<QByteArray>    queryIdList;
  static QHash<QByteArray, qint32> queryIdHash;
#endif
};

#ifdef USE_SMALL_QUERYIDS
QVector<QByteArray>       SUPnPContentDirectory::Data::queryIdList;
QHash<QByteArray, qint32> SUPnPContentDirectory::Data::queryIdHash;
#endif

SUPnPContentDirectory::SUPnPContentDirectory(const QString &basePath, QObject *parent)
    : SUPnPBase(basePath + "contentdirectory/", parent),
      d(new Data())
{
  d->genaServer = new SUPnPGenaServer(SUPnPBase::basePath(), this);

  d->systemUpdateId = 1;

  // Add the root path.
  d->callbacks.insert("/", d);

#ifdef USE_SMALL_OBJECTIDS
  d->objectIdList.append(QByteArray());
  d->objectIdHash.insert(d->objectIdList.last(), d->objectIdList.count() - 1);
#endif
}

SUPnPContentDirectory::~SUPnPContentDirectory()
{
  delete d->genaServer;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SUPnPContentDirectory::initialize(SHttpServer *httpServer, SUPnPMediaServer *mediaServer)
{
  d->genaServer->initialize(httpServer);
  emitEvent(false); // To build the initial event message

  SUPnPMediaServer::Service service;
  SUPnPBase::initialize(httpServer, service);
  service.serviceType = contentDirectoryNS;
  service.serviceId = "urn:upnp-org:serviceId:ContentDirectory";
  service.eventSubURL = d->genaServer->path();
  mediaServer->registerService(service);
}

void SUPnPContentDirectory::close(void)
{
  SUPnPBase::close();
  d->genaServer->close();
}

void SUPnPContentDirectory::registerCallback(const QString &path, Callback *callback)
{
  d->callbacks.insert(path, callback);
}

void SUPnPContentDirectory::unregisterCallback(Callback *callback)
{
  for (QMap<QString, Callback *>::Iterator i=d->callbacks.begin(); i!=d->callbacks.end(); )
  if (i.value() == callback)
    i = d->callbacks.erase(i);
  else
    i++;
}

QByteArray SUPnPContentDirectory::toQueryPath(const QByteArray &path, const QByteArray &query, const QByteArray &suffix)
{
#ifdef USE_SMALL_QUERYIDS
  QHash<QByteArray, qint32>::ConstIterator i = Data::queryIdHash.find(query);
  if (i != Data::queryIdHash.end())
    return path + ".." + ("0000000" + QByteArray::number(*i, 16)).right(8) + suffix;

  Data::queryIdList.append(query);
  Data::queryIdList.last().squeeze();
  Data::queryIdHash.insert(Data::queryIdList.last(), Data::queryIdList.count() - 1);

  return path + ".." + ("0000000" + QByteArray::number(Data::queryIdList.count() - 1, 16)).right(8) + suffix;
#else
  return path + ".." + qCompress(query, 9).toHex() + suffix;
#endif
}

bool SUPnPContentDirectory::fromQueryPath(const QString &queryStr, QString &path, QByteArray &query)
{
  const int lastDot = queryStr.lastIndexOf('.');
  const int lastDotDot = queryStr.lastIndexOf("..");
  if ((lastDot > (lastDotDot + 1)) && (lastDotDot >= 0))
  {
    path = queryStr.left(lastDotDot) + queryStr.mid(lastDot);
    const QByteArray q = queryStr.mid(lastDotDot + 2, (lastDot - lastDotDot) - 2).toAscii();

#ifdef USE_SMALL_QUERYIDS
    const qint32 id = q.toInt(NULL, 16);
    if (id < Data::queryIdList.count())
    {
      query = Data::queryIdList[id];
      return true;
    }

    return false;
#else
    query = qUncompress(QByteArray::fromHex(q));
    return true;
#endif
  }

  return false;
}

void SUPnPContentDirectory::modified(void)
{
  emitEvent(true);
}

void SUPnPContentDirectory::buildDescription(QDomDocument &doc, QDomElement &scpdElm)
{
  QDomElement actionListElm = doc.createElement("actionList");
  {
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "GetSearchCapabilities");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "SearchCaps", "out", "SearchCapabilities");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }
  {
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "GetSortCapabilities");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "SortCaps", "out", "SortCapabilities");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }
  {
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "GetSystemUpdateID");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "Id", "out", "SystemUpdateID");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }
  {
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "Browse");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "ObjectID", "in", "A_ARG_TYPE_ObjectID");
    addActionArgument(doc, argumentListElm, "BrowseFlag", "in", "A_ARG_TYPE_BrowseFlag");
    addActionArgument(doc, argumentListElm, "Filter", "in", "A_ARG_TYPE_Filter");
    addActionArgument(doc, argumentListElm, "StartingIndex", "in", "A_ARG_TYPE_Index");
    addActionArgument(doc, argumentListElm, "RequestedCount", "in", "A_ARG_TYPE_Count");
    addActionArgument(doc, argumentListElm, "SortCriteria", "in", "A_ARG_TYPE_SortCriteria");
    addActionArgument(doc, argumentListElm, "Result", "out", "A_ARG_TYPE_Result");
    addActionArgument(doc, argumentListElm, "NumberReturned", "out", "A_ARG_TYPE_Count");
    addActionArgument(doc, argumentListElm, "TotalMatches", "out", "A_ARG_TYPE_Count");
    addActionArgument(doc, argumentListElm, "UpdateID", "out", "A_ARG_TYPE_UpdateID");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }
  /*{
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "Search");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "ContainerID", "in", "A_ARG_TYPE_ObjectID");
    addActionArgument(doc, argumentListElm, "SearchCriteria", "in", "A_ARG_TYPE_SearchCriteria");
    addActionArgument(doc, argumentListElm, "Filter", "in", "A_ARG_TYPE_Filter");
    addActionArgument(doc, argumentListElm, "StartingIndex", "in", "A_ARG_TYPE_Index");
    addActionArgument(doc, argumentListElm, "RequestedCount", "in", "A_ARG_TYPE_Count");
    addActionArgument(doc, argumentListElm, "SortCriteria", "in", "A_ARG_TYPE_SortCriteria");
    addActionArgument(doc, argumentListElm, "Result", "out", "A_ARG_TYPE_Result");
    addActionArgument(doc, argumentListElm, "NumberReturned", "out", "A_ARG_TYPE_Count");
    addActionArgument(doc, argumentListElm, "TotalMatches", "out", "A_ARG_TYPE_Count");
    addActionArgument(doc, argumentListElm, "UpdateID", "out", "A_ARG_TYPE_UpdateID");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }*/
  scpdElm.appendChild(actionListElm);

  QDomElement serviceStateTableElm = doc.createElement("serviceStateTable");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_ObjectID", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Result", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_SearchCriteria", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_BrowseFlag", "string", QStringList() << "BrowseMetadata" << "BrowseDirectChildren");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Filter", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_SortCriteria", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Index", "ui4");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Count", "ui4");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_UpdateID", "ui4");
  addStateVariable(doc, serviceStateTableElm, false, "SearchCapabilities", "string");
  addStateVariable(doc, serviceStateTableElm, false, "SortCapabilities", "string");
  addStateVariable(doc, serviceStateTableElm, true, "SystemUpdateID", "ui4");
  addStateVariable(doc, serviceStateTableElm, true, "TransferIDs", "string");
  scpdElm.appendChild(serviceStateTableElm);
}

SHttpServer::Status SUPnPContentDirectory::handleSoapMessage(const QDomElement &body, QDomDocument &responseDoc, QDomElement &responseBody, const SHttpServer::RequestMessage &request, const QHostAddress &peerAddress)
{
  SHttpServer::Status status = SHttpServer::Status(402, "Invalid args");

  const QDomElement browseElm = firstChildElementNS(body, contentDirectoryNS, "Browse");
  if (!browseElm.isNull())
  {
    if (handleBrowse(browseElm, responseDoc, responseBody, request, peerAddress))
      status = SHttpServer::Status_Ok;
    else
      status = SHttpServer::Status(701, "No such object");
  }

  const QDomElement getSearchCapabilitiesElm = firstChildElementNS(body, contentDirectoryNS, "GetSearchCapabilities");
  if (!getSearchCapabilitiesElm.isNull())
  {
    QDomElement responseElm = createElementNS(responseDoc, getSearchCapabilitiesElm, "GetSearchCapabilitiesResponse");
    addTextElm(responseDoc, responseElm, "SearchCaps", "");
    responseBody.appendChild(responseElm);
    status = SHttpServer::Status_Ok;
  }

  const QDomElement getSortCapabilitiesElm = firstChildElementNS(body, contentDirectoryNS, "GetSortCapabilities");
  if (!getSortCapabilitiesElm.isNull())
  {
    QDomElement responseElm = createElementNS(responseDoc, getSortCapabilitiesElm, "GetSortCapabilitiesResponse");
    addTextElm(responseDoc, responseElm, "SortCaps", "");
    responseBody.appendChild(responseElm);
    status = SHttpServer::Status_Ok;
  }

  const QDomElement getSystemUpdateIdElm = firstChildElementNS(body, contentDirectoryNS, "GetSystemUpdateID");
  if (!getSystemUpdateIdElm.isNull())
  {
    QDomElement responseElm = createElementNS(responseDoc, getSystemUpdateIdElm, "GetSystemUpdateIDResponse");
    addTextElm(responseDoc, responseElm, "Id", QString::number(d->systemUpdateId));
    responseBody.appendChild(responseElm);
    status = SHttpServer::Status_Ok;
  }

  return status;
}

bool SUPnPContentDirectory::handleBrowse(const QDomElement &elem, QDomDocument &doc, QDomElement &body, const SHttpServer::RequestMessage &request, const QHostAddress &peerAddress)
{
  const QString path = fromObjectID(elem.firstChildElement("ObjectID").text().toAscii());
  const QString browseFlag = elem.firstChildElement("BrowseFlag").text();
  const unsigned start = elem.firstChildElement("StartingIndex").text().toUInt();
  const unsigned count = elem.firstChildElement("RequestedCount").text().toUInt();
  const QString peer = peerAddress.toString();
  const QString host = request.host();

  QDomElement browseResponse = createElementNS(doc, elem, "BrowseResponse");

  const QString basePath = baseDir(path);
  QMap<QString, Callback *>::Iterator callback = d->callbacks.find(basePath);
  for (QString i=basePath; !i.isEmpty() && (callback == d->callbacks.end()); i=parentDir(i))
    callback = d->callbacks.find(i);

  if ((callback == d->callbacks.end()) || !path.startsWith(callback.key()))
  {
    qDebug() << "SUPnPContentDirectory: could not find callback for path:" << path;
    return false;
  }

  if (path.endsWith('/')) // Directory
  {
    QDomElement result = doc.createElement("Result");
    int totalMatches = 0, totalReturned = 0;

    if (browseFlag == "BrowseDirectChildren")
    {
      QDomDocument subDoc;
      QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
      root.setAttribute("xmlns:dc", dublinCoreNS);
      root.setAttribute("xmlns:dlna", dlnaNS);
      root.setAttribute("xmlns:upnp", metadataNS);

      unsigned itemIndex = start;
      foreach (const Item &item, (*callback)->listContentDirItems(peer, path, start, count))
      {
        if (!item.isDir)
        {
          const QString title = (item.played ? "*" : "") + item.title;
          if (item.direct)
            didlFile(subDoc, root, host, item, path + QString::number(itemIndex), title);
          else
            didlContainer(subDoc, root, Item::Type(item.type), path + QString::number(itemIndex), title, allItems(item, QStringList()).count());
        }
        else
          didlDirectory(subDoc, root, Item::Type(item.type), peer, path + item.title + '/');

        itemIndex++;
        totalReturned++;
      }

      totalMatches = (*callback)->countContentDirItems(peer, path);

      subDoc.appendChild(root);
      result.appendChild(doc.createTextNode(subDoc.toString(-1).replace(">", "&gt;"))); // Crude hack for non-compliant XML parsers
    }
    else if (browseFlag == "BrowseMetadata")
    {
      QDomDocument subDoc;
      QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
      root.setAttribute("xmlns:dc", dublinCoreNS);
      root.setAttribute("xmlns:dlna", dlnaNS);
      root.setAttribute("xmlns:upnp", metadataNS);
      didlDirectory(subDoc, root, Item::Type_None, peer, path);
      subDoc.appendChild(root);

      totalMatches = totalReturned = 1;

      result.appendChild(doc.createTextNode(subDoc.toString(-1).replace(">", "&gt;"))); // Crude hack for non-compliant XML parsers
    }

    browseResponse.appendChild(result);

    addTextElm(doc, browseResponse, "NumberReturned", QString::number(totalReturned));
    addTextElm(doc, browseResponse, "TotalMatches", QString::number(totalMatches));
  }
  else
  {
    const QString file = path.mid(basePath.length());
    const QStringList itemProps = splitItemProps(file);
    const unsigned itemIndex = itemProps.first().toUInt();

    // Get the item
    Item item;
    foreach (const Item &i, (*callback)->listContentDirItems(peer, basePath, itemIndex, 1))
      item = i;

    if (item.isNull())
    {
      qDebug() << "SUPnPContentDirectory: could not find item" << itemIndex << "in path:" << basePath;
      return false;
    }

    const QStringList items = allItems(item, itemProps);
    QDomElement result = doc.createElement("Result");
    int totalMatches = 0, totalReturned = 0;

    if (browseFlag == "BrowseDirectChildren")
    {
      QDomDocument subDoc;
      QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
      root.setAttribute("xmlns:dc", dublinCoreNS);
      root.setAttribute("xmlns:dlna", dlnaNS);
      root.setAttribute("xmlns:upnp", metadataNS);

      // Only select the items that were requested.
      for (int i=start, n=0; (i<items.count()) && ((count == 0) || (n<int(count))); i++, n++)
      {
        const QStringList props = splitItemProps(file + '|' + items[i]);
        if (props[1] == "p")
          didlFile(subDoc, root, host, makePlayItem(item, props), path + '|' + items[i]);
        else
          didlContainer(subDoc, root, Item::Type(item.type), path + '|' + items[i], props[3], allItems(item, splitItemProps(items[i])).count());

        totalReturned++;
      }

      totalMatches = items.count();

      subDoc.appendChild(root);
      result.appendChild(doc.createTextNode(subDoc.toString(-1).replace(">", "&gt;"))); // Crude hack for non-compliant XML parsers
    }
    else if (browseFlag == "BrowseMetadata")
    {
      QDomDocument subDoc;
      QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
      root.setAttribute("xmlns:dc", dublinCoreNS);
      root.setAttribute("xmlns:dlna", dlnaNS);
      root.setAttribute("xmlns:upnp", metadataNS);

      if (itemProps[1].isEmpty() || (itemProps[1] == "p"))
        didlFile(subDoc, root, host, makePlayItem(item, itemProps), path);
      else
        didlContainer(subDoc, root, Item::Type(item.type), path, itemProps[3], items.count());

      totalMatches = totalReturned = 1;

      subDoc.appendChild(root);
      result.appendChild(doc.createTextNode(subDoc.toString(-1).replace(">", "&gt;"))); // Crude hack for non-compliant XML parsers
    }

    browseResponse.appendChild(result);
    addTextElm(doc, browseResponse, "NumberReturned", QString::number(totalReturned));
    addTextElm(doc, browseResponse, "TotalMatches", QString::number(totalMatches));
  }

  addTextElm(doc, browseResponse, "UpdateID", QString::number(d->systemUpdateId));
  body.appendChild(browseResponse);

  return true;
}

void SUPnPContentDirectory::didlDirectory(QDomDocument &doc, QDomElement &root, Item::Type type, const QString &peer, const QString &path, const QString &title)
{
  QMap<QString, Callback *>::Iterator callback = d->callbacks.find(path);
  for (QString i=path; !i.isEmpty() && (callback == d->callbacks.end()); i=parentDir(i))
    callback = d->callbacks.find(i);

  if ((callback == d->callbacks.end()) || !path.startsWith(callback.key()))
  {
    qDebug() << "SUPnPContentDirectory: could not find callback for path:" << path;
    return;
  }

  didlContainer(doc, root, type, path, title, (*callback)->countContentDirItems(peer, path));
}

void SUPnPContentDirectory::didlContainer(QDomDocument &doc, QDomElement &root, Item::Type type, const QString &path, const QString &title, int childCount)
{
  const QString parentPath = parentDir(path);
  const QString dcTitle =
      !title.isEmpty()
          ? title
          : (!parentPath.isEmpty()
                ? path.mid(parentPath.length(), path.length() - parentPath.length() - 1)
                : QString("root"));

  QDomElement containerElm = doc.createElement("container");
  containerElm.setAttribute("id", QString::fromAscii(toObjectID(path)));
  containerElm.setAttribute("parentID", QString::fromAscii(toObjectID(parentPath)));
  containerElm.setAttribute("restricted", "1");

  if (childCount > 0)
    containerElm.setAttribute("childCount", QString::number(childCount));

  addTextElm(doc, containerElm, "dc:title", dcTitle);

  switch (type)
  {
  case Item::Type_None:           addTextElm(doc, containerElm, "upnp:class", "object.container.album"); break;
  case Item::Type_Playlist:       addTextElm(doc, containerElm, "upnp:class", "object.container.playlistContainer"); break;

  case Item::Type_Audio:
  case Item::Type_AudioBroadcast:
  case Item::Type_AudioBook:      addTextElm(doc, containerElm, "upnp:class", "object.container.album"); break;
  case Item::Type_Music:          addTextElm(doc, containerElm, "upnp:class", "object.container.album.musicAlbum"); break;

  case Item::Type_Video:
  case Item::Type_Movie:
  case Item::Type_VideoBroadcast: addTextElm(doc, containerElm, "upnp:class", "object.container.album"); break;
  case Item::Type_MusicVideo:     addTextElm(doc, containerElm, "upnp:class", "object.container.album.musicAlbum"); break;

  case Item::Type_Image:          addTextElm(doc, containerElm, "upnp:class", "object.container.album"); break;
  case Item::Type_Photo:          addTextElm(doc, containerElm, "upnp:class", "object.container.album.photoAlbum"); break;
  }

  root.appendChild(containerElm);
}

void SUPnPContentDirectory::didlFile(QDomDocument &doc, QDomElement &root, const QString &host, const Item &item, const QString &path, const QString &title)
{
  QDomElement itemElm = doc.createElement("item");
  itemElm.setAttribute("id", QString::fromAscii(toObjectID(path)));
  itemElm.setAttribute("restricted", "1");
  itemElm.setAttribute("parentID", QString::fromAscii(toObjectID(parentDir(path))));

  addTextElm(doc, itemElm, "dc:title", !title.isEmpty() ? title : item.title);

  if (!item.artist.isEmpty())
    addTextElm(doc, itemElm, "upnp:artist", item.artist);

  if (!item.album.isEmpty())
    addTextElm(doc, itemElm, "upnp:album", item.album);

  if (!item.iconUrl.isEmpty())
  {
    QUrl url = item.iconUrl;
    url.setScheme("http");
    url.setAuthority(host);

    QDomElement iconElm = doc.createElement("upnp:albumArtURI");

    if (url.path().endsWith(".png"))
      iconElm.setAttribute("dlna:profileID", "PNG_SM");
    else if (url.path().endsWith(".jpeg") || url.path().endsWith(".jpg"))
      iconElm.setAttribute("dlna:profileID", "JPEG_TN");

    iconElm.appendChild(doc.createTextNode(url.toString()));
    itemElm.appendChild(iconElm);
  }

  switch (item.type)
  {
  case Item::Type_None:           addTextElm(doc, itemElm, "upnp:class", "object.item"); break;
  case Item::Type_Playlist:       addTextElm(doc, itemElm, "upnp:class", "object.item.playlistItem"); break;

  case Item::Type_Audio:          addTextElm(doc, itemElm, "upnp:class", "object.item.audioItem"); break;
  case Item::Type_Music:          addTextElm(doc, itemElm, "upnp:class", "object.item.audioItem.musicTrack"); break;
  case Item::Type_AudioBroadcast: addTextElm(doc, itemElm, "upnp:class", "object.item.audioItem.audioBroadcast"); break;
  case Item::Type_AudioBook:      addTextElm(doc, itemElm, "upnp:class", "object.item.audioItem.audioBook"); break;

  case Item::Type_Video:          addTextElm(doc, itemElm, "upnp:class", "object.item.videoItem"); break;
  case Item::Type_Movie:          addTextElm(doc, itemElm, "upnp:class", "object.item.videoItem.movie"); break;
  case Item::Type_VideoBroadcast: addTextElm(doc, itemElm, "upnp:class", "object.item.videoItem.videoBroadcast"); break;
  case Item::Type_MusicVideo:     addTextElm(doc, itemElm, "upnp:class", "object.item.videoItem.musicVideoClip"); break;

  case Item::Type_Image:          addTextElm(doc, itemElm, "upnp:class", "object.item.imageItem"); break;
  case Item::Type_Photo:          addTextElm(doc, itemElm, "upnp:class", "object.item.imageItem.photo"); break;
  }

  foreach (const Protocol &protocol, item.protocols)
  {
    QDomElement resElm = doc.createElement("res");
    resElm.setAttribute("protocolInfo", QString(protocol.toByteArray()));

    if (item.duration > 0)
      resElm.setAttribute("duration", QTime().addSecs(item.duration).toString("h:mm:ss.zzz"));

    QUrl url = item.url;
    url.setScheme("http");
    url.setAuthority(host);
    url.addQueryItem("contentFeatures", protocol.contentFeatures().toHex());

    // Encode the filename
    const QString path = toQueryPath(url.toEncoded(QUrl::RemoveQuery), url.encodedQuery(), protocol.suffix);
    resElm.appendChild(doc.createTextNode(path));
    itemElm.appendChild(resElm);
  }

  root.appendChild(itemElm);
}

void SUPnPContentDirectory::emitEvent(bool dirty)
{
  QDomDocument doc;
  QDomElement propertySet = doc.createElementNS(d->genaServer->eventNS, "e:propertyset");

  QDomElement property = createElementNS(doc, propertySet, "property");
  addTextElm(doc, property, "SystemUpdateID", QString::number(d->systemUpdateId.fetchAndAddRelaxed(dirty ? 1 : 0)));
  propertySet.appendChild(property);

  doc.appendChild(propertySet);

  d->genaServer->emitEvent(doc);
}

QStringList SUPnPContentDirectory::allItems(const Item &item, const QStringList &itemProps)
{
  QStringList items;
  if (itemProps.isEmpty() || itemProps[1].isEmpty()) // Root
    items = streamItems(item);
  else if (itemProps[1] == "r")
    items = playSeekItems(item);
  else if (itemProps[1] == "s")
    items = seekItems(item);
  else if (itemProps[1] == "c")
    items = chapterItems(item);

  return items;
}

QStringList SUPnPContentDirectory::streamItems(const Item &item)
{
  if (item.streams.count() > 1)
  {
    QStringList result;
    foreach (const Item::Stream &stream, item.streams)
    {
      QString query;

      for (int i=0; i<stream.queryItems.count(); i++)
        query += '&' + stream.queryItems[i].first + '=' + stream.queryItems[i].second;

      result += ("r" + query + "#" + stream.title);
    }

    return result;
  }

  return playSeekItems(item);
}

QStringList SUPnPContentDirectory::playSeekItems(const Item &item)
{
  QStringList result;

  result += ("p#" + tr("Play"));

  if (item.chapters.count() > 1)
    result += ("c#" + tr("Chapters"));

  if (item.duration > 0)
    result += ("s#" + tr("Seek"));

  return result;
}

QStringList SUPnPContentDirectory::seekItems(const Item &item)
{
  QStringList result;

  for (unsigned i=0; i<item.duration; i+=seekSec)
  {
    QString title = tr("Play from") + " " + QTime().addSecs(i).toString("h:mm");

    result += ("p&position=" + QString::number(i) + "#" + title);
  }

  return result;
}

QStringList SUPnPContentDirectory::chapterItems(const Item &item)
{
  QStringList result;

  int chapterNum = 1;
  foreach (const Item::Chapter &chapter, item.chapters)
  {
    QString title = tr("Chapter") + " " + QString::number(chapterNum++);
    if (!chapter.title.isEmpty())
      title += ", " + chapter.title;

    result += ("p&position=" + QString::number(chapter.position) + "#" + title);
  }

  return result;
}

QStringList SUPnPContentDirectory::splitItemProps(const QString &text)
{
  QStringList itemProps = QStringList() << QString::null << QString::null << QString::null << QString::null;
  foreach (const QString &section, text.split('|'))
  {
    const int hash = section.indexOf('#');
    if (hash > 0)
    {
      itemProps[1] = section.left(1);
      itemProps[2] += section.mid(1, hash - 1);
      itemProps[3] = section.mid(hash + 1);
    }
    else
      itemProps[0] = section.isEmpty() ? itemProps[0] : section;
  }

  return itemProps;
}

SUPnPContentDirectory::Item SUPnPContentDirectory::makePlayItem(const Item &baseItem, const QStringList &itemProps)
{
  Item item = baseItem;

  if (!itemProps[3].isEmpty())
    item.title = itemProps[3];

  if (!itemProps[2].isEmpty())
  foreach (const QString &qi, itemProps[2].split('&'))
  {
    const QStringList qil = qi.split('=');
    if (qil.count() == 2)
      item.url.addQueryItem(qil[0], qil[1]);
  }

  return item;
}

QString SUPnPContentDirectory::baseDir(const QString &dir)
{
  return dir.left(dir.lastIndexOf('/') + 1);
}

QString SUPnPContentDirectory::parentDir(const QString &dir)
{
  if (!dir.isEmpty())
  {
    const int last =
        qMax(dir.left(dir.length() - 1).lastIndexOf('/'),
             dir.left(dir.length() - 1).lastIndexOf('|') - 1);

    if (last >= 0)
      return dir.left(last + 1);
  }

  return QString::null;
}

QByteArray SUPnPContentDirectory::toObjectID(const QString &path)
{
  if (path == "/")
  {
    return "0";
  }
  else if (path.isEmpty())
  {
    return "-1";
  }
  else
  {
#ifdef USE_SMALL_OBJECTIDS
    QHash<QByteArray, qint32>::ConstIterator i = d->objectIdHash.find(path.toUtf8());
    if (i != d->objectIdHash.end())
      return QByteArray::number(*i);

    d->objectIdList.append(path.toUtf8());
    d->objectIdList.last().squeeze();
    d->objectIdHash.insert(d->objectIdList.last(), d->objectIdList.count() - 1);

    return QByteArray::number(d->objectIdList.count() - 1);
#else
    return qCompress(path.toUtf8(), 9).toBase64();
#endif
  }
}

QString SUPnPContentDirectory::fromObjectID(const QByteArray &idStr)
{
  if (idStr == "0")
  {
    return "/";
  }
  else if (idStr == "-1")
  {
    return QString::null;
  }
  else
  {
#ifdef USE_SMALL_OBJECTIDS
    const qint32 id = idStr.toInt();
    if (id < d->objectIdList.count())
      return QString::fromUtf8(d->objectIdList[id]);

    return QString::null;
#else
    return QString::fromUtf8(qUncompress(QByteArray::fromBase64(idStr)));
#endif
  }
}


SUPnPContentDirectory::Item::Item(void)
  : isDir(false), played(false), direct(false), type(Type_None),
    track(0), duration(0)
{
}

SUPnPContentDirectory::Item::~Item()
{
}

bool SUPnPContentDirectory::Item::isNull(void) const
{
  return url.isEmpty();
}

bool SUPnPContentDirectory::Item::isAudio(void) const
{
  return (type >= Type_Audio) && (type < Type_Video);
}

bool SUPnPContentDirectory::Item::isVideo(void) const
{
  return (type >= Type_Video) && (type < Type_Image);
}

bool SUPnPContentDirectory::Item::isImage(void) const
{
  return (type >= Type_Image) && (type <= Type_Photo);
}

bool SUPnPContentDirectory::Item::isMusic(void) const
{
  return (type == Type_Music) || (type == Type_MusicVideo);
}


SUPnPContentDirectory::Item::Stream::Stream(void)
{
}

SUPnPContentDirectory::Item::Stream::~Stream()
{
}


SUPnPContentDirectory::Item::Chapter::Chapter(const QString &title, unsigned position)
  : title(title), position(position)
{
}

SUPnPContentDirectory::Item::Chapter::~Chapter()
{
}


int SUPnPContentDirectory::Data::countContentDirItems(const QString &peer, const QString &path)
{
  QSet<QString> subDirs;

  for (QMap<QString, Callback *>::ConstIterator i=callbacks.begin(); i!=callbacks.end(); i++)
  if (i.key().startsWith(path))
  {
    QString sub = i.key().mid(path.length() - 1);
    sub = sub.left(sub.indexOf('/', 1) + 1);
    if ((sub.length() > 1) && !subDirs.contains(sub) &&
        ((*i)->countContentDirItems(peer, i.key()) > 0))
    {
      subDirs.insert(sub);
    }
  }

  return subDirs.count();
}

QList<SUPnPContentDirectory::Item> SUPnPContentDirectory::Data::listContentDirItems(const QString &peer, const QString &path, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;
  QList<SUPnPContentDirectory::Item> result;
  QSet<QString> names;

  for (QMap<QString, Callback *>::ConstIterator i=callbacks.begin(); i!=callbacks.end(); i++)
  if (i.key().startsWith(path))
  {
    QString sub = i.key().mid(path.length() - 1);
    sub = sub.left(sub.indexOf('/', 1) + 1);
    if ((sub.length() > 1) && !names.contains(sub) &&
        ((*i)->countContentDirItems(peer, i.key()) > 0))
    {
      names.insert(sub);

      if (returnAll || (count > 0))
      {
        if (start == 0)
        {
          Item item;
          item.isDir = true;
          item.title = sub;
          item.title = item.title.startsWith('/') ? item.title.mid(1) : item.title;
          item.title = item.title.endsWith('/') ? item.title.left(item.title.length() - 1) : item.title;

          result += item;
          if (count > 0)
            count--;
        }
        else
          start--;
      }
    }
  }

  return result;
}

} // End of namespace
