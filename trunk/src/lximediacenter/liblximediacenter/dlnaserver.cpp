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

#include <liblximediacenter/dlnaserver.h>
#include <liblximediacenter/htmlparser.h>
#include <liblximediacenter/ssdpserver.h>

namespace LXiMediaCenter {


const quint32 DlnaServer::defaultSortOrder = 0x80000000;
const qint32  DlnaServer::cacheTimeout = 300000;

class DlnaServer::HttpDir : public HttpServerDir
{
public:
  explicit                      HttpDir(HttpServer *server, DlnaServer *parent);

protected:
  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

private:
  DlnaServer       * const parent;
};

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
  const qint32                  sid;

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

struct DlnaServer::Private
{
  static const QString          upnpNS, evntNS, cdirNS, soapNS, didlNS, dcNS;

  HttpServer                  * httpServer;
  HttpDir                     * httpDir;

  QMap<int, DlnaServerDir *>    idMap;
  QAtomicInt                    idCounter;

  int                           cleanTimer;
  QMap<int, EventSession *>     eventSessions;
  QMap<int, DirCacheEntry>      dirCacheEntries;
  QSet<int>                     dirsToUpdate;
  QAtomicInt                    updateTasksRunning;
};

const QString DlnaServer::Private::upnpNS   = "urn:schemas-upnp-org:metadata-1-0/upnp/";
const QString DlnaServer::Private::evntNS   = "urn:schemas-upnp-org:event-1-0";
const QString DlnaServer::Private::cdirNS   = "urn:schemas-upnp-org:service:ContentDirectory:1";
const QString DlnaServer::Private::soapNS   = "http://schemas.xmlsoap.org/soap/envelope/";
const QString DlnaServer::Private::didlNS   = "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/";
const QString DlnaServer::Private::dcNS     = "http://purl.org/dc/elements/1.1/";
QAtomicInt DlnaServer::EventSession::sidCounter = 0x10000000;


DlnaServer::DlnaServer(HttpServer *httpServer, QObject *parent)
    : QObject(parent),
      p(new Private())
{
  p->httpServer = httpServer;
  p->httpDir = NULL;
  p->idCounter = 1;
  p->cleanTimer = startTimer((EventSession::timeout * 1000) / 2);

  DlnaServerDir * const rootDir = new DlnaServerDir(this);
  setRoot(rootDir);
  p->idMap.insert(0, rootDir);
}

DlnaServer::~DlnaServer()
{
  QThreadPool::globalInstance()->waitForDone();

  mutex.lock();

  foreach (EventSession *session, p->eventSessions)
    delete session;

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void DlnaServer::initialize(SsdpServer *ssdpServer)
{
  p->httpDir = new HttpDir(p->httpServer, this);
  p->httpServer->addDir("/upnp", p->httpDir);

  ssdpServer->publish("upnp:rootdevice", p->httpServer, "/upnp/devicedescr.xml");
  ssdpServer->publish("urn:schemas-upnp-org:device:MediaServer:1", p->httpServer, "/upnp/devicedescr.xml");
  ssdpServer->publish("urn:schemas-upnp-org:service:ContentDirectory:1", p->httpServer, "/upnp/devicedescr.xml");
}

void DlnaServer::close(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  delete p->httpDir;
  p->httpDir = NULL;
}

void DlnaServer::update(qint32 dirId)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  foreach (EventSession *session, p->eventSessions)
    session->update(dirId);
}

DlnaServerDir * DlnaServer::getDir(qint32 dirId)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  QMap<int, DlnaServerDir *>::Iterator i = p->idMap.find(dirId);
  if (i != p->idMap.end())
    return *i;

  return NULL;
}

const DlnaServerDir * DlnaServer::getDir(qint32 dirId) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  QMap<int, DlnaServerDir *>::ConstIterator i = p->idMap.find(dirId);
  if (i != p->idMap.end())
    return *i;

  return NULL;
}

bool DlnaServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  if ((request.path() == "/upnp/contentdircontrol") && (request.method() == "POST"))
  {
    QTime timer;
    timer.start();

    QByteArray data = socket->readAll();
    while ((data.count() < int(request.contentLength())) && (qAbs(timer.elapsed()) < 250))
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
      if (request.hasKey("SID"))
      { // Update subscription
        const int sid = request.value("SID").toInt(NULL, 16);

        SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

        QMap<int, EventSession *>::Iterator i = p->eventSessions.find(sid);
        if (i != p->eventSessions.end())
        {
          (*i)->resubscribe();
          l.unlock();

          QHttpResponseHeader response(200);
          response.setValue("SID", QString::number(sid, 16));
          response.setValue("Timeout", "Second-" + QString::number(EventSession::timeout));
          response.setValue("Server", SsdpServer::getServerId());
          socket->write(response.toString().toUtf8());
          return false;
        }
        else
        {
          l.unlock();

          socket->write(QHttpResponseHeader(404).toString().toUtf8());
          return false;
        }
      }
      else if (request.hasKey("Callback"))
      { // New subscription
        SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

        // Prevent creating too much sessions.
        if (p->eventSessions.count() < int(EventSession::maxInstances))
        {
          EventSession * const session =
              new EventSession(this, request.value("Callback").replace('<', "").replace('>', ""));
          p->eventSessions[session->sessionId()] = session;

          l.unlock();

          QHttpResponseHeader response(200);
          response.setValue("SID", QString::number(session->sessionId(), 16));
          response.setValue("Timeout", "Second-" + QString::number(EventSession::timeout));
          response.setValue("Server", SsdpServer::getServerId());
          socket->write(response.toString().toUtf8());
          if (socket->waitForBytesWritten(5000))
            session->triggerEvent();

          return false;
        }
        else
        {
          l.unlock();

          socket->write(QHttpResponseHeader(500).toString().toUtf8());
          return false;
        }
      }
    }
    else if (request.method() == "UNSUBSCRIBE")
    {
      const int sid = request.value("SID").toInt(NULL, 16);

      SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

      QMap<int, EventSession *>::Iterator i = p->eventSessions.find(sid);
      if (i != p->eventSessions.end())
      {
        delete *i;
        p->eventSessions.erase(i);

        l.unlock();

        QHttpResponseHeader response(200);
        response.setContentType("text/xml;charset=utf-8");
        response.setValue("Server", SsdpServer::getServerId());
        socket->write(response.toString().toUtf8());
        return false;
      }
      else
      {
        l.unlock();

        socket->write(QHttpResponseHeader(404).toString().toUtf8());
        return false;
      }
    }
  }
  else if ((request.path() == "/upnp/devicedescr.xml") || (request.path() == "/upnp/contentdirdescr.xml"))
  {
    QHttpResponseHeader response(200);
    response.setContentType("text/xml;charset=utf-8");
    response.setValue("Cache-Control", "no-cache");
    response.setValue("Accept-Ranges", "bytes");
    response.setValue("Connection", "close");
    response.setValue("contentFeatures.dlna.org", "");
    response.setValue("Server", SsdpServer::getServerId());
    socket->write(response.toString().toUtf8());

    HtmlParser localParser;
    localParser.setField("UUID", SsdpServer::getUuid());
    localParser.setField("BASEURL", "http://" + request.value("Host") + "/");
    socket->write(localParser.parseFile(":" + request.path()));

    return false;
  }

  qDebug() << "DlnaServer: Could not handle request:" << request.method() << request.path();
  socket->write(QHttpResponseHeader(404).toString().toUtf8());
  return false;
}

void DlnaServer::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == p->cleanTimer)
  {
    if (mutex.tryLock(0))
    {
      // Cleanup old sessions.
      foreach (EventSession *session, p->eventSessions)
      if (!session->isActive())
      {
        p->eventSessions.remove(session->sessionId());
        delete session;
      }

      mutex.unlock();
    }
  }
  else
    QObject::timerEvent(e);
}

QMap<qint32, DlnaServer::DirCacheEntry>::Iterator DlnaServer::updateCacheEntry(int dirId)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  qint32 updateId = 0;

  // Update the cache entry for this directory
  QMap<qint32, DirCacheEntry>::Iterator ce = p->dirCacheEntries.find(dirId);
  if (ce != p->dirCacheEntries.end())
  {
    qint32 u = ce->updateId;
    QMap<qint32, DlnaServerDir *>::Iterator dir = p->idMap.find(dirId);
    if (dir != p->idMap.end())
      u = (*dir)->updateId;

    if ((qAbs(ce->update.elapsed()) >= cacheTimeout) || (ce->updateId != u))
    {
      p->dirCacheEntries.erase(ce);

      // Remove other obsolete cache items
      for (QMap<qint32, DirCacheEntry>::Iterator i=p->dirCacheEntries.begin(); i!=p->dirCacheEntries.end(); )
      {
        qint32 u = i->updateId;
        QMap<qint32, DlnaServerDir *>::Iterator dir = p->idMap.find(i.key());
        if (dir != p->idMap.end())
          u = (*dir)->updateId;

        if ((qAbs(i->update.elapsed()) >= cacheTimeout) || (i->updateId != u))
          i = p->dirCacheEntries.erase(i);
        else
          i++;
      }

      ce = p->dirCacheEntries.end();
    }
  }

  if ((ce == p->dirCacheEntries.end()) || (ce->children.isEmpty()))
  {
    ce = p->dirCacheEntries.insert(dirId, DirCacheEntry());

    DlnaServerDir::FileMap files;
    DlnaServerDir::DirMap dirs;

    QMap<int, DlnaServerDir *>::Iterator i = p->idMap.find(dirId);
    if (i != p->idMap.end())
    {
      files = (*i)->listFiles();
      dirs = (*i)->listDirs();
      updateId = (*i)->updateId;
    }

    for (DlnaServerDir::FileMap::ConstIterator i=files.begin(); i!=files.end(); i++)
      ce->children.insert(
          ("00000000" + QString::number(i->sortOrder, 16)).right(8) + i.key().toUpper(),
          DirCacheEntry::Item(i.key(), *i));

    for (DlnaServerDir::DirMap::ConstIterator i=dirs.begin(); i!=dirs.end(); i++)
    {
      DlnaServerDir * const dir = qobject_cast<DlnaServerDir *>(*i);
      if (dir)
      {
        ce->children.insert(
            ("00000000" + QString::number(dir->sortOrder, 16)).right(8) + i.key().toUpper(),
            DirCacheEntry::Item(i.key(), dir));
      }
    }

    ce->updateId = updateId;
    ce->update.start();
  }

  return ce;
}

bool DlnaServer::handleBrowse(const QDomElement &elem, const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QString host = request.value("Host");
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
  if (request.hasKey("User-Agent"))
    settings.setValue("UserAgent", request.value("User-Agent"));

  const QString transcodeSize = settings.value("TranscodeSize", genericTranscodeSize).toString();
  const QString transcodeCrop = settings.value("TranscodeCrop", genericTranscodeCrop).toString();
  const QString encodeMode = settings.value("EncodeMode", genericEncodeMode).toString();
  const QString transcodeChannels = settings.value("TranscodeChannels", genericTranscodeChannels).toString();
  const QString transcodeMusicChannels = settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString();

  // Find requested directory
  const int dirId = objectId.text().split(':').first().toInt(NULL, 16);

  QDomDocument doc;
  QDomElement root = doc.createElementNS(p->soapNS, "s:Envelope");
  root.setAttributeNS(p->soapNS, "s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
  doc.appendChild(root);
  QDomElement body = doc.createElementNS(p->soapNS, "s:Body");
  root.appendChild(body);
  QDomElement browseResponse = doc.createElementNS(p->cdirNS, "u:BrowseResponse");
  body.appendChild(browseResponse);

  // Build result
  const int start = qMax(0, startingIndex.text().toInt());
  const int end = start + qMax(0, requestedCount.text().toInt());
  int count = 0, returned = 0;
  qint32 upid = 0;
  QDomElement result = doc.createElement("Result");

  if (browseFlag.text() == "BrowseDirectChildren")
  {
    SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

    QDomDocument doc;
    QDomElement root = doc.createElementNS(p->didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", p->dcNS);
    root.setAttribute("xmlns:upnp", p->upnpNS);
    doc.appendChild(root);

    // Find or build the cache entry for this directory
    QMap<qint32, DirCacheEntry>::Iterator ce = updateCacheEntry(dirId);
    upid = ce->updateId;

    // Append the relevant entries for this directory
    foreach (const DirCacheEntry::Item &item, ce->children)
    {
      if ((count >= start) && (count < end))
      {
        if ((item.id & DIR_ID_MASK) != 0)
        {
          root.appendChild(didlDirectory(doc, item, dirId));
        }
        else if ((item.id & FILE_ID_MASK) != 0)
        {
          const QString channels = item.music ? transcodeMusicChannels : transcodeChannels;

          root.appendChild(didlFile(doc, host, transcodeSize, transcodeCrop, encodeMode, channels, item, dirId, false));
        }

        returned++;
      }

      count++;
    }

    // Prevent the cache from becoming too big
    if (p->dirCacheEntries.count() > 64)
    for (QMap<int, DirCacheEntry>::Iterator i=p->dirCacheEntries.begin();
         (p->dirCacheEntries.count()>32) && (i!=p->dirCacheEntries.end()); )
    {
      if (i.key() == dirId)
        i++;
      else
        i = p->dirCacheEntries.erase(i);
    }

    result.appendChild(doc.createTextNode(doc.toString(-1) + "\n"));
  }
  else if (browseFlag.text() == "BrowseMetadata")
  {
    SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

    QDomDocument doc;
    QDomElement root = doc.createElementNS(p->didlNS, "DIDL-Lite");
    root.setAttribute("xmlns:dc", p->dcNS);
    root.setAttribute("xmlns:upnp", p->upnpNS);
    doc.appendChild(root);

    DlnaServerDir::FileMap files;
    QMap<int, DlnaServerDir *>::Iterator i = p->idMap.find(dirId);
    if (i != p->idMap.end())
    {
      files = (*i)->listFiles();
      upid = (*i)->updateId;
    }

    const int fileId = objectId.text().split(':').last().toInt(NULL, 16);
    for (DlnaServerDir::FileMap::ConstIterator i=files.begin(); i!=files.end(); i++)
    if (i->id == fileId)
    {
      const DirCacheEntry::Item item(i.key(), *i);
      const QString channels = item.music ? transcodeMusicChannels : transcodeChannels;

      root.appendChild(didlFile(doc, host, transcodeSize, transcodeCrop, encodeMode, channels, item, dirId, true));

      returned = count = 1;
      break;
    }

    result.appendChild(doc.createTextNode(doc.toString(-1) + "\n"));
  }

  browseResponse.appendChild(result);
  QDomElement numReturned = doc.createElement("NumberReturned");
  numReturned.appendChild(doc.createTextNode(QString::number(returned)));
  browseResponse.appendChild(numReturned);
  QDomElement totalMatches = doc.createElement("TotalMatches");
  totalMatches.appendChild(doc.createTextNode(QString::number(count)));
  browseResponse.appendChild(totalMatches);
  QDomElement updateID = doc.createElement("UpdateID");
  updateID.appendChild(doc.createTextNode(QString::number(upid)));
  browseResponse.appendChild(updateID);

  const QByteArray content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + doc.toString(-1).toUtf8();

  QHttpResponseHeader response(200);
  response.setContentType("text/xml;charset=utf-8");
  response.setContentLength(content.length());
  response.setValue("Cache-Control", "no-cache");
  response.setValue("Accept-Ranges", "bytes");
  response.setValue("Connection", "close");
  response.setValue("contentFeatures.dlna.org", "");
  response.setValue("Server", SsdpServer::getServerId());

  socket->write(response.toString().toUtf8());
  socket->write(content);

  return false;
}

QDomElement DlnaServer::didlDirectory(QDomDocument &doc, const DirCacheEntry::Item &item, int parentId)
{
  QDomElement containerElm = doc.createElement("container");
  containerElm.setAttribute("id", QString::number(item.id, 16));
  containerElm.setAttribute("searchable", "false");
  containerElm.setAttribute("parentID", QString::number(parentId, 16));
  containerElm.setAttribute("restricted", "false");

  QDomElement titleElm = doc.createElement("dc:title");
  titleElm.appendChild(doc.createTextNode(item.title));
  containerElm.appendChild(titleElm);

  QDomElement upnpClassElm = doc.createElement("upnp:class");
  upnpClassElm.appendChild(doc.createTextNode("object.container.storageFolder"));
  containerElm.appendChild(upnpClassElm);

  return containerElm;
}

QDomElement DlnaServer::didlFile(QDomDocument &doc, const QString &host, const QString &transcodeSize, const QString &transcodeCrop, const QString &encodeMode, const QString &transcodeChannels, const DirCacheEntry::Item &item, int parentId, bool addMeta)
{
  QDomElement itemElm = doc.createElement("item");
  itemElm.setAttribute("id", QString::number(parentId, 16) + ":" + QString::number(item.id, 16));
  itemElm.setAttribute("parentID", QString::number(parentId, 16));
  itemElm.setAttribute("restricted", "false");

  QDomElement titleElm = doc.createElement("dc:title");
  titleElm.appendChild(doc.createTextNode(item.title));
  itemElm.appendChild(titleElm);

  if (addMeta && item.date.isValid())
  {
    QDomElement creatorElm = doc.createElement("dc:date");
    creatorElm.appendChild(doc.createTextNode(item.date.toString("yyyy-MM-dd")));
    itemElm.appendChild(creatorElm);
  }

  if (addMeta && !item.iconUrl.isEmpty())
  {
    QDomElement iconElm = doc.createElement("upnp:icon");
    iconElm.appendChild(doc.createTextNode(item.iconUrl));
    itemElm.appendChild(iconElm);
  }

  if (addMeta && !item.creator.isEmpty())
  {
    QDomElement creatorElm = doc.createElement("dc:creator");
    creatorElm.appendChild(doc.createTextNode(item.creator));
    itemElm.appendChild(creatorElm);
  }

  if (addMeta && !item.description.isEmpty())
  {
    QDomElement descrElm = doc.createElement("dc:description");
    descrElm.appendChild(doc.createTextNode(item.description));
    itemElm.appendChild(descrElm);
  }

  QString imageSize = "size=0x0";
  foreach (const GlobalSettings::TranscodeSize &size, GlobalSettings::allTranscodeSizes())
  if (size.name == transcodeSize)
  {
    imageSize = "size=" + QString::number(size.size.width()) +
                "x" + QString::number(size.size.height()) +
                "x" + QString::number(size.size.aspectRatio(), 'f', 3);
    break;
  }

  if (!transcodeCrop.isEmpty())
    imageSize += "/" + transcodeCrop.toLower();

  QString channelSetup = "requestchannels=0";
  foreach (const GlobalSettings::TranscodeChannel &channel, GlobalSettings::allTranscodeChannels())
  if (channel.name == transcodeChannels)
  {
    if (item.music)
      channelSetup = "forcechannels=" + QString::number(channel.channels, 16);
    else
      channelSetup = "requestchannels=" + QString::number(channel.channels, 16);

    break;
  }

  QString encode = "encode=fast";
  if (!encodeMode.isEmpty())
    encode = "encode=" + encodeMode.toLower();

  encode += "&priority=high";

  QDomElement upnpClassElm = doc.createElement("upnp:class");
  QString postfix = item.url.contains('?') ? "&" : "?";
  if (item.mimeType.startsWith("video/"))
  {
    upnpClassElm.appendChild(doc.createTextNode("object.item.videoItem"));
    postfix += imageSize + "&" + channelSetup + "&" + encode + "&header=false";
  }
  else if (item.mimeType.startsWith("audio/"))
  {
    upnpClassElm.appendChild(doc.createTextNode("object.item.audioItem"));
    postfix += channelSetup + "&" + encode;
  }
  else if (item.mimeType.startsWith("image/"))
  {
    upnpClassElm.appendChild(doc.createTextNode("object.item.imageItem"));
    postfix += imageSize + "&" + encode;
  }
  else
  {
    upnpClassElm.appendChild(doc.createTextNode("object.item"));
    postfix = "";
  }

  itemElm.appendChild(upnpClassElm);

  QDomElement resElm = doc.createElement("res");
  resElm.setAttribute("protocolInfo", "http-get:*:" + item.mimeType + ":DLNA.ORG_PS=1;DLNA.ORG_CI=0;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=21700000000000000000000000000000");

  if (item.duration.isValid())
    resElm.setAttribute("duration", QString::number(item.duration.toSec() / 3600) + ":" +
                                    QString::number((item.duration.toSec() / 60) % 60) + ":" +
                                    QString::number(item.duration.toSec() % 60));

  resElm.appendChild(doc.createTextNode("http://" + host + item.url + postfix));
  itemElm.appendChild(resElm);

  return itemElm;
}


DlnaServer::File::File(void)
    : id(0),
      played(false),
      sortOrder(0),
      music(false)
{
}

DlnaServer::File::File(DlnaServer *parent)
    : id(parent->p->idCounter.fetchAndAddOrdered(1) | DlnaServer::FILE_ID_MASK),
      played(false),
      sortOrder(defaultSortOrder),
      music(false)
{
}


DlnaServer::HttpDir::HttpDir(HttpServer *server, DlnaServer *parent)
    : HttpServerDir(server),
      parent(parent)
{
}

bool DlnaServer::HttpDir::handleConnection(const QHttpRequestHeader &h, QAbstractSocket *s)
{
  return parent->handleConnection(h, s);
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
  SDebug::MutexLocker l(&parent->mutex, __FILE__, __LINE__);

  updateId++;
  containers += dirId;
  lastUpdate.start();
  dirty = true;
}

void DlnaServer::EventSession::resubscribe(void)
{
  SDebug::MutexLocker l(&parent->mutex, __FILE__, __LINE__);

  lastSubscribe = QDateTime::currentDateTime();
}

bool DlnaServer::EventSession::isActive(void) const
{
  if (QThread::isRunning())
  {
    SDebug::MutexLocker l(&parent->mutex, __FILE__, __LINE__);

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
    if (parent->mutex.tryLock(250))
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
          QMap<int, DlnaServerDir *>::ConstIterator i = parent->p->idMap.find(containerId);
          if (i != parent->p->idMap.end())
            updateIds += QString::number(containerId, 16) + "," + QString::number((*i)->updateId) + ",";
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
        parent->mutex.unlock();

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

                const QHttpResponseHeader responseHeader(QString::fromUtf8(response));
                if (responseHeader.statusCode() != 200)
                {
                  qDebug() << "DlnaServer: got error response:" << response;
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
        parent->mutex.unlock();
    }
  }
}


DlnaServer::DirCacheEntry::Item::Item(const QString &title, const File &file)                           
    : File(file),
      title(title)
{
}

DlnaServer::DirCacheEntry::Item::Item(const QString &title, const DlnaServerDir *dir)                           
    : title(title)
{
  id = dir->id;
  played = dir->played;
  sortOrder = dir->sortOrder;
  date = dir->date;
}


DlnaServerDir::DlnaServerDir(DlnaServer *parent)
    : FileServerDir(parent),
      id(parent->p->idCounter.fetchAndAddOrdered(1) | DlnaServer::DIR_ID_MASK),
      sortOrder(DlnaServer::defaultSortOrder),
      updateId(1)
{
  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  parent->p->idMap.insert(id, this);
}

DlnaServerDir::~DlnaServerDir()
{
  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  if (server()->p) // This might already be deleted if this is the root directory.
    server()->p->idMap.remove(id);
}

void DlnaServerDir::addFile(const QString &name, const DlnaServer::File &file)
{
  Q_ASSERT(!name.isEmpty());
  Q_ASSERT(file.id != 0);

  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  files[name] = file;

  updateId++;
  server()->update(id);
}

void DlnaServerDir::removeFile(const QString &name)
{
  Q_ASSERT(!name.isEmpty());

  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  files.remove(name);

  updateId++;
  server()->update(id);
}

void DlnaServerDir::addDir(const QString &name, FileServerDir *dir)
{
  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  FileServerDir::addDir(name, dir);

  updateId++;
  server()->update(id);
}

void DlnaServerDir::removeDir(const QString &name)
{
  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  FileServerDir::removeDir(name);

  updateId++;
  server()->update(id);
}

void DlnaServerDir::clear(void)
{
  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  const qint32 newUpdateId = updateId + 1;

  files.clear();

  FileServerDir::clear();

  updateId = newUpdateId;
  server()->update(id);
}

int DlnaServerDir::count(void) const
{
  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  return listFiles().count() + FileServerDir::count();
}

const DlnaServerDir::FileMap & DlnaServerDir::listFiles(void)
{
  return files;
}

DlnaServer::File DlnaServerDir::findFile(const QString &name)
{
  Q_ASSERT(!name.isEmpty());

  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  FileMap::ConstIterator i = files.find(name);
  if (i != files.end())
    return *i;

  return DlnaServer::File();
}

QString DlnaServerDir::findIcon(void) const
{
  foreach (const DlnaServer::File &file, listFiles())
  if (!file.iconUrl.isEmpty())
    return file.iconUrl;

  foreach (const FileServerDir *dir, listDirs())
  {
    const DlnaServerDir * const ddir = qobject_cast<const DlnaServerDir *>(dir);
    if (ddir)
    {
      const QString iconUrl = ddir->findIcon();
      if (!iconUrl.isEmpty())
        return iconUrl;
    }
  }

  return QString::null;
}


DlnaServerAlphaDir::DlnaServerAlphaDir(DlnaServer *parent)
    : DlnaServerDir(parent),
      subdirLimit(20),
      alphaMode(false)
{
}

DlnaServerAlphaDir::~DlnaServerAlphaDir()
{
}

void DlnaServerAlphaDir::setSubdirLimit(unsigned s)
{
  Q_ASSERT(alphaDirs.isEmpty());

  subdirLimit = s;
}

void DlnaServerAlphaDir::addDir(const QString &name, FileServerDir *dir)
{
  Q_ASSERT(!name.isEmpty());
  Q_ASSERT(qobject_cast<DlnaServerDir *>(dir));

  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  allDirs[name] = dir;

  DirMap::Iterator i = alphaDirs.find(name);
  if (i != alphaDirs.end())
    alphaDirs.erase(i);

  if (!alphaMode)
  {
    if (alphaDirs.count() < int(subdirLimit))
    { // Single layer
      DlnaServerDir::addDir(name, dir);
      alphaDirs[name] = dir;
      return;
    }
    else
    { // Move to alpha layer
      for (DirMap::ConstIterator i=alphaDirs.begin(); i!=alphaDirs.end(); i++)
        DlnaServerDir::removeDir(i.key());

      Q_ASSERT(DlnaServerDir::listDirs().isEmpty());

      for (DirMap::ConstIterator i=allDirs.begin(); i!=allDirs.end(); i++)
        addAlphaDir(i.key(), static_cast<DlnaServerDir *>(i.value()));

      alphaMode = true;
    }
  }

  addAlphaDir(name, static_cast<DlnaServerDir *>(dir));
}

void DlnaServerAlphaDir::removeDir(const QString &name)
{
  Q_ASSERT(!name.isEmpty());

  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  allDirs.remove(name);

  if (alphaMode)
  {
    removeAlphaDir(name);

    if (alphaDirs.count() <= int(subdirLimit))
    { // Move to single layer
      for (DirMap::ConstIterator i=alphaDirs.begin(); i!=alphaDirs.end(); i++)
        removeAlphaDir(i.key());

      Q_ASSERT(DlnaServerDir::listDirs().isEmpty());

      for (DirMap::ConstIterator i=allDirs.begin(); i!=allDirs.end(); i++)
        DlnaServerDir::addDir(i.key(), i.value());

      alphaMode = false;
    }
  }
  else
  {
    DlnaServerDir::removeDir(name);
    alphaDirs.remove(name);
  }
}

void DlnaServerAlphaDir::clear(void)
{
  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  foreach (FileServerDir *dir, alphaDirs)
    delete dir;

  alphaDirs.clear();
  allDirs.clear();

  if (alphaMode)
  {
    const DirMap dirs = DlnaServerDir::listDirs();
    for (DirMap::ConstIterator i=dirs.begin(); i!=dirs.end(); i++)
    {
      DlnaServerDir::removeDir(i.key());
      delete *i;
    }
  }

  DlnaServerDir::clear();
}

void DlnaServerAlphaDir::addAlphaDir(const QString &name, DlnaServerDir *dir)
{
  Q_ASSERT(!name.isEmpty());
  Q_ASSERT(dir);

  foreach (const QString &firstLetter, toFirstLetters(name))
  {
    FileServerDir *letterDir = findDir(firstLetter);
    if (letterDir == NULL)
      DlnaServerDir::addDir(firstLetter, letterDir = new DlnaServerDir(server()));

    letterDir->addDir(name, dir);
  }

  // Update latest items
  if (name[0].isLetterOrNumber() && dir->date.isValid())
  {
    DlnaServerDir *latestDir = static_cast<DlnaServerDir *>(findDir(tr("Latest")));
    if (latestDir == NULL)
    {
      latestDir = new DlnaServerDir(server());
      latestDir->sortOrder -= 1;
      DlnaServerDir::addDir(tr("Latest"), latestDir);
    }

    const DirMap * const subDirs = &(latestDir->listDirs());
    if (subDirs->count() < 10)
    {
      latestDir->addDir(name, dir);
    }
    else for (DirMap::ConstIterator i=subDirs->begin(); i!=subDirs->end(); i++)
    if (static_cast<DlnaServerDir *>(*i)->date < dir->date)
    {
      latestDir->removeDir(i.key());
      latestDir->addDir(name, dir);
      break;
    }
  }
}

void DlnaServerAlphaDir::removeAlphaDir(const QString &name)
{
  Q_ASSERT(!name.isEmpty());

  if (alphaDirs.remove(name) > 0)
  foreach (const QString &firstLetter, toFirstLetters(name))
  {
    FileServerDir * const letterDir = DlnaServerDir::findDir(firstLetter);
    if (letterDir)
    {
      letterDir->removeDir(name);
      if (letterDir->isEmpty())
      {
        DlnaServerDir::removeDir(firstLetter);
        delete letterDir;
      }
    }
  }

  FileServerDir *latestDir = findDir(tr("Latest"));
  if (latestDir)
  {
    latestDir->removeDir(name);
    if (latestDir->isEmpty())
    {
      DlnaServerDir::removeDir(tr("Latest"));
      delete latestDir;
    }
  }
}

QSet<QString> DlnaServerAlphaDir::toFirstLetters(const QString &name)
{
  Q_ASSERT(!name.isEmpty());

  QSet<QString> firstLetters;
  foreach (const QString &word, name.simplified().split(' '))
  if (!word.isEmpty() && word[0].isLetterOrNumber())
  {
    const QString firstLetter = SStringParser::toBasicLatin(word[0]).toUpper();
    if (!firstLetter.isEmpty())
      firstLetters += firstLetter;
  }

  return firstLetters;
}


} // End of namespace
