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

#include "upnpcontentdirectory.h"
#include "upnpgenaserver.h"

namespace LXiServer {

const char  * const UPnPContentDirectory::contentDirectoryNS = "urn:schemas-upnp-org:service:ContentDirectory:1";
const unsigned  UPnPContentDirectory::seekSec = 120;

struct UPnPContentDirectory::Data : UPnPContentDirectory::Callback
{
  virtual int                   countDlnaItems(const QString &path);
  virtual QList<Item>           listDlnaItems(const QString &path, unsigned start, unsigned count);

  UPnPGenaServer              * genaServer;

  QMap<Item::Type, ProtocolList> protocols;
  QMap<QString, QMap<QString, QString> > queryItems;
  QMap<QString, QString>        activeClients;

  QMap<ItemID, ItemData>        itemData;
  volatile ItemID               idCounter;
  QMap<QString, Callback *>     callbacks;
  QAtomicInt                    systemUpdateId;
};

UPnPContentDirectory::UPnPContentDirectory(const QString &basePath, QObject *parent)
    : UPnPBase(basePath + "contentdirectory/", parent),
      d(new Data())
{
  d->genaServer = new UPnPGenaServer(UPnPBase::basePath(), this);

  d->idCounter = Q_UINT64_C(0x8000000000000001); // First object ID
  d->systemUpdateId = 1;

  // Add the root path.
  ItemData itemData;
  itemData.path = "/";
  itemData.itemID = 0;
  itemData.item.isDir = true;

  d->itemData.insert(itemData.itemID, itemData);
  d->callbacks.insert(itemData.path, d);
}

UPnPContentDirectory::~UPnPContentDirectory()
{
  delete d->genaServer;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void UPnPContentDirectory::initialize(HttpServer *httpServer, UPnPMediaServer *mediaServer)
{
  d->genaServer->initialize(httpServer);
  emitEvent(false); // To build the initial event message

  UPnPMediaServer::Service service;
  UPnPBase::initialize(httpServer, service);
  service.serviceType = contentDirectoryNS;
  service.serviceId = "urn:upnp-org:serviceId:ContentDirectory";
  service.eventSubURL = d->genaServer->path();
  mediaServer->registerService(service);
}

void UPnPContentDirectory::close(void)
{
  UPnPBase::close();
  d->genaServer->close();
}

void UPnPContentDirectory::setProtocols(Item::Type type, const ProtocolList &protocols)
{
  QWriteLocker l(lock());

  d->protocols[type] = protocols;
}

void UPnPContentDirectory::setQueryItems(const QString &peer, const QMap<QString, QString> &queryItems)
{
  QWriteLocker l(lock());

  d->queryItems[peer] = queryItems;
}

QMap<QString, QString> UPnPContentDirectory::activeClients(void) const
{
  QReadLocker l(lock());

  QMap<QString, QString> result = d->activeClients;

  d->activeClients.clear();

  return result;
}

void UPnPContentDirectory::registerCallback(const QString &path, Callback *callback)
{
  QWriteLocker l(lock());

  d->callbacks.insert(path, callback);
}

void UPnPContentDirectory::unregisterCallback(Callback *callback)
{
  QWriteLocker l(lock());

  for (QMap<QString, Callback *>::Iterator i=d->callbacks.begin(); i!=d->callbacks.end(); )
  if (i.value() == callback)
    i = d->callbacks.erase(i);
  else
    i++;
}

void UPnPContentDirectory::modified(void)
{
  emitEvent(true);
}

void UPnPContentDirectory::buildDescription(QDomDocument &doc, QDomElement &scpdElm)
{
  QDomElement actionListElm = doc.createElement("actionList");
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
  scpdElm.appendChild(actionListElm);

  QDomElement serviceStateTableElm = doc.createElement("serviceStateTable");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_ObjectID", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Result", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_BrowseFlag", "string", QStringList() << "BrowseMetadata" << "BrowseDirectChildren");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Filter", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_SortCriteria", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Index", "ui4");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Count", "ui4");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_UpdateID", "ui4");
  addStateVariable(doc, serviceStateTableElm, false, "SearchCapabilities", "string");
  addStateVariable(doc, serviceStateTableElm, false, "SortCapabilities", "string");
  addStateVariable(doc, serviceStateTableElm, true, "SystemUpdateID", "ui4");
  scpdElm.appendChild(serviceStateTableElm);
}

void UPnPContentDirectory::handleSoapMessage(const QDomElement &body, QDomDocument &responseDoc, QDomElement &responseBody, const HttpServer::RequestHeader &request, const QHostAddress &peerAddress)
{
  const QDomElement browseElm = firstChildElementNS(body, contentDirectoryNS, "Browse");
  if (!browseElm.isNull())
    handleBrowse(browseElm, responseDoc, responseBody, request, peerAddress);

  const QDomElement getSearchCapabilitiesElm = firstChildElementNS(body, contentDirectoryNS, "GetSearchCapabilities");
  if (!getSearchCapabilitiesElm.isNull())
  {
    QDomElement response = createElementNS(responseDoc, getSearchCapabilitiesElm, "GetSearchCapabilitiesResponse");
    addTextElm(responseDoc, response, "SearchCaps", "");
    responseBody.appendChild(response);
  }

  const QDomElement getSortCapabilitiesElm = firstChildElementNS(body, contentDirectoryNS, "GetSortCapabilities");
  if (!getSortCapabilitiesElm.isNull())
  {
    QDomElement response = createElementNS(responseDoc, getSortCapabilitiesElm, "GetSortCapabilitiesResponse");
    addTextElm(responseDoc, response, "SortCaps", "");
    responseBody.appendChild(response);
  }

  const QDomElement getSystemUpdateIdElm = firstChildElementNS(body, contentDirectoryNS, "GetSystemUpdateID");
  if (!getSystemUpdateIdElm.isNull())
  {
    QDomElement response = createElementNS(responseDoc, getSystemUpdateIdElm, "GetSystemUpdateIDResponse");
    addTextElm(responseDoc, response, "Id", QString::number(d->systemUpdateId));
    responseBody.appendChild(response);
  }
}

void UPnPContentDirectory::handleBrowse(const QDomElement &elem, QDomDocument &doc, QDomElement &body, const HttpServer::RequestHeader &request, const QHostAddress &peerAddress)
{
  const QDomElement objectId = elem.firstChildElement("ObjectID");
  const QDomElement browseFlag = elem.firstChildElement("BrowseFlag");
  //const QDomElement filter = elem.firstChildElement("Filter");
  const QDomElement startingIndex = elem.firstChildElement("StartingIndex");
  const QDomElement requestedCount = elem.firstChildElement("RequestedCount");

  // Find requested directory
  QWriteLocker l(lock());

  d->activeClients[peerAddress.toString()] = request.field("User-Agent");

  const ItemID pathId = fromIDString(objectId.text().trimmed());
  QMap<ItemID, ItemData>::Iterator itemData = d->itemData.find(pathId);
  if (itemData == d->itemData.end())
    return;

  QDomElement browseResponse = createElementNS(doc, elem, "BrowseResponse");
  addTextElm(doc, browseResponse, "UpdateID", QString::number(d->systemUpdateId));

  if (itemData->item.isDir)
  {
    QString dir = itemData->path;
    QMap<QString, Callback *>::Iterator callback = d->callbacks.find(dir);
    while ((callback == d->callbacks.end()) && !dir.isEmpty())
    {
      dir = dir.left(dir.left(dir.length() - 1).lastIndexOf('/') + 1);
      callback = d->callbacks.find(dir);
    }

    if ((callback == d->callbacks.end()) || !itemData->path.startsWith(callback.key()))
      return;

    browseDir(
        doc, browseResponse,
        *itemData, *callback, peerAddress.toString(), request.host(),
        browseFlag.text(), startingIndex.text().toUInt(), requestedCount.text().toUInt());
  }
  else
  {
    browseFile(
        doc, browseResponse,
        *itemData, peerAddress.toString(), request.host(),
        browseFlag.text(), startingIndex.text().toUInt(), requestedCount.text().toUInt());
  }

  body.appendChild(browseResponse);
}

void UPnPContentDirectory::browseDir(QDomDocument &doc, QDomElement &browseResponse, ItemData &itemData, Callback *callback, const QString &peer, const QString &host, const QString &browseFlag, unsigned start, unsigned count)
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

    foreach (const Item &item, callback->listDlnaItems(itemData.path, start, count))
    {
      if (!item.isDir && (item.mode == Item::Mode_Direct))
        root.appendChild(didlFile(subDoc, peer, host, addChildItem(itemData, item, true)));
      else
        root.appendChild(didlDirectory(subDoc, addChildItem(itemData, item, true)));

      totalReturned++;
    }

    totalMatches = callback->countDlnaItems(itemData.path);

    subDoc.appendChild(root);
    result.appendChild(doc.createTextNode(subDoc.toString(-1)));
  }
  else if (browseFlag == "BrowseMetadata")
  {
    QDomDocument subDoc;
    QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", dublinCoreNS);
    root.setAttribute("xmlns:dlna", dlnaNS);
    root.setAttribute("xmlns:upnp", metadataNS);
    subDoc.appendChild(root);

    if (!itemData.item.isDir &&
        ((itemData.item.audioStreams.isEmpty() &&
          itemData.item.videoStreams.isEmpty() &&
          itemData.item.subtitleStreams.isEmpty()) ||
         ((itemData.item.type & 0x7F) != Item::Type_Video)))
    {
      root.appendChild(didlFile(subDoc, peer, host, itemData));
    }
    else
      root.appendChild(didlDirectory(subDoc, itemData));

    totalMatches = totalReturned = 1;

    result.appendChild(doc.createTextNode(subDoc.toString(-1)));
  }

  browseResponse.appendChild(result);
  addTextElm(doc, browseResponse, "NumberReturned", QString::number(totalReturned));
  addTextElm(doc, browseResponse, "TotalMatches", QString::number(totalMatches));
}

void UPnPContentDirectory::browseFile(QDomDocument &doc, QDomElement &browseResponse, ItemData &itemData, const QString &peer, const QString &host, const QString &browseFlag, unsigned start, unsigned count)
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
    subDoc.appendChild(root);

    QVector<ItemData> all;
    switch (Item::Mode(itemData.item.mode))
    {
    case Item::Mode_Default:
      if (!itemData.item.audioStreams.isEmpty() ||
          !itemData.item.videoStreams.isEmpty() ||
          !itemData.item.subtitleStreams.isEmpty())
      {
        const QList<Item::Stream> audioStreams = itemData.item.audioStreams;
        const QList<Item::Stream> dataStreams =
            itemData.item.subtitleStreams <<
            Item::Stream(0x0000FFFF);

        if ((audioStreams.count() > 1) || (dataStreams.count() > 1))
        {
          for (int a=0, an=audioStreams.count(); a < an; a++)
          for (int d=0, dn=dataStreams.count(); d < dn; d++)
          {
            Item item = itemData.item;
            item.played = false;
            item.mode = Item::Mode_PlaySeek;

            item.title = QString::number(a + 1) + ". ";
            if (!audioStreams[a].lang.isEmpty())
              item.title += audioStreams[a].lang;
            else
              item.title += tr("Unknown");

            item.url.addQueryItem("language", QString::number(audioStreams[a].id, 16));

            if (dataStreams[d].id != 0x0000FFFF)
            {
              item.title += ", " + QString::number(d + 1) + ". ";
              if (!dataStreams[d].lang.isEmpty())
                item.title += dataStreams[d].lang;
              else
                item.title += tr("Unknown");

              item.url.addQueryItem("subtitles", QString::number(dataStreams[d].id, 16));
            }

            all += addChildItem(itemData, item, true);
          }

          break;
        }
      }
      // Deliberately no break.

    case Item::Mode_PlaySeek:
      {
        Item playItem = itemData.item;
        playItem.played = false;
        playItem.mode = Item::Mode_Direct;
        playItem.title = tr("Play");
        all += addChildItem(itemData, playItem, false);

        if (itemData.item.duration > 0)
        {
          Item seekItem = itemData.item;
          seekItem.played = false;
          seekItem.mode = Item::Mode_Seek;
          seekItem.title = tr("Seek");
          all += addChildItem(itemData, seekItem, true);
        }

        if (itemData.item.chapters.count() > 1)
        {
          Item seekItem = itemData.item;
          seekItem.played = false;
          seekItem.mode = Item::Mode_Chapters;
          seekItem.title = tr("Chapters");
          all += addChildItem(itemData, seekItem, true);
        }
      }
      break;

    case Item::Mode_Seek:
      for (unsigned i=0; i<itemData.item.duration; i+=seekSec)
      {
        Item item = itemData.item;
        item.played = false;
        item.mode = Item::Mode_Direct;
        item.title = tr("Play from") + " " + QTime().addSecs(i).toString("h:mm");
        item.url.addQueryItem("position", QString::number(i));
        all += addChildItem(itemData, item, false);
      }
      break;

    case Item::Mode_Chapters:
      {
        int chapterNum = 1;
        foreach (const Item::Chapter &chapter, itemData.item.chapters)
        {
          Item item = itemData.item;
          item.played = false;
          item.mode = Item::Mode_Direct;

          item.title = tr("Chapter") + " " + QString::number(chapterNum++);
          if (!chapter.title.isEmpty())
            item.title += ", " + chapter.title;

          item.url.addQueryItem("position", QString::number(chapter.position));
          all += addChildItem(itemData, item, false);
        }
      }
      break;

    case Item::Mode_Direct:
      break;
    }

    // Only select the items that were requested.
    for (int i=start, n=0; (i<all.count()) && ((count == 0) || (n<int(count))); i++, n++)
    {
      if (all[i].item.mode != Item::Mode_Direct)
        root.appendChild(didlDirectory(subDoc, all[i]));
      else
        root.appendChild(didlFile(subDoc, peer, host, all[i]));

      totalReturned++;
    }

    totalMatches = all.count();

    result.appendChild(doc.createTextNode(subDoc.toString(-1)));
  }
  else if (browseFlag == "BrowseMetadata")
  {
    QDomDocument subDoc;
    QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", dublinCoreNS);
    root.setAttribute("xmlns:dlna", dlnaNS);
    root.setAttribute("xmlns:upnp", metadataNS);
    subDoc.appendChild(root);

    if (itemData.item.mode != Item::Mode_Direct)
      root.appendChild(didlDirectory(subDoc, itemData));
    else
      root.appendChild(didlFile(subDoc, peer, host, itemData));

    totalMatches = totalReturned = 1;

    result.appendChild(doc.createTextNode(subDoc.toString(-1)));
  }

  browseResponse.appendChild(result);
  addTextElm(doc, browseResponse, "NumberReturned", QString::number(totalReturned));
  addTextElm(doc, browseResponse, "TotalMatches", QString::number(totalMatches));
}

UPnPContentDirectory::ItemData UPnPContentDirectory::addChildItem(ItemData &itemData, const Item &item, bool asDir)
{
  QWriteLocker l(lock());

  const QString fullPath = itemData.path + item.title + (asDir ? "/" : "");

  // Find existing child
  foreach (ItemID childId, itemData.children)
  {
    QMap<ItemID, ItemData>::Iterator childData = d->itemData.find(childId);
    if (childData != d->itemData.end())
    if (childData->path == fullPath)
    {
      childData->item = item;
      return *childData;
    }
  }

  // Add new child
  ItemData childData;
  childData.path = fullPath;
  childData.itemID = d->idCounter++;
  childData.parentID = itemData.itemID;
  childData.item = item;
  d->itemData.insert(childData.itemID, childData);

  itemData.children.append(childData.itemID);

  return childData;
}

QDomElement UPnPContentDirectory::didlDirectory(QDomDocument &doc, const ItemData &dir) const
{
  QDomElement containerElm = doc.createElement("container");
  containerElm.setAttribute("id", toIDString(dir.itemID));
  containerElm.setAttribute("restricted", "true");
  containerElm.setAttribute("parentID", toIDString(dir.parentID));

  addTextElm(doc, containerElm, "dc:title", (dir.item.played ? "*" : "") + dir.item.title);
  addTextElm(doc, containerElm, "upnp:class", "object.container.storageFolder");

  return containerElm;
}

QDomElement UPnPContentDirectory::didlFile(QDomDocument &doc, const QString &peer, const QString &host, const ItemData &file) const
{
  QDomElement itemElm = doc.createElement("item");
  itemElm.setAttribute("id", toIDString(file.itemID));
  itemElm.setAttribute("restricted", "true");
  itemElm.setAttribute("parentID", toIDString(file.parentID));

  addTextElm(doc, itemElm, "dc:title", (file.item.played ? "*" : "") + file.item.title);

  if (!file.item.artist.isEmpty())   addTextElm(doc, itemElm, "upnp:artist", file.item.artist);
  if (!file.item.album.isEmpty())    addTextElm(doc, itemElm, "upnp:album", file.item.album);
  if (!file.item.iconUrl.isEmpty())  addTextElm(doc, itemElm, "upnp:icon", file.item.iconUrl.toString());

  QUrl url = file.item.url;
  url.setScheme("http");
  url.setAuthority(host);
  url.addQueryItem("music", (file.item.type & Item::Type_FlagMusic) ? "true" : "false");

  QMap<QString, QMap<QString, QString> >::ConstIterator peerItems = d->queryItems.find(peer);
  if (peerItems == d->queryItems.end())
    peerItems = d->queryItems.find(QString::null);

  if (peerItems != d->queryItems.end())
  for (QMap<QString, QString>::ConstIterator i = peerItems->begin(); i != peerItems->end(); i++)
    url.addQueryItem(i.key(), i.value());

  QList<Item::Type> types;
  switch (Item::Type(file.item.type & 0x7f))
  {
  case Item::Type_Audio:
    addTextElm(doc, itemElm, "upnp:class", "object.item.audioItem");
    types += Item::Type_Audio;
    break;

  case Item::Type_Video:
    addTextElm(doc, itemElm, "upnp:class", "object.item.videoItem");
    types += Item::Type_Video;

    if (file.item.type & Item::Type_FlagMusic)
      types += Item::Type_Audio;

    break;

  case Item::Type_Image:
    addTextElm(doc, itemElm, "upnp:class", "object.item.imageItem");
    types += Item::Type_Image;
    break;

  default:
    addTextElm(doc, itemElm, "upnp:class", "object.item");
    break;
  }

  foreach (Item::Type type, types)
  foreach (const Protocol &protocol, d->protocols[type])
  {
    QDomElement resElm = doc.createElement("res");
    resElm.setAttribute("protocolInfo", protocol.toString());

    if (file.item.duration > 0)
      resElm.setAttribute("duration", QTime().addSecs(file.item.duration).toString("h:mm:ss.zzz"));

    QUrl u = url;
    u.setPath(u.path() + protocol.suffix);

    for (QMap<QString, QString>::ConstIterator i = protocol.queryItems.begin();
         i != protocol.queryItems.end();
         i++)
    {
      u.removeQueryItem(i.key());
      u.addQueryItem(i.key(), i.value());
    }

    resElm.appendChild(doc.createTextNode(u.toString()));
    itemElm.appendChild(resElm);
  }

  return itemElm;
}

void UPnPContentDirectory::emitEvent(bool dirty)
{
  QDomDocument doc;
  QDomElement propertySet = doc.createElementNS(d->genaServer->eventNS, "e:propertyset");

  QDomElement property = createElementNS(doc, propertySet, "property");
  addTextElm(doc, property, "SystemUpdateID", QString::number(d->systemUpdateId.fetchAndAddRelaxed(dirty ? 1 : 0)));
  propertySet.appendChild(property);

  doc.appendChild(propertySet);

  d->genaServer->emitEvent(doc);
}

QString UPnPContentDirectory::toIDString(ItemID id)
{
  if (id != ItemID(Q_INT64_C(-1)))
    return QString::number(id, 16);

  return "-1";
}

UPnPContentDirectory::ItemID UPnPContentDirectory::fromIDString(const QString &id)
{
  if (id != "-1")
    return id.toULongLong(NULL, 16);

  return ItemID(Q_INT64_C(-1));
}

int UPnPContentDirectory::Data::countDlnaItems(const QString &)
{
  return callbacks.count() - 1;
}

QList<UPnPContentDirectory::Item> UPnPContentDirectory::Data::listDlnaItems(const QString &, unsigned start, unsigned count)
{
  QList<UPnPContentDirectory::Item> result;

  for (QMap<QString, Callback *>::ConstIterator i=callbacks.begin(); i!=callbacks.end(); i++)
  if (i.key() != "/")
  {
    UPnPContentDirectory::Item item;
    item.isDir = true;
    item.title = i.key();
    item.title = item.title.startsWith('/') ? item.title.mid(1) : item.title;
    item.title = item.title.endsWith('/') ? item.title.left(item.title.length() - 1) : item.title;

    result += item;
  }

  if (count > 0)
  {
    while ((start > 0) && !result.isEmpty())
    {
      result.takeFirst();
      start--;
    }

    if (result.count() >= int(count))
    {
      while (result.count() > int(count))
        result.takeAt(count);

      count = 0;
    }
    else
      count -= result.count();
  }

  return result;
}

} // End of namespace
