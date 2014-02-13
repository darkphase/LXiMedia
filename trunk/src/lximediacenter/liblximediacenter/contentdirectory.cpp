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

#include "contentdirectory.h"

namespace LXiMediaCenter {

const char      ContentDirectory::serviceId[]       = "urn:upnp-org:serviceId:ContentDirectory";
const char      ContentDirectory::httpBaseDir[]     = "/upnp/condir/";
const unsigned  ContentDirectory::seekSec = 120;

struct ContentDirectory::Data : ContentDirectory::Callback
{
  virtual                       ~Data() { }
  virtual QList<Item>           listContentDirItems(const QByteArray &client, const QString &path, int start, int &count);
  virtual Item                  getContentDirItem(const QByteArray &client, const QString &path);

  static const QEvent::Type     emitUpdateEventType;

  quint32                       systemUpdateId;
  QMap<QString, Callback *>     callbacks;

  QVector<QByteArray>           objectIdList;
  QHash<QByteArray, qint32>     objectIdHash;
  QVector<QByteArray>           objectUrlList;
  QHash<QByteArray, qint32>     objectUrlHash;
};

const QEvent::Type  ContentDirectory::Data::emitUpdateEventType = QEvent::Type(QEvent::registerEventType());

ContentDirectory::ContentDirectory(RootDevice *parent, ConnectionManager *connectionManager)
  : QObject(parent),
    parent(parent),
    connectionManager(connectionManager),
    d(new Data())
{
  d->systemUpdateId = 0;

  // Add the root path.
  d->callbacks.insert("/", d);

  d->objectIdList.append(QByteArray());
  d->objectIdHash.insert(d->objectIdList.last(), d->objectIdList.count() - 1);

  parent->registerService(serviceId, this);
}

ContentDirectory::~ContentDirectory()
{
  parent->unregisterService(serviceId);

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void ContentDirectory::registerCallback(const QString &path, Callback *callback)
{
  d->callbacks.insert(path, callback);
}

void ContentDirectory::unregisterCallback(Callback *callback)
{
  for (QMap<QString, Callback *>::Iterator i=d->callbacks.begin(); i!=d->callbacks.end(); )
  if (i.value() == callback)
    i = d->callbacks.erase(i);
  else
    i++;
}

void ContentDirectory::handleAction(const RequestInfo &requestInfo, ActionBrowse &action)
{
  const QString path = fromObjectID(action.getObjectID());
  const int start = action.getStartingIndex();
  const int count = action.getRequestedCount();

  QByteArray client = requestInfo.userAgent;
  const int space = requestInfo.userAgent.indexOf(' ');
  if (space > 0)
    client = client.left(space);

  client += '@' + requestInfo.sourceAddress;

  const QString basePath = baseDir(path);
  QMap<QString, Callback *>::Iterator callback = d->callbacks.find(basePath);
  for (QString i=basePath; !i.isEmpty() && (callback == d->callbacks.end()); i=parentDir(i))
    callback = d->callbacks.find(i);

  if ((callback == d->callbacks.end()) || !path.startsWith(callback.key()))
  {
    qWarning() << "ContentDirectory: could not find callback for path:" << path;
    return;
  }

  int totalMatches = 0;

  if (path.endsWith('/')) // Directory
  {
    switch (action.getBrowseFlag())
    {
    case ActionBrowse::BrowseDirectChildren:
      totalMatches = count;
      foreach (const Item &item, (*callback)->listContentDirItems(client, path, start, totalMatches))
      {
        if (!item.isDir)
        {
          switch (item.type)
          {
          case Item::Type_None:
          case Item::Type_Music:
          case Item::Type_MusicVideo:
          case Item::Type_Image:
          case Item::Type_Photo:
            addFile(action, requestInfo.host, item, item.path, item.title);
            break;

          case Item::Type_AudioBroadcast:
          case Item::Type_Video:
          case Item::Type_VideoBroadcast:
            addFile(action, requestInfo.host, item, item.path, item.played ? ('*' + item.title) : item.title);
            break;

          case Item::Type_Audio:
          case Item::Type_AudioBook:
          case Item::Type_Movie:
            addContainer(action, Item::Type(item.type), item.path, item.played ? ('*' + item.title) : item.title);
            break;
          }
        }
        else
          addDirectory(action, Item::Type(item.type), client, item.path);
      }
      break;

    case ActionBrowse::BrowseMetadata:
      addDirectory(action, Item::Type_None, client, path);
      totalMatches = 1;
      break;
    }
  }
  else
  {
    const QStringList itemProps = splitItemProps(path);

    // Get the item
    const Item item = (*callback)->getContentDirItem(client, itemProps[0]);
    if (item.isNull())
    {
      qWarning() << "ContentDirectory: could not find item" << itemProps[0];
      return;
    }

    const QStringList items = allItems(item, itemProps);
    switch (action.getBrowseFlag())
    {
    case ActionBrowse::BrowseDirectChildren:
      // Only select the items that were requested.
      for (int i=start, n=0; (i<items.count()) && ((count == 0) || (n<int(count))); i++, n++)
      {
        const QStringList props = splitItemProps(path + '\t' + items[i]);
        if (props[1] == "p")
          addFile(action, requestInfo.host, makePlayItem(item, props), path + '\t' + items[i]);
        else
          addContainer(action, Item::Type(item.type), path + '\t' + items[i], props[3], allItems(item, splitItemProps(items[i])).count());
      }

      totalMatches = items.count();
      break;

    case ActionBrowse::BrowseMetadata:
      if (itemProps[1].isEmpty() || (itemProps[1] == "p"))
        addFile(action, requestInfo.host, makePlayItem(item, itemProps), path);
      else
        addContainer(action, Item::Type(item.type), path, itemProps[3], items.count());

      totalMatches = 1;
      break;
    }
  }

  action.setResponse(totalMatches, d->systemUpdateId);
}

void ContentDirectory::handleAction(const RequestInfo &, ActionSearch &action)
{
  action.setResponse(0, d->systemUpdateId);
}

void ContentDirectory::handleAction(const RequestInfo &, ActionGetSearchCapabilities &action)
{
  action.setResponse(QByteArray());
}

void ContentDirectory::handleAction(const RequestInfo &, ActionGetSortCapabilities &action)
{
  action.setResponse(QByteArray());
}

void ContentDirectory::handleAction(const RequestInfo &, ActionGetSystemUpdateID &action)
{
  action.setResponse(d->systemUpdateId);
}

void ContentDirectory::handleAction(const RequestInfo &, ActionGetFeatureList &action)
{
  QList<QByteArray> containers;
  containers += "object.item.audioItem";
  containers += "object.item.videoItem";
  containers += "object.item.imageItem";

  action.setResponse(containers);
}

const char * ContentDirectory::serviceType(void)
{
  return RootDevice::serviceTypeContentDirectory;
}

void ContentDirectory::initialize(void)
{
  d->systemUpdateId = 1;

  parent->registerHttpCallback(httpBaseDir, this);
}

void ContentDirectory::close(void)
{
  d->objectIdList.clear();
  d->objectIdHash.clear();
  d->objectUrlList.clear();
  d->objectUrlHash.clear();

  d->objectIdList.append(QByteArray());
  d->objectIdHash.insert(d->objectIdList.last(), d->objectIdList.count() - 1);
}

void ContentDirectory::writeServiceDescription(RootDevice::ServiceDescription &desc) const
{
  {
    static const char * const argname[] = { "ObjectID"            , "BrowseFlag"            , "Filter"            , "StartingIndex"   , "RequestedCount"  , "SortCriteria"            , "Result"            , "NumberReturned"  , "TotalMatches"    , "UpdateID"            };
    static const char * const argdir[]  = { "in"                  , "in"                    , "in"                , "in"              , "in"              , "in"                      , "out"               , "out"             , "out"             , "out"                 };
    static const char * const argvar[]  = { "A_ARG_TYPE_ObjectID" , "A_ARG_TYPE_BrowseFlag" , "A_ARG_TYPE_Filter" , "A_ARG_TYPE_Index", "A_ARG_TYPE_Count", "A_ARG_TYPE_SortCriteria" , "A_ARG_TYPE_Result" , "A_ARG_TYPE_Count", "A_ARG_TYPE_Count", "A_ARG_TYPE_UpdateID" };
    desc.addAction("Browse", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "ContainerID"         , "SearchCriteria"            , "Filter"            , "StartingIndex"   , "RequestedCount"  , "SortCriteria"            , "Result"            , "NumberReturned"  , "TotalMatches"    , "UpdateID"            };
    static const char * const argdir[]  = { "in"                  , "in"                        , "in"                , "in"              , "in"              , "in"                      , "out"               , "out"             , "out"             , "out"                 };
    static const char * const argvar[]  = { "A_ARG_TYPE_ObjectID" , "A_ARG_TYPE_SearchCriteria" , "A_ARG_TYPE_Filter" , "A_ARG_TYPE_Index", "A_ARG_TYPE_Count", "A_ARG_TYPE_SortCriteria" , "A_ARG_TYPE_Result" , "A_ARG_TYPE_Count", "A_ARG_TYPE_Count", "A_ARG_TYPE_UpdateID" };
    desc.addAction("Search", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "SearchCaps"          };
    static const char * const argdir[]  = { "out"                 };
    static const char * const argvar[]  = { "SearchCapabilities"  };
    desc.addAction("GetSearchCapabilities", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "SortCaps"          };
    static const char * const argdir[]  = { "out"               };
    static const char * const argvar[]  = { "SortCapabilities"  };
    desc.addAction("GetSortCapabilities", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "Id"              };
    static const char * const argdir[]  = { "out"             };
    static const char * const argvar[]  = { "SystemUpdateID"  };
    desc.addAction("GetSystemUpdateID", argname, argdir, argvar);
  }
  { // Samsung GetFeatureList
    static const char * const argname[] = { "FeatureList"             };
    static const char * const argdir[]  = { "out"                     };
    static const char * const argvar[]  = { "A_ARG_TYPE_Featurelist"  };
    desc.addAction("X_GetFeatureList", argname, argdir, argvar);
  }

  desc.addStateVariable("A_ARG_TYPE_ObjectID"      , "string", false );
  desc.addStateVariable("A_ARG_TYPE_Result"        , "string", false );
  desc.addStateVariable("A_ARG_TYPE_SearchCriteria", "string", false );
  static const char * const browseFlagValues[] = { "BrowseMetadata", "BrowseDirectChildren" };
  desc.addStateVariable("A_ARG_TYPE_BrowseFlag"    , "string", false, browseFlagValues);
  desc.addStateVariable("A_ARG_TYPE_Filter"        , "string", false );
  desc.addStateVariable("A_ARG_TYPE_SortCriteria"  , "string", false );
  desc.addStateVariable("A_ARG_TYPE_Index"         , "ui4"   , false );
  desc.addStateVariable("A_ARG_TYPE_Count"         , "ui4"   , false );
  desc.addStateVariable("A_ARG_TYPE_UpdateID"      , "ui4"   , false );
  desc.addStateVariable("A_ARG_TYPE_Featurelist"   , "string", false ); // Samsung GetFeatureList
  desc.addStateVariable("SearchCapabilities"       , "string", false );
  desc.addStateVariable("SortCapabilities"         , "string", false );
  desc.addStateVariable("SystemUpdateID"           , "ui4"   , true  );
  desc.addStateVariable("TransferIDs"              , "string", true  );
}

void ContentDirectory::writeEventableStateVariables(RootDevice::EventablePropertySet &propset) const
{
  propset.addProperty("SystemUpdateID", QString::number(d->systemUpdateId));
  propset.addProperty("TransferIDs", "");
}

void ContentDirectory::customEvent(QEvent *e)
{
  if (e->type() == d->emitUpdateEventType)
    parent->emitEvent(serviceId);
  else
    QObject::customEvent(e);
}

HttpStatus ContentDirectory::httpRequest(const QUrl &request, const RequestInfo &requestInfo, QByteArray &contentType, QIODevice *&response)
{
  if (request.path().startsWith(httpBaseDir))
  {
    const HttpStatus result = parent->handleHttpRequest(fromObjectURL(request), requestInfo, contentType, response);
    if (result == HttpStatus_Ok)
      connectionManager->addOutputConnection(request, contentType, response);

    return result;
  }

  return HttpStatus_NotFound;
}

void ContentDirectory::addDirectory(ActionBrowse &action, Item::Type type, const QString &, const QString &path, const QString &title)
{
  QMap<QString, Callback *>::Iterator callback = d->callbacks.find(path);
  for (QString i=path; !i.isEmpty() && (callback == d->callbacks.end()); i=parentDir(i))
    callback = d->callbacks.find(i);

  if ((callback == d->callbacks.end()) || !path.startsWith(callback.key()))
  {
    qWarning() << "ContentDirectory: could not find callback for path:" << path;
    return;
  }

  addContainer(action, type, path, title);
}

void ContentDirectory::addContainer(ActionBrowse &action, Item::Type type, const QString &path, const QString &title, int childCount)
{
  const QString parentPath = parentDir(path);

  BrowseContainer container;
  container.id = toObjectID(path);
  container.parentID = toObjectID(parentPath);
  container.restricted = true;
  container.childCount = childCount;

  container.title =
      !title.isEmpty()
          ? title.toUtf8()
          : (!parentPath.isEmpty()
                ? path.mid(parentPath.length(), path.length() - parentPath.length() - 1).toUtf8()
                : QByteArray("root"));

  switch (type)
  {
  case Item::Type_None:           container.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.container.album")); break;

  case Item::Type_Audio:
  case Item::Type_AudioBroadcast:
  case Item::Type_AudioBook:      container.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.container.album")); break;
  case Item::Type_Music:          container.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.container.album.musicAlbum")); break;

  case Item::Type_Video:
  case Item::Type_Movie:
  case Item::Type_VideoBroadcast: container.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.container.album")); break;
  case Item::Type_MusicVideo:     container.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.container.album.musicAlbum")); break;

  case Item::Type_Image:          container.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.container.album")); break;
  case Item::Type_Photo:          container.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.container.album.photoAlbum")); break;
  }

  action.addContainer(container);
}

void ContentDirectory::addFile(ActionBrowse &action, const QString &host, const Item &item, const QString &path, const QString &title)
{
  const QString parentPath = parentDir(path);

  BrowseItem browseItem;
  browseItem.id = toObjectID(path);
  browseItem.parentID = toObjectID(parentPath);
  browseItem.restricted = true;
  browseItem.title = !title.isEmpty() ? title.toUtf8() : item.title.toUtf8();

  if (!item.artist.isEmpty())
    browseItem.attributes += qMakePair(QByteArray("upnp:artist"), item.artist.toUtf8());

  if (!item.album.isEmpty())
    browseItem.attributes += qMakePair(QByteArray("upnp:album"), item.album.toUtf8());

  if (!item.iconUrl.isEmpty())
  {
    QUrl url = item.iconUrl;
    url.setScheme("http");
    url.setAuthority(host);

    if (url.path().endsWith(".jpeg") || url.path().endsWith(".jpg"))
      browseItem.attributes += qMakePair(QByteArray("upnp:albumArtURI"), toObjectURL(url, ".jpeg"));
    else
      browseItem.attributes += qMakePair(QByteArray("upnp:albumArtURI"), toObjectURL(url, ".png"));
  }

  switch (item.type)
  {
  case Item::Type_None:           browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item")); break;

  case Item::Type_Audio:          browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.audioItem")); break;
  case Item::Type_Music:          browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.audioItem.musicTrack")); break;
  case Item::Type_AudioBroadcast: browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.audioItem.audioBroadcast")); break;
  case Item::Type_AudioBook:      browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.audioItem.audioBook")); break;

  case Item::Type_Video:          browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.videoItem")); break;
  case Item::Type_Movie:          browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.videoItem.movie")); break;
  case Item::Type_VideoBroadcast: browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.videoItem.videoBroadcast")); break;
  case Item::Type_MusicVideo:     browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.videoItem.musicVideoClip")); break;

  case Item::Type_Image:          browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.imageItem")); break;
  case Item::Type_Photo:          browseItem.attributes += qMakePair(QByteArray("upnp:class"), QByteArray("object.item.imageItem.photo")); break;
  }

  foreach (const ConnectionManager::Protocol &protocol, item.protocols)
  {
    QUrl url = item.url;
    url.setScheme("http");
    url.setAuthority(host);
    QUrlQuery query(url);
    query.addQueryItem("contentFeatures", protocol.contentFeatures().toBase64());
    url.setQuery(query);

    browseItem.files += qMakePair(toObjectURL(url, protocol.suffix), protocol);
  }

  action.addItem(browseItem);
}

QStringList ContentDirectory::allItems(const Item &item, const QStringList &itemProps)
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

QStringList ContentDirectory::streamItems(const Item &item)
{
  if (item.streams.count() > 1)
  {
    QStringList result;
    foreach (const Item::Stream &stream, item.streams)
    {
      QString query;

      for (int i=0; i<stream.queryItems.count(); i++)
        query += '&' + stream.queryItems[i].first + '=' + stream.queryItems[i].second;

      result += ('r' + query + '#' + stream.title);
    }

    return result;
  }

  return playSeekItems(item);
}

QStringList ContentDirectory::playSeekItems(const Item &item)
{
  QStringList result;

  result += ("p#" + tr("Play"));

  if (item.chapters.count() > 1)
    result += ("c#" + tr("Chapters"));

  if (item.duration > seekSec)
    result += ("s#" + tr("Seek"));

  return result;
}

QStringList ContentDirectory::seekItems(const Item &item)
{
  QStringList result;

  for (unsigned i=0; i<item.duration; i+=seekSec)
  {
    QString title = tr("Play from") + " " + QTime(0, 0).addSecs(i).toString("h:mm");

    result += ("p&position=" + QString::number(i) + "#" + title);
  }

  return result;
}

QStringList ContentDirectory::chapterItems(const Item &item)
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

QStringList ContentDirectory::splitItemProps(const QString &text)
{
  QStringList itemProps = QStringList() << QString::null << QString::null << QString::null << QString::null;
  foreach (const QString &section, text.split('\t'))
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

ContentDirectory::Item ContentDirectory::makePlayItem(const Item &baseItem, const QStringList &itemProps)
{
  Item item = baseItem;

  if (!itemProps[3].isEmpty())
    item.title = itemProps[3];

  if (!itemProps[2].isEmpty())
  {
    QUrlQuery query(item.url);

    foreach (const QString &qi, itemProps[2].split('&', QString::SkipEmptyParts))
    {
      const QStringList qil = qi.split('=');
      if (qil.count() == 2)
        query.addQueryItem(qil[0], qil[1]);
    }

    item.url.setQuery(query);
  }

  return item;
}

QString ContentDirectory::baseDir(const QString &dir)
{
  return dir.left(dir.lastIndexOf('/') + 1);
}

QString ContentDirectory::parentDir(const QString &dir)
{
  if (!dir.isEmpty())
  {
    const int last =
        qMax(dir.left(dir.length() - 1).lastIndexOf('/'),
             dir.left(dir.length() - 1).lastIndexOf('\t') - 1);

    if (last >= 0)
      return dir.left(last + 1);
  }

  return QString::null;
}

QByteArray ContentDirectory::toObjectID(const QString &path)
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
    QHash<QByteArray, qint32>::ConstIterator i = d->objectIdHash.find(path.toUtf8());
    if (i != d->objectIdHash.end())
      return QByteArray::number(*i);

    d->objectIdList.append(path.toUtf8());
    d->objectIdList.last().squeeze();
    d->objectIdHash.insert(d->objectIdList.last(), d->objectIdList.count() - 1);

    return QByteArray::number(d->objectIdList.count() - 1);
  }
}

QString ContentDirectory::fromObjectID(const QByteArray &idStr)
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
    const qint32 id = idStr.toInt();
    if (id < d->objectIdList.count())
      return QString::fromUtf8(d->objectIdList[id]);

    return QString::null;
  }
}

QByteArray ContentDirectory::toObjectURL(const QUrl &url, const QByteArray &suffix)
{
  const QByteArray encoded = url.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority);

  QUrl newUrl;
  QHash<QByteArray, qint32>::ConstIterator i = d->objectUrlHash.find(encoded);
  if (i == d->objectUrlHash.end())
  {
    d->objectUrlList.append(encoded);
    d->objectUrlList.last().squeeze();
    d->objectUrlHash.insert(d->objectUrlList.last(), d->objectUrlList.count() - 1);

    newUrl = httpBaseDir + ("0000000" + QByteArray::number(d->objectUrlList.count() - 1, 16)).right(8) + suffix;
  }
  else
    newUrl = httpBaseDir + ("0000000" + QByteArray::number(*i, 16)).right(8) + suffix;

  newUrl.setScheme("http");
  newUrl.setAuthority(url.authority());
  return newUrl.toEncoded();
}

QUrl ContentDirectory::fromObjectURL(const QUrl &url)
{
  const QByteArray path = url.path().toUtf8();
  const int lastSlash = path.lastIndexOf('/');
  if (lastSlash >= 0)
  {
    const qint32 id = path.mid(lastSlash + 1, 8).toInt(NULL, 16);
    if ((id >= 0) && (id < d->objectUrlList.count()))
    {
      QUrl newUrl = QUrl::fromEncoded(d->objectUrlList[id]);
      newUrl.setScheme("http");
      newUrl.setAuthority(url.authority());
      return newUrl;
    }
  }

  return QUrl();
}


ContentDirectory::Item::Item(void)
  : isDir(false), played(false), type(Type_None), track(0), duration(0)
{
}

ContentDirectory::Item::~Item()
{
}

bool ContentDirectory::Item::isNull(void) const
{
  return url.isEmpty();
}

bool ContentDirectory::Item::isAudio(void) const
{
  return (type >= Type_Audio) && (type < Type_Video);
}

bool ContentDirectory::Item::isVideo(void) const
{
  return (type >= Type_Video) && (type < Type_Image);
}

bool ContentDirectory::Item::isImage(void) const
{
  return (type >= Type_Image) && (type <= Type_Photo);
}

bool ContentDirectory::Item::isMusic(void) const
{
  return (type == Type_Music) || (type == Type_MusicVideo);
}


ContentDirectory::Item::Stream::Stream(void)
{
}

ContentDirectory::Item::Stream::~Stream()
{
}


ContentDirectory::Item::Chapter::Chapter(const QString &title, unsigned position)
  : title(title), position(position)
{
}

ContentDirectory::Item::Chapter::~Chapter()
{
}


ContentDirectory::BrowseItem::BrowseItem()
  : restricted(true),
    duration(0)
{
}

ContentDirectory::BrowseItem::~BrowseItem()
{
}


ContentDirectory::BrowseContainer::BrowseContainer()
  : restricted(true),
    childCount(-1)
{
}

ContentDirectory::BrowseContainer::~BrowseContainer()
{
}


QList<ContentDirectory::Item> ContentDirectory::Data::listContentDirItems(const QByteArray &client, const QString &path, int start, int &count)
{
  const bool returnAll = count == 0;
  QList<ContentDirectory::Item> result;
  QSet<QString> names;

  for (QMap<QString, Callback *>::ConstIterator i=callbacks.begin(); i!=callbacks.end(); i++)
  if (i.key().startsWith(path))
  {
    QString sub = i.key().mid(path.length() - 1);
    sub = sub.left(sub.indexOf('/', 1) + 1);
    if ((sub.length() > 1) && !names.contains(sub))
    {
      int total = 1;
      if (!(*i)->listContentDirItems(client, i.key(), 0, total).isEmpty() && (total > 0))
      {
        names.insert(sub);

        if (returnAll || (count > 0))
        {
          if (start == 0)
          {
            Item item;
            item.isDir = true;
            item.path = sub;
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
  }

  count = names.count();

  return result;
}

ContentDirectory::Item ContentDirectory::Data::getContentDirItem(const QByteArray &, const QString &)
{
  return Item();
}

} // End of namespace
