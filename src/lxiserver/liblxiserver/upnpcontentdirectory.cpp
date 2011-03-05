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

namespace LXiServer {

const char  * const UPnPContentDirectory::contentDirectoryNS = "urn:schemas-upnp-org:service:ContentDirectory:1";
const unsigned  UPnPContentDirectory::seekSec = 120;

struct UPnPContentDirectory::ItemData
{
  inline                        ItemData(void) : updateId(0)                    { }

  QString                       path;
  Item                          item;
  QVector<ItemID>               children;
  unsigned                      updateId;
};

struct UPnPContentDirectory::Data : UPnPContentDirectory::Callback
{
  virtual int                   countDlnaItems(const QString &path);
  virtual QList<Item>           listDlnaItems(const QString &path, unsigned start, unsigned count);

  QMap<QByteArray, QList<QByteArray> > formats;
  QMap<QString, QMap<QString, QString> > queryItems;
  QMap<QString, QString>        activeClients;

  QMap<ItemID, ItemData>        itemData;
  volatile ItemID               idCounter;
  QMap<QString, Callback *>     callbacks;
  QAtomicInt                    systemUpdateId;
};

UPnPContentDirectory::UPnPContentDirectory(QObject *parent)
    : UPnPBase("/upnp/contentdirectory/",
               contentDirectoryNS,
               "urn:upnp-org:serviceId:ContentDirectory",
               parent),
      d(new Data())
{
  d->idCounter = 1;

  // Add the root path.
  ItemData itemData;
  itemData.path = "/";
  itemData.item.isDir = true;

  d->itemData.insert(0, itemData);
  d->callbacks.insert(itemData.path, d);
}

UPnPContentDirectory::~UPnPContentDirectory()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void UPnPContentDirectory::setFormats(const QByteArray &type, const QList<QByteArray> &formats)
{
  QWriteLocker l(lock());

  d->formats[type] = formats;
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

void UPnPContentDirectory::emitEvent(void)
{
  d->systemUpdateId.ref();

  UPnPBase::emitEvent();
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
  const QDomElement browseElem = body.firstChildElement("u:Browse");
  if (!browseElem.isNull())
    handleBrowse(browseElem, responseDoc, responseBody, request, peerAddress);

  const QDomElement getSearchCapabilitiesElem = body.firstChildElement("u:GetSearchCapabilities");
  if (!getSearchCapabilitiesElem.isNull())
  {
    QDomElement response = responseDoc.createElementNS(contentDirectoryNS, "u:GetSearchCapabilitiesResponse");
    addTextElm(responseDoc, response, "SearchCaps", "");
    responseBody.appendChild(response);
  }

  const QDomElement getSortCapabilitiesElem = body.firstChildElement("u:GetSortCapabilities");
  if (!getSortCapabilitiesElem.isNull())
  {
    QDomElement response = responseDoc.createElementNS(contentDirectoryNS, "u:GetSortCapabilitiesResponse");
    addTextElm(responseDoc, response, "SortCaps", "");
    responseBody.appendChild(response);
  }

  const QDomElement getSystemUpdateIdElem = body.firstChildElement("u:GetSystemUpdateID");
  if (!getSystemUpdateIdElem.isNull())
  {
    QDomElement response = responseDoc.createElementNS(contentDirectoryNS, "u:GetSystemUpdateIDResponse");
    addTextElm(responseDoc, response, "Id", QString::number(d->systemUpdateId));
    responseBody.appendChild(response);
  }
}

void UPnPContentDirectory::addEventProperties(QDomDocument &doc, QDomElement &propertySet)
{
  QDomElement property = doc.createElementNS(eventNS, "e:property");
  addTextElm(doc, property, "SystemUpdateID", QString::number(d->systemUpdateId));
  propertySet.appendChild(property);
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

  const ItemID pathId = fromIDString(objectId.text());
  QMap<ItemID, ItemData>::Iterator itemData = d->itemData.find(pathId);
  if (itemData == d->itemData.end())
    return;

  QDomElement browseResponse = doc.createElementNS(contentDirectoryNS, "u:BrowseResponse");
  body.appendChild(browseResponse);

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
        pathId, *itemData, *callback, peerAddress.toString(), request.host(),
        browseFlag.text(), startingIndex.text().toUInt(), requestedCount.text().toUInt());
  }
  else
  {
    browseFile(
        doc, browseResponse,
        pathId, *itemData, peerAddress.toString(), request.host(),
        browseFlag.text(), startingIndex.text().toUInt(), requestedCount.text().toUInt());
  }

  QDomElement updateIDElm = doc.createElement("UpdateID");
  updateIDElm.appendChild(doc.createTextNode(QString::number(itemData->updateId)));
  browseResponse.appendChild(updateIDElm);
}

void UPnPContentDirectory::browseDir(QDomDocument &doc, QDomElement &browseResponse, ItemID pathId, ItemData &itemData, Callback *callback, const QString &peer, const QString &host, const QString &browseFlag, unsigned start, unsigned count)
{
  QDomElement result = doc.createElement("Result");
  int totalMatches = 0, totalReturned = 0;

  if (browseFlag == "BrowseDirectChildren")
  {
    QDomDocument subDoc;
    QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", dublinCoreNS);
    root.setAttribute("xmlns:upnp", metadataNS);
    subDoc.appendChild(root);

    foreach (const Item &item, callback->listDlnaItems(itemData.path, start, count))
    {
      if (!item.isDir && (item.mode == Item::Mode_Direct))
        root.appendChild(didlFile(subDoc, peer, host, item, addChildItem(itemData, item, true), pathId));
      else
        root.appendChild(didlDirectory(subDoc, item, addChildItem(itemData, item, true), pathId));

      totalReturned++;
    }

    totalMatches = callback->countDlnaItems(itemData.path);

    result.appendChild(doc.createTextNode(subDoc.toString(-1) + "\n"));
  }
  else if (browseFlag == "BrowseMetadata")
  {
    QDomDocument subDoc;
    QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", dublinCoreNS);
    root.setAttribute("xmlns:upnp", metadataNS);
    subDoc.appendChild(root);

    if (!itemData.item.isDir &&
        ((itemData.item.audioStreams.isEmpty() &&
          itemData.item.videoStreams.isEmpty() &&
          itemData.item.subtitleStreams.isEmpty()) ||
         !itemData.item.mimeType.startsWith("video")))
    {
      root.appendChild(didlFile(subDoc, peer, host, itemData.item, pathId));
    }
    else
      root.appendChild(didlDirectory(subDoc, itemData.item, pathId));

    totalMatches = totalReturned = 1;

    result.appendChild(doc.createTextNode(subDoc.toString(-1) + "\n"));
  }

  browseResponse.appendChild(result);
  addTextElm(doc, browseResponse, "NumberReturned", QString::number(totalReturned));
  addTextElm(doc, browseResponse, "TotalMatches", QString::number(totalMatches));
}

void UPnPContentDirectory::browseFile(QDomDocument &doc, QDomElement &browseResponse, ItemID pathId, ItemData &itemData, const QString &peer, const QString &host, const QString &browseFlag, unsigned start, unsigned count)
{
  QDomElement result = doc.createElement("Result");
  int totalMatches = 0, totalReturned = 0;

  if (browseFlag == "BrowseDirectChildren")
  {
    QDomDocument subDoc;
    QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", dublinCoreNS);
    root.setAttribute("xmlns:upnp", metadataNS);
    subDoc.appendChild(root);

    QVector<ItemID> all;
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
      QMap<ItemID, ItemData>::Iterator childData = d->itemData.find(all[i]);

      if (childData->item.mode != Item::Mode_Direct)
        root.appendChild(didlDirectory(subDoc, childData->item, childData.key(), pathId));
      else
        root.appendChild(didlFile(subDoc, peer, host, childData->item, childData.key(), pathId));

      totalReturned++;
    }

    totalMatches = all.count();

    result.appendChild(doc.createTextNode(subDoc.toString(-1) + "\n"));
  }
  else if (browseFlag == "BrowseMetadata")
  {
    QDomDocument subDoc;
    QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", dublinCoreNS);
    root.setAttribute("xmlns:upnp", metadataNS);
    subDoc.appendChild(root);

    if (itemData.item.mode != Item::Mode_Direct)
      root.appendChild(didlDirectory(subDoc, itemData.item, pathId));
    else
      root.appendChild(didlFile(subDoc, peer, host, itemData.item, pathId));

    totalMatches = totalReturned = 1;

    result.appendChild(doc.createTextNode(subDoc.toString(-1) + "\n"));
  }

  browseResponse.appendChild(result);
  addTextElm(doc, browseResponse, "NumberReturned", QString::number(totalReturned));
  addTextElm(doc, browseResponse, "TotalMatches", QString::number(totalMatches));
}

UPnPContentDirectory::ItemID UPnPContentDirectory::addChildItem(ItemData &itemData, const Item &item, bool asDir)
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
      return childId;
    }
  }

  // Add new child
  const ItemID childId = d->idCounter++;
  ItemData childData;
  childData.path = fullPath;
  childData.item = item;
  d->itemData.insert(childId, childData);

  itemData.children.append(childId);

  return childId;
}

QDomElement UPnPContentDirectory::didlDirectory(QDomDocument &doc, const Item &dir, ItemID id, ItemID parentId) const
{
  QDomElement containerElm = doc.createElement("container");
  containerElm.setAttribute("id", toIDString(id));
  containerElm.setAttribute("searchable", "false");
  containerElm.setAttribute("restricted", "false");

  if (parentId != 0)
    containerElm.setAttribute("parentID", toIDString(parentId));

  addTextElm(doc, containerElm, "dc:title", (dir.played ? "*" : "") + dir.title);
  addTextElm(doc, containerElm, "upnp:class", "object.container.storageFolder");

  return containerElm;
}

QDomElement UPnPContentDirectory::didlFile(QDomDocument &doc, const QString &peer, const QString &host, const Item &item, ItemID id, ItemID parentId) const
{
  QDomElement itemElm = doc.createElement("item");
  itemElm.setAttribute("id", toIDString(id));
  itemElm.setAttribute("restricted", "false");
  if (parentId != 0)
    itemElm.setAttribute("parentID", toIDString(parentId));

                                addTextElm(doc, itemElm, "dc:title", (item.played ? "*" : "") + item.title);
  if (!item.artist.isEmpty())   addTextElm(doc, itemElm, "upnp:artist", item.artist);
  if (!item.album.isEmpty())    addTextElm(doc, itemElm, "upnp:album", item.album);
  if (!item.iconUrl.isEmpty())  addTextElm(doc, itemElm, "upnp:icon", item.iconUrl.toString());

  QUrl url = item.url;
  url.setScheme("http");
  url.setAuthority(host);
  url.addQueryItem("music", item.music ? "true" : "false");

  QMap<QString, QMap<QString, QString> >::ConstIterator peerItems = d->queryItems.find(peer);
  if (peerItems == d->queryItems.end())
    peerItems = d->queryItems.find(QString::null);

  if (peerItems != d->queryItems.end())
  for (QMap<QString, QString>::ConstIterator i = peerItems->begin(); i != peerItems->end(); i++)
    url.addQueryItem(i.key(), i.value());

  bool found = false;
  QList<QByteArray> formats;
  QDomElement upnpClassElm = doc.createElement("upnp:class");
  for (QMap<QByteArray, QList<QByteArray> >::ConstIterator i = d->formats.begin();
       i != d->formats.end();
       i++)
  {
    if (item.mimeType.startsWith(i.key() + "/"))
    {
      upnpClassElm.appendChild(doc.createTextNode("object.item." + i.key() + "Item"));
      formats = i.value();

      found = true;
      break;
    }
  }

  if (!found)
    upnpClassElm.appendChild(doc.createTextNode("object.item"));

  itemElm.appendChild(upnpClassElm);

  /* DLNA.ORG_FLAGS, padded with 24 trailing 0s
   *     80000000  31  senderPaced
   *     40000000  30  lsopTimeBasedSeekSupported
   *     20000000  29  lsopByteBasedSeekSupported
   *     10000000  28  playcontainerSupported
   *      8000000  27  s0IncreasingSupported
   *      4000000  26  sNIncreasingSupported
   *      2000000  25  rtspPauseSupported
   *      1000000  24  streamingTransferModeSupported
   *       800000  23  interactiveTransferModeSupported
   *       400000  22  backgroundTransferModeSupported
   *       200000  21  connectionStallingSupported
   *       100000  20  dlnaVersion15Supported
   *
   *     Example: (1 << 24) | (1 << 22) | (1 << 21) | (1 << 20)
   *       DLNA.ORG_FLAGS=01700000[000000000000000000000000] // [] show padding
   */
  foreach (const QByteArray &format, formats)
  {
    const QByteArray mime = format.left(format.indexOf('.'));
    const QByteArray suffix = format.mid(format.indexOf('.'));

    QDomElement resElm = doc.createElement("res");
    resElm.setAttribute("protocolInfo", QString::fromAscii("http-get:*:" + mime + ":DLNA.ORG_PS=1;DLNA.ORG_CI=0;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01700000000000000000000000000000"));

    if (item.duration > 0)
      resElm.setAttribute("duration", QTime().addSecs(item.duration).toString("h:mm:ss.zzz"));

    QUrl u = url;
    u.setPath(u.path() + suffix);
    //qDebug() << resElm.attribute("protocolInfo") << u.toString();

    resElm.appendChild(doc.createTextNode(u.toString()));
    itemElm.appendChild(resElm);
  }

  return itemElm;
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
