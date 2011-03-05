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

#include "upnpbase.h"
#include "upnpmediaserver.h"

namespace LXiServer {

const char  * const UPnPBase::didlNS        = "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/";
const char  * const UPnPBase::dublinCoreNS  = "http://purl.org/dc/elements/1.1/";
const char  * const UPnPBase::eventNS       = "urn:schemas-upnp-org:event-1-0";
const char  * const UPnPBase::metadataNS    = "urn:schemas-upnp-org:metadata-1-0";
const char  * const UPnPBase::soapNS        = "http://schemas.xmlsoap.org/soap/envelope/";
const QEvent::Type  UPnPBase::scheduleEventType = QEvent::Type(QEvent::registerEventType());

class UPnPBase::EventSession : public QRunnable
{
public:
                                EventSession(UPnPBase *, const QString &eventUrl);
  virtual                       ~EventSession();

  void                          resubscribe(void);

  inline qint32                 sessionId(void) const                           { return sid; }
  bool                          isActive(void) const;
  inline bool                   isDirty(void) const                             { return dirty && !scheduled; }
  inline void                   resetDirty(void)                                { dirty = false; scheduled = true; }
  inline void                   setDirty(void)                                  { dirty = true; }

  static QThreadPool          & threadPool(void);

protected:
  virtual void                  run(void);

public:
  static const unsigned         maxInstances;
  static const int              interval, timeout;

private:
  static QAtomicInt             sidCounter;
  const int                     sid;

  UPnPBase              * const parent;
  const QUrl                    eventUrl;
  QDateTime                     lastSubscribe;
  volatile bool                 running;
  volatile bool                 dirty;
  volatile bool                 scheduled;
  QTime                         lastUpdate;
};

struct UPnPBase::Data
{
  inline                        Data(void) : lock(QReadWriteLock::Recursive) { }

  QReadWriteLock                lock;
  QString                       basePath;
  QString                       serviceType;
  QString                       serviceId;
  HttpServer                  * httpServer;
  UPnPMediaServer             * mediaServer;

  int                           cleanTimer;
  QMap<int, EventSession *>     eventSessions;
};

UPnPBase::UPnPBase(const QString &basePath, const QString &serviceType, const QString &serviceId, QObject *parent)
  : QObject(parent),
    d(new Data())
{
  d->basePath = basePath;
  d->serviceType = serviceType;
  d->serviceId = serviceId;
  d->httpServer = NULL;
  d->mediaServer = NULL;
  d->cleanTimer = startTimer((EventSession::timeout * 1000) / 2);

  // The threads that emit events simultaneously is managed by this threadpool.
#ifdef QT_NO_DEBUG
  EventSession::threadPool().setMaxThreadCount(8);
#else // Limited to one thread in debug mode.
  EventSession::threadPool().setMaxThreadCount(1);
#endif
}

UPnPBase::~UPnPBase()
{
  foreach (EventSession *session, d->eventSessions)
    delete session;

  if (d->httpServer)
    d->httpServer->unregisterCallback(this);

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void UPnPBase::initialize(HttpServer *httpServer, UPnPMediaServer *mediaServer)
{
  QWriteLocker l(&d->lock);

  d->httpServer = httpServer;
  d->mediaServer = mediaServer;

  httpServer->registerCallback(d->basePath, this);

  UPnPMediaServer::Service service;
  service.serviceType = d->serviceType;
  service.serviceId = d->serviceId;
  service.descriptionUrl = d->basePath + "description.xml";
  service.controlURL = d->basePath + "control";
  service.eventSubURL = d->basePath + "eventsub";

  mediaServer->registerService(service);
}

void UPnPBase::close(void)
{
  QWriteLocker l(&d->lock);

  if (d->httpServer)
    d->httpServer->unregisterCallback(this);

  d->httpServer = NULL;
  d->mediaServer = NULL;
}

void UPnPBase::emitEvent(void)
{
  QReadLocker l(lock());

  foreach (EventSession *session, d->eventSessions)
    session->setDirty();

  QCoreApplication::postEvent(this, new QEvent(scheduleEventType));
}

HttpServer::SocketOp UPnPBase::handleHttpRequest(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  if ((request.path() == d->basePath + "control") && (request.method() == "POST"))
    return handleControl(request, socket);
  else if (request.path() == d->basePath + "eventsub")
    return handleEventSub(request, socket);
  else if (request.path() == d->basePath + "description.xml")
    return handleDescription(request, socket);

  qWarning() << "UPnPBase: Could not handle request:" << request.method() << request.path();
  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}

HttpServer::SocketOp UPnPBase::handleControl(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  QTime timer;
  timer.start();

  QByteArray data = socket->readAll();
  while ((data.count() < int(request.contentLength())) && (qAbs(timer.elapsed()) < 5000))
  if (socket->waitForReadyRead())
    data += socket->readAll();

  if (data.count() > 0)
  {
    QDomDocument doc;
    const QDomElement body = parseSoapMessage(doc, data);
    if (!body.isNull())
    {
      QDomDocument responseDoc;
      QDomElement responseBody = makeSoapMessage(responseDoc);

      handleSoapMessage(body, responseDoc, responseBody, request, socket->peerAddress());

      const QByteArray content = serializeSoapMessage(responseDoc);
      HttpServer::ResponseHeader response(HttpServer::Status_Ok);
      response.setContentType("text/xml;charset=utf-8");
      response.setContentLength(content.length());
      response.setField("Cache-Control", "no-cache");
      response.setField("Accept-Ranges", "bytes");
      response.setField("Connection", "close");
      response.setField("contentFeatures.dlna.org", "");
      response.setField("Server", d->mediaServer->serverId());
      socket->write(response);
      socket->write(content);

      return HttpServer::SocketOp_Close;
    }
  }

  qWarning() << "UPnPBase::handleControl(): Could not handle request:" << request.method() << request.path();
  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}

HttpServer::SocketOp UPnPBase::handleEventSub(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  if (request.method() == "SUBSCRIBE")
  {
    if (request.hasField("SID"))
    { // Update subscription
      const int sid = request.field("SID").toInt(NULL, 16);

      QWriteLocker l(&d->lock);

      QMap<int, EventSession *>::Iterator i = d->eventSessions.find(sid);
      if (i != d->eventSessions.end())
      {
        (*i)->resubscribe();
        l.unlock();

        HttpServer::ResponseHeader response(HttpServer::Status_Ok);
        response.setField("SID", QString::number(sid, 16));
        response.setField("Timeout", "Second-" + QString::number(EventSession::timeout));
        response.setField("Server", d->mediaServer->serverId());
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
      QWriteLocker l(&d->lock);

      // Prevent creating too much sessions.
      if (d->eventSessions.count() < int(EventSession::maxInstances))
      {
        EventSession * const session =
            new EventSession(this, request.field("Callback").replace('<', "").replace('>', ""));

        d->eventSessions[session->sessionId()] = session;

        l.unlock();

        HttpServer::ResponseHeader response(HttpServer::Status_Ok);
        response.setField("SID", QString::number(session->sessionId(), 16));
        response.setField("Timeout", "Second-" + QString::number(EventSession::timeout));
        response.setField("Server", d->mediaServer->serverId());
        socket->write(response);
        if (socket->waitForBytesWritten(5000))
        {
          session->resetDirty();
          EventSession::threadPool().start(session);
        }

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

    QWriteLocker l(&d->lock);

    QMap<int, EventSession *>::Iterator i = d->eventSessions.find(sid);
    if (i != d->eventSessions.end())
    {
      delete *i;
      d->eventSessions.erase(i);

      l.unlock();

      HttpServer::ResponseHeader response(HttpServer::Status_Ok);
      response.setContentType("text/xml;charset=utf-8");
      response.setField("Server", d->mediaServer->serverId());
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

  qWarning() << "UPnPBase::handleEventSub(): Could not handle request:" << request.method() << request.path();
  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}

HttpServer::SocketOp UPnPBase::handleDescription(const HttpServer::RequestHeader &, QAbstractSocket *socket)
{
  QDomDocument doc;
  QDomElement scpdElm = doc.createElement("scpd");
  scpdElm.setAttribute("xmlns", "urn:schemas-upnp-org:service-1-0");
  addSpecVersion(doc, scpdElm);

  buildDescription(doc, scpdElm);

  doc.appendChild(scpdElm);

  const QByteArray content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + doc.toByteArray();
  HttpServer::ResponseHeader response(HttpServer::Status_Ok);
  response.setContentType("text/xml;charset=utf-8");
  response.setContentLength(content.length());
  response.setField("Cache-Control", "no-cache");
  response.setField("Accept-Ranges", "bytes");
  response.setField("Connection", "close");
  response.setField("contentFeatures.dlna.org", "");
  response.setField("Server", mediaServer()->serverId());
  socket->write(response);
  socket->write(content);

  return HttpServer::SocketOp_Close;
}

void UPnPBase::customEvent(QEvent *e)
{
  if (e->type() == scheduleEventType)
  {
    QWriteLocker l(&d->lock);

    foreach (EventSession *session, d->eventSessions)
    if (session->isActive() && session->isDirty())
    {
      session->resetDirty();
      EventSession::threadPool().start(session);
    }
  }
  else
    QObject::customEvent(e);
}

void UPnPBase::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == d->cleanTimer)
  {
    if (d->lock.tryLockForWrite(0))
    {
      // Cleanup old sessions.
      foreach (EventSession *session, d->eventSessions)
      if (!session->isActive())
      {
        d->eventSessions.remove(session->sessionId());
        delete session;
      }

      d->lock.unlock();
    }
  }
  else
    QObject::timerEvent(e);
}

QReadWriteLock * UPnPBase::lock(void) const
{
  return &d->lock;
}

HttpServer * UPnPBase::httpServer(void) const
{
  return d->httpServer;
}

UPnPMediaServer * UPnPBase::mediaServer(void) const
{
  return d->mediaServer;
}

void UPnPBase::addTextElm(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &value)
{
  QDomElement subElm = doc.createElement(name);
  subElm.appendChild(doc.createTextNode(value));
  elm.appendChild(subElm);
}

void UPnPBase::addSpecVersion(QDomDocument &doc, QDomElement &elm, int major, int minor)
{
  QDomElement specVersionElm = doc.createElement("specVersion");
  addTextElm(doc, specVersionElm, "major", QString::number(major));
  addTextElm(doc, specVersionElm, "minor", QString::number(minor));
  elm.appendChild(specVersionElm);
}

void UPnPBase::addActionArgument(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &direction, const QString &relatedStateVariable)
{
  QDomElement argumentElm = doc.createElement("argument");
  addTextElm(doc, argumentElm, "name", name);
  addTextElm(doc, argumentElm, "direction", direction);
  addTextElm(doc, argumentElm, "relatedStateVariable", relatedStateVariable);
  elm.appendChild(argumentElm);
}

void UPnPBase::addStateVariable(QDomDocument &doc, QDomElement &elm, bool sendEvents, const QString &name, const QString &dataType, const QStringList &allowedValues)
{
  QDomElement stateVariableElm = doc.createElement("stateVariable");
  stateVariableElm.setAttribute("sendEvents", sendEvents ? "yes" : "no");
  addTextElm(doc, stateVariableElm, "name", name);
  addTextElm(doc, stateVariableElm, "dataType", dataType);

  if (!allowedValues.isEmpty())
  {
    QDomElement allowedValueListElm = doc.createElement("allowedValueList");

    foreach (const QString &allowedValue, allowedValues)
      addTextElm(doc, allowedValueListElm, "allowedValue", allowedValue);

    stateVariableElm.appendChild(allowedValueListElm);
  }

  elm.appendChild(stateVariableElm);
}

QDomElement UPnPBase::makeSoapMessage(QDomDocument &doc)
{
  QDomElement root = doc.createElementNS(soapNS, "s:Envelope");
  root.setAttributeNS(soapNS, "s:encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
  doc.appendChild(root);
  QDomElement body = doc.createElementNS(soapNS, "s:Body");
  root.appendChild(body);

  return body;
}

QByteArray UPnPBase::serializeSoapMessage(const QDomDocument &doc)
{
  return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + doc.toByteArray(-1);
}

QDomElement UPnPBase::parseSoapMessage(QDomDocument &doc, const QByteArray &data)
{
  doc = QDomDocument("Envelope");
  if (doc.setContent(data))
    return doc.documentElement().firstChildElement("s:Body");

  return QDomElement();
}

const unsigned  UPnPBase::EventSession::maxInstances = 256;
const int       UPnPBase::EventSession::interval = 2;
const int       UPnPBase::EventSession::timeout = 180;
QAtomicInt      UPnPBase::EventSession::sidCounter = 1;

UPnPBase::EventSession::EventSession(UPnPBase *parent, const QString &eventUrl)
    : sid(sidCounter.fetchAndAddOrdered(1)),
      parent(parent),
      eventUrl(eventUrl),
      lastSubscribe(QDateTime::currentDateTime()),
      running(true),
      dirty(true)
{
  QRunnable::setAutoDelete(false);
}

UPnPBase::EventSession::~EventSession()
{
  running = false;
}

void UPnPBase::EventSession::resubscribe(void)
{
  QWriteLocker l(&parent->d->lock);

  lastSubscribe = QDateTime::currentDateTime();
}

bool UPnPBase::EventSession::isActive(void) const
{
  if (running)
  {
    QWriteLocker l(&parent->d->lock);

    if (lastSubscribe.secsTo(QDateTime::currentDateTime()) < (timeout + 30))
      return true;
  }

  return false;
}

QThreadPool & UPnPBase::EventSession::threadPool(void)
{
  static QThreadPool t;

  return t;
}

void UPnPBase::EventSession::run(void)
{
  // Build the message
  QDomDocument doc;
  QDomElement propertySet = doc.createElementNS(eventNS, "e:propertyset");
  parent->addEventProperties(doc, propertySet);
  doc.appendChild(propertySet);

  const QByteArray content = doc.toString(-1).toUtf8();
  HttpServer::RequestHeader header;
  header.setRequest("NOTIFY", eventUrl.path().toUtf8());
  header.setContentType("text/xml;charset=utf-8");
  header.setContentLength(content.length());
  header.setField("Host", eventUrl.host() + ":" + QString::number(eventUrl.port(80)));
  header.setField("NT", "upnp:event");
  header.setField("NTS", "upnp:propchange");
  header.setField("SID", QString::number(sid, 16));
  header.setField("SEQ", "0");

  // Now send the message
  static const int maxRequestTime = 5000;
  QTime timer; timer.start();

  QTcpSocket socket;
  socket.connectToHost(eventUrl.host(), eventUrl.port(80));
  if (socket.waitForConnected(qMax(maxRequestTime - timer.elapsed(), 10)))
  {
    socket.write(header + content);
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
            qWarning() << "UPnPBase::EventSession: got error response:" << response;
            running = false;
          }

          socket.close();
          break;
        }
      }
    }
  }

  scheduled = false;
}

} // End of namespace
