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

#include "dlnaserver.h"
#include "htmlparser.h"
#include "ssdpserver.h"

namespace LXiMediaCenter {

class DlnaServer::EventSession : protected QThread
{
public:
                                EventSession(DlnaServer *, const QString &eventUrl);
  virtual                       ~EventSession();

  void                          update(qint32 dirId);
  inline void                   triggerEvent(void)                              { triggerSem.release(1); }
  void                          resubscribe(void);

  inline qint32                 sessionId(void) const                           { return sid; }
  bool                          isActive(void) const;

protected:
  virtual void                  run(void);

public:
  static const unsigned         maxInstances;
  static const int              timeout;

private:
  static QAtomicInt             sidCounter;
  const int                     sid;

  DlnaServer            * const parent;
  const QUrl                    eventUrl;
  QSemaphore                    triggerSem;
  QDateTime                     lastSubscribe;
  volatile bool                 running;
  volatile bool                 dirty;
  qint32                        updateId;
  QSet<qint32>                  containers;
  QTime                         lastUpdate;
};

struct DlnaServer::StreamSettings
{
  QString                       host;
  QString                       transcodeSize;
  QString                       transcodeCrop;
  QString                       encodeMode;
  QString                       transcodeChannels;
  QString                       transcodeMusicChannels;
};

struct DlnaServer::ItemData
{
  inline                        ItemData(void) : updateId(0)                    { }

  QString                       path;
  Item                          item;
  QVector<ItemID>               children;
  unsigned                      updateId;
};

struct DlnaServer::Private : DlnaServer::Callback
{
  inline                        Private(void) : lock(QReadWriteLock::Recursive) { }

  virtual int                   countDlnaItems(const QString &path);
  virtual QList<Item>           listDlnaItems(const QString &path, unsigned start, unsigned count);

  static const QString          upnpNS, evntNS, cdirNS, soapNS, didlNS, dcNS;

  HttpServer                  * httpServer;

  QReadWriteLock                lock;
  QMap<int, EventSession *>     eventSessions;
  int                           cleanTimer;
  QMap<ItemID, ItemData>        itemData;
  volatile ItemID               idCounter;
  QMap<QString, Callback *>     callbacks;
};

const QString DlnaServer::Private::upnpNS   = "urn:schemas-upnp-org:metadata-1-0/upnp/";
const QString DlnaServer::Private::evntNS   = "urn:schemas-upnp-org:event-1-0";
const QString DlnaServer::Private::cdirNS   = "urn:schemas-upnp-org:service:ContentDirectory:1";
const QString DlnaServer::Private::soapNS   = "http://schemas.xmlsoap.org/soap/envelope/";
const QString DlnaServer::Private::didlNS   = "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/";
const QString DlnaServer::Private::dcNS     = "http://purl.org/dc/elements/1.1/";
QAtomicInt DlnaServer::EventSession::sidCounter = 0x10000000;

const unsigned  DlnaServer::seekSec = 120;

DlnaServer::DlnaServer(QObject *parent)
    : QObject(parent),
      p(new Private())
{
  p->httpServer = NULL;
  p->cleanTimer = startTimer((EventSession::timeout * 1000) / 2);
  p->idCounter = 1;

  // Add the root path.
  ItemData itemData;
  itemData.path = "/";
  itemData.item.isDir = true;

  p->itemData.insert(0, itemData);
  p->callbacks.insert(itemData.path, p);
}

DlnaServer::~DlnaServer()
{
  if (p->httpServer)
    p->httpServer->unregisterCallback(this);

  p->lock.lockForWrite();

  foreach (EventSession *session, p->eventSessions)
    delete session;

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void DlnaServer::initialize(HttpServer *httpServer, SsdpServer *ssdpServer)
{
  p->httpServer = httpServer;
  httpServer->registerCallback("/upnp/", this);

  ssdpServer->publish("upnp:rootdevice", httpServer, "/upnp/devicedescr.xml");
  ssdpServer->publish("urn:schemas-upnp-org:device:MediaServer:1", httpServer, "/upnp/devicedescr.xml");
  ssdpServer->publish("urn:schemas-upnp-org:service:ContentDirectory:1", httpServer, "/upnp/devicedescr.xml");
}

void DlnaServer::close(void)
{
  if (p->httpServer)
    p->httpServer->unregisterCallback(this);
}

void DlnaServer::registerCallback(const QString &path, Callback *callback)
{
  SDebug::WriteLocker l(&p->lock, __FILE__, __LINE__);

  p->callbacks.insert(path, callback);
}

void DlnaServer::unregisterCallback(Callback *callback)
{
  SDebug::WriteLocker l(&p->lock, __FILE__, __LINE__);

  for (QMap<QString, Callback *>::Iterator i=p->callbacks.begin(); i!=p->callbacks.end(); )
  if (i.value() == callback)
    i = p->callbacks.erase(i);
  else
    i++;
}

void DlnaServer::update(const QString &path)
{
  SDebug::WriteLocker l(&p->lock, __FILE__, __LINE__);

  for (QMap<ItemID, ItemData>::ConstIterator i=p->itemData.begin(); i!=p->itemData.end(); i++)
  if (i->path == path)
  foreach (EventSession *session, p->eventSessions)
    session->update(i.key());
}

void DlnaServer::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == p->cleanTimer)
  {
    if (p->lock.tryLockForWrite(0))
    {
      // Cleanup old sessions.
      foreach (EventSession *session, p->eventSessions)
      if (!session->isActive())
      {
        p->eventSessions.remove(session->sessionId());
        delete session;
      }

      p->lock.unlock();
    }
  }
  else
    QObject::timerEvent(e);
}

HttpServer::SocketOp DlnaServer::handleHttpRequest(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  if ((request.path() == "/upnp/contentdircontrol") && (request.method() == "POST"))
  {
    QTime timer;
    timer.start();

    QByteArray data = socket->readAll();
    while ((data.count() < int(request.contentLength())) && (qAbs(timer.elapsed()) < 5000))
    if (socket->waitForReadyRead())
      data += socket->readAll();

    if (data.count() > 0)
    {
      QDomDocument envelope("Envelope");
      if (envelope.setContent(data))
      {
        const QDomElement browseElem =
            envelope.documentElement().firstChildElement("s:Body")
                                      .firstChildElement("u:Browse");

        if (!browseElem.isNull())
          return handleBrowse(browseElem, request, socket);
      }
    }
  }
  else if (request.path() == "/upnp/contentdireventsub")
  {
    if (request.method() == "SUBSCRIBE")
    {
      if (request.hasField("SID"))
      { // Update subscription
        const int sid = request.field("SID").toInt(NULL, 16);

        SDebug::WriteLocker l(&p->lock, __FILE__, __LINE__);

        QMap<int, EventSession *>::Iterator i = p->eventSessions.find(sid);
        if (i != p->eventSessions.end())
        {
          (*i)->resubscribe();
          l.unlock();

          HttpServer::ResponseHeader response(HttpServer::Status_Ok);
          response.setField("SID", QString::number(sid, 16));
          response.setField("Timeout", "Second-" + QString::number(EventSession::timeout));
          response.setField("Server", SsdpServer::getServerId());
          socket->write(response);
          return HttpServer::SocketOp_Close;
        }
        else
        {
          l.unlock();

          socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
          return HttpServer::SocketOp_Close;
        }
      }
      else if (request.hasField("Callback"))
      { // New subscription
        SDebug::WriteLocker l(&p->lock, __FILE__, __LINE__);

        // Prevent creating too much sessions.
        if (p->eventSessions.count() < int(EventSession::maxInstances))
        {
          EventSession * const session =
              new EventSession(this, request.field("Callback").replace('<', "").replace('>', ""));
          p->eventSessions[session->sessionId()] = session;

          l.unlock();

          HttpServer::ResponseHeader response(HttpServer::Status_Ok);
          response.setField("SID", QString::number(session->sessionId(), 16));
          response.setField("Timeout", "Second-" + QString::number(EventSession::timeout));
          response.setField("Server", SsdpServer::getServerId());
          socket->write(response);
          if (socket->waitForBytesWritten(5000))
            session->triggerEvent();

          return HttpServer::SocketOp_Close;
        }
        else
        {
          l.unlock();

          socket->write(HttpServer::ResponseHeader(HttpServer::Status_InternalServerError));
          return HttpServer::SocketOp_Close;
        }
      }
    }
    else if (request.method() == "UNSUBSCRIBE")
    {
      const int sid = request.field("SID").toInt(NULL, 16);

      SDebug::WriteLocker l(&p->lock, __FILE__, __LINE__);

      QMap<int, EventSession *>::Iterator i = p->eventSessions.find(sid);
      if (i != p->eventSessions.end())
      {
        delete *i;
        p->eventSessions.erase(i);

        l.unlock();

        HttpServer::ResponseHeader response(HttpServer::Status_Ok);
        response.setContentType("text/xml;charset=utf-8");
        response.setField("Server", SsdpServer::getServerId());
        socket->write(response);
        return HttpServer::SocketOp_Close;
      }
      else
      {
        l.unlock();

        socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
        return HttpServer::SocketOp_Close;
      }
    }
  }
  else if ((request.path() == "/upnp/devicedescr.xml") ||
           (request.path() == "/upnp/contentdirdescr.xml"))
  {
    HttpServer::ResponseHeader response(HttpServer::Status_Ok);
    response.setContentType("text/xml;charset=utf-8");
    response.setField("Cache-Control", "no-cache");
    response.setField("Accept-Ranges", "bytes");
    response.setField("Connection", "close");
    response.setField("contentFeatures.dlna.org", "");
    response.setField("Server", SsdpServer::getServerId());
    socket->write(response);

    HtmlParser localParser;
    localParser.setField("UUID", SsdpServer::getUuid());
    localParser.setField("BASEURL", "http://" + request.host() + "/");
    socket->write(localParser.parseFile(":" + request.path()));

    return HttpServer::SocketOp_Close;
  }

  qWarning() << "DlnaServer: Could not handle request:" << request.method() << request.path();
  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}

HttpServer::SocketOp DlnaServer::handleBrowse(const QDomElement &elem, const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QDomElement objectId = elem.firstChildElement("ObjectID");
  const QDomElement browseFlag = elem.firstChildElement("BrowseFlag");
  //const QDomElement filter = elem.firstChildElement("Filter");
  const QDomElement startingIndex = elem.firstChildElement("StartingIndex");
  const QDomElement requestedCount = elem.firstChildElement("RequestedCount");

  // Find the settings for this node
  GlobalSettings settings;
  settings.beginGroup("DLNA");

  const QString genericTranscodeSize =
      settings.value("TranscodeSize", settings.defaultTranscodeSizeName()).toString();
  const QString genericTranscodeCrop =
      settings.value("TranscodeCrop", settings.defaultTranscodeCropName()).toString();
  const QString genericEncodeMode =
      settings.value("EncodeMode", settings.defaultEncodeModeName()).toString();
  const QString genericTranscodeChannels =
      settings.value("TranscodeChannels", settings.defaultTranscodeChannelName()).toString();
  const QString genericTranscodeMusicChannels =
      settings.value("TranscodeMusicChannels", settings.defaultTranscodeMusicChannelName()).toString();

  settings.beginGroup("Client_" + socket->peerAddress().toString());
  settings.setValue("LastSeen", QDateTime::currentDateTime());
  if (request.hasField("User-Agent"))
    settings.setValue("UserAgent", request.field("User-Agent"));

  StreamSettings streamSettings;
  streamSettings.host = request.host();
  streamSettings.transcodeSize = settings.value("TranscodeSize", genericTranscodeSize).toString();
  streamSettings.transcodeCrop = settings.value("TranscodeCrop", genericTranscodeCrop).toString();
  streamSettings.encodeMode = settings.value("EncodeMode", genericEncodeMode).toString();
  streamSettings.transcodeChannels = settings.value("TranscodeChannels", genericTranscodeChannels).toString();
  streamSettings.transcodeMusicChannels = settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString();

  // Find requested directory
  SDebug::WriteLocker l(&p->lock, __FILE__, __LINE__);

  const ItemID pathId = fromIDString(objectId.text());
  QMap<ItemID, ItemData>::Iterator itemData = p->itemData.find(pathId);
  if (itemData == p->itemData.end())
  {
    socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
    return HttpServer::SocketOp_Close;
  }

  QDomDocument doc;
  QDomElement root = doc.createElementNS(p->soapNS, "s:Envelope");
  root.setAttributeNS(p->soapNS, "s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
  doc.appendChild(root);
  QDomElement body = doc.createElementNS(p->soapNS, "s:Body");
  root.appendChild(body);
  QDomElement browseResponse = doc.createElementNS(p->cdirNS, "u:BrowseResponse");
  body.appendChild(browseResponse);

  if (itemData->item.isDir)
  {
    QString dir = itemData->path;
    QMap<QString, Callback *>::Iterator callback = p->callbacks.find(dir);
    while ((callback == p->callbacks.end()) && !dir.isEmpty())
    {
      dir = dir.left(dir.left(dir.length() - 1).lastIndexOf('/') + 1);
      callback = p->callbacks.find(dir);
    }

    if ((callback == p->callbacks.end()) || !itemData->path.startsWith(callback.key()))
    {
      socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
      return HttpServer::SocketOp_Close;
    }

    browseDir(
        doc, browseResponse,
        pathId, *itemData, *callback, streamSettings,
        browseFlag.text(), startingIndex.text().toUInt(), requestedCount.text().toUInt());
  }
  else
  {
    browseFile(
        doc, browseResponse,
        pathId, *itemData, streamSettings,
        browseFlag.text(), startingIndex.text().toUInt(), requestedCount.text().toUInt());
  }

  QDomElement updateIDElm = doc.createElement("UpdateID");
  updateIDElm.appendChild(doc.createTextNode(QString::number(itemData->updateId)));
  browseResponse.appendChild(updateIDElm);

  const QByteArray content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + doc.toString(-1).toUtf8();

  HttpServer::ResponseHeader response(HttpServer::Status_Ok);
  response.setContentType("text/xml;charset=utf-8");
  response.setContentLength(content.length());
  response.setField("Cache-Control", "no-cache");
  response.setField("Accept-Ranges", "bytes");
  response.setField("Connection", "close");
  response.setField("contentFeatures.dlna.org", "");
  response.setField("Server", SsdpServer::getServerId());

  socket->write(response);
  socket->write(content);
  return HttpServer::SocketOp_Close;
}

void DlnaServer::browseDir(QDomDocument &doc, QDomElement &browseResponse, ItemID pathId, ItemData &itemData, Callback *callback, const StreamSettings &streamSettings, const QString &browseFlag, unsigned start, unsigned count)
{
  QDomElement result = doc.createElement("Result");
  int totalMatches = 0, totalReturned = 0;

  if (browseFlag == "BrowseDirectChildren")
  {
    QDomDocument subDoc;
    QDomElement root = subDoc.createElementNS(p->didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", p->dcNS);
    root.setAttribute("xmlns:upnp", p->upnpNS);
    subDoc.appendChild(root);

    foreach (const Item &item, callback->listDlnaItems(itemData.path, start, count))
    {
      if (!item.isDir && (item.mode == Item::Mode_Direct))
        root.appendChild(didlFile(subDoc, streamSettings, item, addChildItem(itemData, item, true), pathId));
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
    QDomElement root = subDoc.createElementNS(p->didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", p->dcNS);
    root.setAttribute("xmlns:upnp", p->upnpNS);
    subDoc.appendChild(root);

    if (!itemData.item.isDir && (itemData.item.mediaInfo.isNull() || !itemData.item.mimeType.startsWith("video")))
      root.appendChild(didlFile(subDoc, streamSettings, itemData.item, pathId));
    else
      root.appendChild(didlDirectory(subDoc, itemData.item, pathId));

    totalMatches = totalReturned = 1;

    result.appendChild(doc.createTextNode(subDoc.toString(-1) + "\n"));
  }

  browseResponse.appendChild(result);
  QDomElement numReturnedElm = doc.createElement("NumberReturned");
  numReturnedElm.appendChild(doc.createTextNode(QString::number(totalReturned)));
  browseResponse.appendChild(numReturnedElm);
  QDomElement totalMatchesElm = doc.createElement("TotalMatches");
  totalMatchesElm.appendChild(doc.createTextNode(QString::number(totalMatches)));
  browseResponse.appendChild(totalMatchesElm);
}

void DlnaServer::browseFile(QDomDocument &doc, QDomElement &browseResponse, ItemID pathId, ItemData &itemData, const StreamSettings &streamSettings, const QString &browseFlag, unsigned start, unsigned count)
{
  QDomElement result = doc.createElement("Result");
  int totalMatches = 0, totalReturned = 0;

  if (browseFlag == "BrowseDirectChildren")
  {
    QDomDocument subDoc;
    QDomElement root = subDoc.createElementNS(p->didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", p->dcNS);
    root.setAttribute("xmlns:upnp", p->upnpNS);
    subDoc.appendChild(root);

    QVector<ItemID> all;
    switch (Item::Mode(itemData.item.mode))
    {
    case Item::Mode_Default:
      if (!itemData.item.mediaInfo.isNull())
      {
        const QList<SMediaInfo::AudioStreamInfo> audioStreams = itemData.item.mediaInfo.audioStreams();
        const QList<SMediaInfo::DataStreamInfo> dataStreams =
            itemData.item.mediaInfo.dataStreams() <<
            SIOInputNode::DataStreamInfo(SIOInputNode::DataStreamInfo::Type_Subtitle, 0xFFFF, NULL, SDataCodec());

        if ((audioStreams.count() > 1) || (dataStreams.count() > 1))
        {
          for (int a=0, an=audioStreams.count(); a < an; a++)
          for (int d=0, dn=dataStreams.count(); d < dn; d++)
          {
            Item item = itemData.item;
            item.played = false;
            item.mode = Item::Mode_PlaySeek;

            item.title = QString::number(a + 1) + ". ";
            if (audioStreams[a].language[0])
              item.title += SStringParser::iso639Language(audioStreams[a].language);
            else
              item.title += tr("Unknown");

            item.url.addQueryItem("language", QString::number(audioStreams[a], 16));

            if (dataStreams[d].streamId() != 0xFFFF)
            {
              item.title += ", " + QString::number(d + 1) + ". ";
              if (dataStreams[d].language[0])
                item.title += SStringParser::iso639Language(dataStreams[d].language);
              else
                item.title += tr("Unknown");

              item.url.addQueryItem("subtitles", QString::number(dataStreams[d], 16));
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

        if (!itemData.item.mediaInfo.isNull())
        {
          if (itemData.item.mediaInfo.duration().isValid())
          {
            Item seekItem = itemData.item;
            seekItem.played = false;
            seekItem.mode = Item::Mode_Seek;
            seekItem.title = tr("Seek");
            all += addChildItem(itemData, seekItem, true);
          }

          if (itemData.item.mediaInfo.chapters().count() > 1)
          {
            Item seekItem = itemData.item;
            seekItem.played = false;
            seekItem.mode = Item::Mode_Chapters;
            seekItem.title = tr("Chapters");
            all += addChildItem(itemData, seekItem, true);
          }
        }
      }
      break;

    case Item::Mode_Seek:
      if (!itemData.item.mediaInfo.isNull())
      for (STime i=STime::fromSec(0); i<itemData.item.mediaInfo.duration(); i+=STime::fromSec(seekSec))
      {
        Item item = itemData.item;
        item.played = false;
        item.mode = Item::Mode_Direct;
        item.title = tr("Play from") + " " + QTime().addSecs(i.toSec()).toString("h:mm");
        item.url.addQueryItem("position", QString::number(i.toSec()));
        all += addChildItem(itemData, item, false);
      }
      break;

    case Item::Mode_Chapters:
      if (!itemData.item.mediaInfo.isNull())
      {
        int chapterNum = 1;
        foreach (const SMediaInfo::Chapter &chapter, itemData.item.mediaInfo.chapters())
        {
          Item item = itemData.item;
          item.played = false;
          item.mode = Item::Mode_Direct;

          item.title = tr("Chapter") + " " + QString::number(chapterNum++);
          if (!chapter.title.isEmpty())
            item.title += ", " + chapter.title;

          item.url.addQueryItem("position", QString::number(chapter.begin.toSec()));
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
      QMap<ItemID, ItemData>::Iterator childData = p->itemData.find(all[i]);

      if (childData->item.mode != Item::Mode_Direct)
        root.appendChild(didlDirectory(subDoc, childData->item, childData.key(), pathId));
      else
        root.appendChild(didlFile(subDoc, streamSettings, childData->item, childData.key(), pathId));

      totalReturned++;
    }

    totalMatches = all.count();

    result.appendChild(doc.createTextNode(subDoc.toString(-1) + "\n"));
  }
  else if (browseFlag == "BrowseMetadata")
  {
    QDomDocument subDoc;
    QDomElement root = subDoc.createElementNS(p->didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", p->dcNS);
    root.setAttribute("xmlns:upnp", p->upnpNS);
    subDoc.appendChild(root);

    if (itemData.item.mode != Item::Mode_Direct)
      root.appendChild(didlDirectory(subDoc, itemData.item, pathId));
    else
      root.appendChild(didlFile(subDoc, streamSettings, itemData.item, pathId));

    totalMatches = totalReturned = 1;

    result.appendChild(doc.createTextNode(subDoc.toString(-1) + "\n"));
  }

  browseResponse.appendChild(result);
  QDomElement numReturnedElm = doc.createElement("NumberReturned");
  numReturnedElm.appendChild(doc.createTextNode(QString::number(totalReturned)));
  browseResponse.appendChild(numReturnedElm);
  QDomElement totalMatchesElm = doc.createElement("TotalMatches");
  totalMatchesElm.appendChild(doc.createTextNode(QString::number(totalMatches)));
  browseResponse.appendChild(totalMatchesElm);
}

DlnaServer::ItemID DlnaServer::addChildItem(ItemData &itemData, const Item &item, bool asDir)
{
  SDebug::WriteLocker l(&p->lock, __FILE__, __LINE__);

  const QString fullPath = itemData.path + item.title + (asDir ? "/" : "");

  // Find existing child
  foreach (ItemID childId, itemData.children)
  {
    QMap<ItemID, ItemData>::Iterator childData = p->itemData.find(childId);
    if (childData != p->itemData.end())
    if (childData->path == fullPath)
    {
      childData->item = item;
      return childId;
    }
  }

  // Add new child
  const ItemID childId = p->idCounter++;
  ItemData childData;
  childData.path = fullPath;
  childData.item = item;
  p->itemData.insert(childId, childData);

  itemData.children.append(childId);

  return childId;
}

QDomElement DlnaServer::didlDirectory(QDomDocument &doc, const Item &dir, ItemID id, ItemID parentId)
{
  QDomElement containerElm = doc.createElement("container");
  containerElm.setAttribute("id", toIDString(id));
  containerElm.setAttribute("searchable", "false");
  containerElm.setAttribute("restricted", "false");

  if (parentId != 0)
    containerElm.setAttribute("parentID", toIDString(parentId));

  QDomElement titleElm = doc.createElement("dc:title");
  titleElm.appendChild(doc.createTextNode((dir.played ? "*" : "") + dir.title));
  containerElm.appendChild(titleElm);

  QDomElement upnpClassElm = doc.createElement("upnp:class");
  upnpClassElm.appendChild(doc.createTextNode("object.container.storageFolder"));
  containerElm.appendChild(upnpClassElm);

  return containerElm;
}

QDomElement DlnaServer::didlFile(QDomDocument &doc, const StreamSettings &streamSettings, const Item &item, ItemID id, ItemID parentId)
{
  QDomElement itemElm = doc.createElement("item");
  itemElm.setAttribute("id", toIDString(id));
  itemElm.setAttribute("restricted", "false");

  if (parentId != 0)
    itemElm.setAttribute("parentID", toIDString(parentId));

  QDomElement titleElm = doc.createElement("dc:title");
  titleElm.appendChild(doc.createTextNode((item.played ? "*" : "") + item.title));
  itemElm.appendChild(titleElm);

  if (!item.mediaInfo.isNull())
  {
    QDomElement artistElm = doc.createElement("upnp:artist");
    artistElm.appendChild(doc.createTextNode(item.mediaInfo.author()));
    itemElm.appendChild(artistElm);

    QDomElement actorElm = doc.createElement("upnp:actor");
    actorElm.appendChild(doc.createTextNode(item.mediaInfo.author()));
    itemElm.appendChild(actorElm);

    QDomElement albumElm = doc.createElement("upnp:album");
    albumElm.appendChild(doc.createTextNode(item.mediaInfo.album()));
    itemElm.appendChild(albumElm);

    if (item.mediaInfo.year() > 0)
    {
      QDomElement dateElm = doc.createElement("dc:date");
      dateElm.appendChild(doc.createTextNode(QString::number(item.mediaInfo.year())));
      itemElm.appendChild(dateElm);
    }
  }

  if (!item.iconUrl.isEmpty())
  {
    QDomElement iconElm = doc.createElement("upnp:icon");
    iconElm.appendChild(doc.createTextNode(item.iconUrl.toString()));
    itemElm.appendChild(iconElm);
  }

  QUrl url = item.url;
  url.setScheme("http");
  url.setAuthority(streamSettings.host);

  foreach (const GlobalSettings::TranscodeSize &size, GlobalSettings::allTranscodeSizes())
  if (size.name == streamSettings.transcodeSize)
  {
    QString sizeStr =
        QString::number(size.size.width()) + "x" +
        QString::number(size.size.height()) + "x" +
        QString::number(size.size.aspectRatio(), 'f', 3);

    if (!streamSettings.transcodeCrop.isEmpty())
      sizeStr += "/" + streamSettings.transcodeCrop.toLower();

    url.addQueryItem("size", sizeStr);
    break;
  }

  const QString transcodeChannels = item.music ? streamSettings.transcodeMusicChannels : streamSettings.transcodeChannels;
  foreach (const GlobalSettings::TranscodeChannel &channel, GlobalSettings::allTranscodeChannels())
  if (channel.name == transcodeChannels)
  {
    if (item.music)
      url.addQueryItem("forcechannels", QString::number(channel.channels, 16));
    else
      url.addQueryItem("requestchannels", QString::number(channel.channels, 16));

    break;
  }

  url.addQueryItem("priority", "high");
  if (!streamSettings.encodeMode.isEmpty())
    url.addQueryItem("encode", streamSettings.encodeMode.toLower());
  else
    url.addQueryItem("encode", "fast");

  QList<QByteArray> formats;
  QDomElement upnpClassElm = doc.createElement("upnp:class");
  if (item.mimeType.startsWith("video/"))
  {
    upnpClassElm.appendChild(doc.createTextNode("object.item.videoItem"));
    formats += "video/mpeg.mpeg";
    //formats += "video/ogg.ogv";
    formats += "video/x-flv.flv";
  }
  else if (item.mimeType.startsWith("audio/"))
  {
    upnpClassElm.appendChild(doc.createTextNode("object.item.audioItem"));
    formats += "audio/mpeg.mpa";
    //formats += "audio/ogg.oga";
    formats += "audio/wave.wav";
    formats += "video/x-flv.flv";
  }
  else if (item.mimeType.startsWith("image/"))
  {
    upnpClassElm.appendChild(doc.createTextNode("object.item.imageItem"));
    formats += "image/jpeg.jpeg";
  }
  else
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

    if (!item.mediaInfo.isNull() && item.mediaInfo.duration().isValid())
      resElm.setAttribute("duration", QTime().addSecs(item.mediaInfo.duration().toSec()).toString("h:mm:ss.zzz"));

    QUrl u = url;
    u.setPath(u.path() + suffix);
    //qDebug() << resElm.attribute("protocolInfo") << u.toString();

    resElm.appendChild(doc.createTextNode(u.toString()));
    itemElm.appendChild(resElm);
  }

  return itemElm;
}

const unsigned  DlnaServer::EventSession::maxInstances = 64;
const int       DlnaServer::EventSession::timeout = 180;

DlnaServer::EventSession::EventSession(DlnaServer *parent, const QString &eventUrl)
    : sid(sidCounter.fetchAndAddOrdered(1)),
      parent(parent),
      eventUrl(eventUrl),
      triggerSem(0),
      lastSubscribe(QDateTime::currentDateTime()),
      running(true),
      dirty(true),
      updateId(1)
{
  QThread::start(QThread::LowPriority);
}

DlnaServer::EventSession::~EventSession()
{
  running = false;
  triggerSem.release(1);
  QThread::wait();
}

void DlnaServer::EventSession::update(qint32 dirId)
{
  SDebug::WriteLocker l(&parent->p->lock, __FILE__, __LINE__);

  updateId++;
  containers += dirId;
  lastUpdate.start();
  dirty = true;
}

void DlnaServer::EventSession::resubscribe(void)
{
  SDebug::WriteLocker l(&parent->p->lock, __FILE__, __LINE__);

  lastSubscribe = QDateTime::currentDateTime();
}

bool DlnaServer::EventSession::isActive(void) const
{
  if (QThread::isRunning())
  {
    SDebug::WriteLocker l(&parent->p->lock, __FILE__, __LINE__);

    if (lastSubscribe.secsTo(QDateTime::currentDateTime()) < (timeout + 30))
      return true;
  }

  return false;
}

void DlnaServer::EventSession::run(void)
{
  while (running)
  {
    triggerSem.tryAcquire(1, 2000);

    if (running && dirty)
    if (parent->p->lock.tryLockForWrite(250))
    {
      if (lastUpdate.isNull() || (qAbs(lastUpdate.elapsed()) >= 2000)) // Max rate 0.5 Hz
      {
        dirty = false;

        // Build the message
        QHttpRequestHeader header("NOTIFY", eventUrl.path());
        header.setContentType("text/xml;charset=utf-8");
        header.setValue("Host", eventUrl.host() + ":" + QString::number(eventUrl.port(80)));
        header.setValue("NT", "upnp:event");
        header.setValue("NTS", "upnp:propchange");
        header.setValue("SID", QString::number(sid, 16));
        header.setValue("SEQ", "0");

        QDomDocument doc;
        QDomElement root = doc.createElementNS(parent->p->evntNS, "e:propertyset");
        doc.appendChild(root);
        QDomElement property1 = doc.createElementNS(parent->p->evntNS, "e:property");
        root.appendChild(property1);
        QDomElement systemUpdateID = doc.createElement("SystemUpdateID");
        systemUpdateID.appendChild(doc.createTextNode(QString::number(updateId)));
        property1.appendChild(systemUpdateID);
        QDomElement property2 = doc.createElementNS(parent->p->evntNS, "e:property");
        root.appendChild(property2);
        QDomElement containerUpdateIDs = doc.createElement("ContainerUpdateIDs");

        QString updateIds;
        foreach (qint32 containerId, containers)
        {
          QMap<ItemID, ItemData>::ConstIterator i = parent->p->itemData.find(containerId);
          if (i != parent->p->itemData.end())
            updateIds += toIDString(containerId) + "," + QString::number(i->updateId) + ",";
        }

        if (!updateIds.isEmpty())
          containerUpdateIDs.appendChild(doc.createTextNode(updateIds.left(updateIds.length() - 1)));

        containers.clear();

        property2.appendChild(containerUpdateIDs);
        QDomElement property3 = doc.createElementNS(parent->p->evntNS, "e:property");
        root.appendChild(property3);
        property3.appendChild(doc.createElement("TransferIDs"));
        const QByteArray content = doc.toString(-1).toUtf8();

        header.setContentLength(content.length());

        lastUpdate.start();
        parent->p->lock.unlock();

        // Now send the message
        static const int maxRequestTime = 5000;
        QTime timer; timer.start();

        QTcpSocket socket;
        socket.connectToHost(eventUrl.host(), eventUrl.port(80));
        if (socket.waitForConnected(qMax(maxRequestTime - timer.elapsed(), 10)))
        {
          socket.write(header.toString().toUtf8() + content);
          if (socket.waitForBytesWritten(qMax(maxRequestTime - timer.elapsed(), 10)))
          {
            QByteArray response;

            while (socket.waitForReadyRead(qMax(maxRequestTime - timer.elapsed(), 10)))
            while (socket.canReadLine())
            {
              const QByteArray line = socket.readLine();

              if (line.trimmed().length() > 0)
              {
                response += line;
              }
              else // Header complete
              {
                response += line;

                const HttpServer::ResponseHeader responseHeader(response);
                if (responseHeader.status() != HttpServer::Status_Ok)
                {
                  qWarning() << "DlnaServer: got error response:" << response;
                  running = false;
                }

                socket.close();
                break;
              }
            }
          }
        }
      }
      else
        parent->p->lock.unlock();
    }
  }
}

int DlnaServer::Private::countDlnaItems(const QString &)
{
  return callbacks.count() - 1;
}

QList<DlnaServer::Item> DlnaServer::Private::listDlnaItems(const QString &, unsigned start, unsigned count)
{
  QList<DlnaServer::Item> result;

  for (QMap<QString, Callback *>::ConstIterator i=callbacks.begin(); i!=callbacks.end(); i++)
  if (i.key() != "/")
  {
    DlnaServer::Item item;
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
