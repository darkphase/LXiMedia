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

namespace LXiServer {

const char  * const SUPnPContentDirectory::contentDirectoryNS = "urn:schemas-upnp-org:service:ContentDirectory:1";
const unsigned  SUPnPContentDirectory::seekSec = 120;

struct SUPnPContentDirectory::Data : SUPnPContentDirectory::Callback
{
  virtual int                   countContentDirItems(const QString &path);
  virtual QList<Item>           listContentDirItems(const QString &path, unsigned start, unsigned count);

  SUPnPGenaServer             * genaServer;

  QMap<ProtocolType, ProtocolList> protocols;
  QMap<QString, QMap<QString, QString> > queryItems;
  QMap<QString, QString>        activeClients;

  QMap<QString, Callback *>     callbacks;
  QAtomicInt                    systemUpdateId;

#ifdef USE_SMALL_OBJECTIDS
  QVector<QString>              objectIdList;
  QHash<QString, qint32>        objectIdHash;
#endif
};

SUPnPContentDirectory::SUPnPContentDirectory(const QString &basePath, QObject *parent)
    : SUPnPBase(basePath + "contentdirectory/", parent),
      d(new Data())
{
  d->genaServer = new SUPnPGenaServer(SUPnPBase::basePath(), this);

  d->systemUpdateId = 1;

  // Add the root path.
  d->callbacks.insert("/", d);

#ifdef USE_SMALL_OBJECTIDS
  d->objectIdList.append(QString::null);
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

void SUPnPContentDirectory::setProtocols(ProtocolType type, const ProtocolList &protocols)
{
  d->protocols[type] = protocols;
}

void SUPnPContentDirectory::setQueryItems(const QString &peer, const QMap<QString, QString> &queryItems)
{
  d->queryItems[peer] = queryItems;
}

QMap<QString, QString> SUPnPContentDirectory::activeClients(void) const
{
  return d->activeClients;
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

void SUPnPContentDirectory::modified(void)
{
  emitEvent(true);
}

void SUPnPContentDirectory::buildDescription(QDomDocument &doc, QDomElement &scpdElm)
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

void SUPnPContentDirectory::handleSoapMessage(const QDomElement &body, QDomDocument &responseDoc, QDomElement &responseBody, const SHttpServer::RequestHeader &request, const QHostAddress &peerAddress)
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

void SUPnPContentDirectory::handleBrowse(const QDomElement &elem, QDomDocument &doc, QDomElement &body, const SHttpServer::RequestHeader &request, const QHostAddress &peerAddress)
{
  const QString path = fromObjectID(elem.firstChildElement("ObjectID").text());
  const QString browseFlag = elem.firstChildElement("BrowseFlag").text();
  const unsigned start = elem.firstChildElement("StartingIndex").text().toUInt();
  const unsigned count = elem.firstChildElement("RequestedCount").text().toUInt();
  const QString peer = peerAddress.toString();
  const QString host = request.host();

  // Find requested directory
  d->activeClients[peer] = request.field("User-Agent");

  QDomElement browseResponse = createElementNS(doc, elem, "BrowseResponse");
  addTextElm(doc, browseResponse, "UpdateID", QString::number(d->systemUpdateId));

  const QString basePath = baseDir(path);
  QMap<QString, Callback *>::Iterator callback = d->callbacks.find(basePath);
  for (QString i=basePath; !i.isEmpty() && (callback == d->callbacks.end()); i=parentDir(i))
    callback = d->callbacks.find(i);

  if ((callback == d->callbacks.end()) || !path.startsWith(callback.key()))
  {
    qDebug() << "SUPnPContentDirectory: could not find callback for path:" << path;
    return;
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
      foreach (const Item &item, (*callback)->listContentDirItems(path, start, count))
      {
        if (!item.isDir)
        {
          const QString title = (item.played ? "*" : "") + item.title;
          if (item.direct)
            root.appendChild(didlFile(subDoc, peer, host, item, path + QString::number(itemIndex), title));
          else
            root.appendChild(didlDirectory(subDoc, Item::Type(item.type), path + QString::number(itemIndex), title));
        }
        else
          root.appendChild(didlDirectory(subDoc, Item::Type(item.type), path + item.title + '/'));

        itemIndex++;
        totalReturned++;
      }

      totalMatches = (*callback)->countContentDirItems(path);

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
      root.appendChild(didlDirectory(subDoc, Item::Type_None, path));
      subDoc.appendChild(root);

      totalMatches = totalReturned = 1;

      result.appendChild(doc.createTextNode(subDoc.toString(-1)));
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
    foreach (const Item &i, (*callback)->listContentDirItems(basePath, itemIndex, 1))
      item = i;

    if (item.isNull())
    {
      qDebug() << "SUPnPContentDirectory: could not find item" << itemIndex << "in path:" << basePath;
      return;
    }

    QDomElement result = doc.createElement("Result");
    int totalMatches = 0, totalReturned = 0;

    if (browseFlag == "BrowseDirectChildren")
    {
      QDomDocument subDoc;
      QDomElement root = subDoc.createElementNS(didlNS, "DIDL-Lite");
      root.setAttribute("xmlns:dc", dublinCoreNS);
      root.setAttribute("xmlns:dlna", dlnaNS);
      root.setAttribute("xmlns:upnp", metadataNS);

      QStringList items;
      if (itemProps[1].isEmpty()) // Root
        items = streamItems(item);
      else if (itemProps[1] == "r")
        items = playSeekItems(item);
      else if (itemProps[1] == "s")
        items = seekItems(item);
      else if (itemProps[1] == "c")
        items = chapterItems(item);

      // Only select the items that were requested.
      for (int i=start, n=0; (i<items.count()) && ((count == 0) || (n<int(count))); i++, n++)
      {
        const QStringList props = splitItemProps(file + '|' + items[i]);
        if (props[1] == "p")
          root.appendChild(didlFile(subDoc, peer, host, makePlayItem(item, props), path + '|' + items[i]));
        else
          root.appendChild(didlDirectory(subDoc, Item::Type(item.type), path + '|' + items[i], props[3]));

        totalReturned++;
      }

      totalMatches = items.count();

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

      if (itemProps[1].isEmpty() || (itemProps[1] == "p"))
        root.appendChild(didlFile(subDoc, peer, host, makePlayItem(item, itemProps), path));
      else
        root.appendChild(didlDirectory(subDoc, Item::Type(item.type), path, itemProps[3]));

      totalMatches = totalReturned = 1;

      subDoc.appendChild(root);
      result.appendChild(doc.createTextNode(subDoc.toString(-1)));
    }

    browseResponse.appendChild(result);
    addTextElm(doc, browseResponse, "NumberReturned", QString::number(totalReturned));
    addTextElm(doc, browseResponse, "TotalMatches", QString::number(totalMatches));
  }

  body.appendChild(browseResponse);
}

QDomElement SUPnPContentDirectory::didlDirectory(QDomDocument &doc, Item::Type type, const QString &path, const QString &title)
{
  const QString parentPath = parentDir(path);
  const QString dcTitle =
      !title.isEmpty()
          ? title
          : (!parentPath.isEmpty()
                ? path.mid(parentPath.length(), path.length() - parentPath.length() - 1)
                : QString("root"));

  QDomElement containerElm = doc.createElement("container");
  containerElm.setAttribute("id", toObjectID(path));
  containerElm.setAttribute("restricted", "true");
  containerElm.setAttribute("parentID", toObjectID(parentPath));

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

  return containerElm;
}

QDomElement SUPnPContentDirectory::didlFile(QDomDocument &doc, const QString &peer, const QString &host, const Item &item, const QString &path, const QString &title)
{
  QDomElement itemElm = doc.createElement("item");
  itemElm.setAttribute("id", toObjectID(path));
  itemElm.setAttribute("restricted", "true");
  itemElm.setAttribute("parentID", toObjectID(parentDir(path)));

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

  QUrl url = item.url;
  url.setScheme("http");
  url.setAuthority(host);

  if ((item.type == Item::Type_Music) || (item.type == Item::Type_MusicVideo))
    url.addQueryItem("music", "true");

  QMap<QString, QMap<QString, QString> >::ConstIterator peerItems = d->queryItems.find(peer);
  if (peerItems == d->queryItems.end())
    peerItems = d->queryItems.find(QString::null);

  if (peerItems != d->queryItems.end())
  for (QMap<QString, QString>::ConstIterator i = peerItems->begin(); i != peerItems->end(); i++)
    url.addQueryItem(i.key(), i.value());

  switch (Item::Type(item.type))
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

  QList<ProtocolType> protocolTypes;
  switch (Item::Type(item.type))
  {
  case Item::Type_None:           protocolTypes += ProtocolType_None; break;
  case Item::Type_Playlist:       protocolTypes += ProtocolType_Video; protocolTypes += ProtocolType_Audio; break;

  case Item::Type_Audio:
  case Item::Type_Music:
  case Item::Type_AudioBroadcast:
  case Item::Type_AudioBook:      protocolTypes += ProtocolType_Audio; break;

  case Item::Type_Video:
  case Item::Type_Movie:
  case Item::Type_VideoBroadcast: protocolTypes += ProtocolType_Video; break;
  case Item::Type_MusicVideo:     protocolTypes += ProtocolType_Video; protocolTypes += ProtocolType_Audio; break;

  case Item::Type_Image:
  case Item::Type_Photo:          protocolTypes += ProtocolType_Image; break;
  }

  foreach (ProtocolType protocolType, protocolTypes)
  foreach (const Protocol &protocol, d->protocols[protocolType])
  {
    QDomElement resElm = doc.createElement("res");
    resElm.setAttribute("protocolInfo", protocol.toString());

    if (item.duration > 0)
      resElm.setAttribute("duration", QTime().addSecs(item.duration).toString("h:mm:ss.zzz"));

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

QStringList SUPnPContentDirectory::streamItems(const Item &item)
{
  if (!item.audioStreams.isEmpty() ||
      !item.videoStreams.isEmpty() ||
      !item.subtitleStreams.isEmpty())
  {
    const QList<Item::Stream> audioStreams = item.audioStreams;
    const QList<Item::Stream> dataStreams = QList<Item::Stream>()
        << item.subtitleStreams << Item::Stream(0x0000FFFF);

    if ((audioStreams.count() > 1) || (dataStreams.count() > 1))
    {
      QStringList result;

      for (int a=0, an=audioStreams.count(); a < an; a++)
      for (int d=0, dn=dataStreams.count(); d < dn; d++)
      {
        QString title, query;

        title = QString::number(a + 1) + ". ";
        if (!audioStreams[a].lang.isEmpty())
          title += audioStreams[a].lang;
        else
          title += tr("Unknown");

        query += "&language=" + QString::number(audioStreams[a].id, 16);

        if (dataStreams[d].id != 0x0000FFFF)
        {
          title += ", " + QString::number(d + 1) + ". ";
          if (!dataStreams[d].lang.isEmpty())
            title += dataStreams[d].lang;
          else
            title += tr("Unknown");

          query += "&subtitles=" + QString::number(dataStreams[d].id, 16);
        }

        result += ("r" + query + "#" + title);
      }

      return result;
    }
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

QString SUPnPContentDirectory::toObjectID(const QString &path)
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
    QHash<QString, qint32>::ConstIterator i = d->objectIdHash.find(path);
    if (i != d->objectIdHash.end())
      return QString::number(*i);

    d->objectIdList.append(path);
    d->objectIdList.last().squeeze();
    d->objectIdHash.insert(d->objectIdList.last(), d->objectIdList.count() - 1);

    return QString::number(d->objectIdList.count() - 1);
#else
    return QString::fromAscii(qCompress(path.toUtf8(), 9).toBase64());
    //return path;
#endif
  }
}

QString SUPnPContentDirectory::fromObjectID(const QString &idStr)
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
      return d->objectIdList[id];

    return QString::null;
#else
    return QString::fromUtf8(qUncompress(QByteArray::fromBase64(idStr.toAscii())));
    //return idStr;
#endif
  }
}

int SUPnPContentDirectory::Data::countContentDirItems(const QString &path)
{
  QSet<QString> subDirs;

  for (QMap<QString, Callback *>::ConstIterator i=callbacks.begin(); i!=callbacks.end(); i++)
  if (i.key().startsWith(path))
  {
    QString sub = i.key().mid(path.length() - 1);
    sub = sub.left(sub.indexOf('/', 1) + 1);
    if (sub.length() > 1)
      subDirs.insert(sub);
  }

  return subDirs.count();
}

QList<SUPnPContentDirectory::Item> SUPnPContentDirectory::Data::listContentDirItems(const QString &path, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;
  QList<SUPnPContentDirectory::Item> result;
  QSet<QString> names;

  for (QMap<QString, Callback *>::ConstIterator i=callbacks.begin(); i!=callbacks.end(); i++)
  if (i.key().startsWith(path))
  {
    QString sub = i.key().mid(path.length() - 1);
    sub = sub.left(sub.indexOf('/', 1) + 1);
    if ((sub.length() > 1) && !names.contains(sub))
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
