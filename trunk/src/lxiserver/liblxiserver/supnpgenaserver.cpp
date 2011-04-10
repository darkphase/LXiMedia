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

#include "supnpgenaserver.h"
#include "shttpclient.h"
#include "supnpbase.h"

namespace LXiServer {

const char  * const SUPnPGenaServer::eventNS       = "urn:schemas-upnp-org:event-1-0";
const QEvent::Type  SUPnPGenaServer::scheduleEventType = QEvent::Type(QEvent::registerEventType());

class SUPnPGenaServer::EventSession : public SHttpClient
{
public:
                                EventSession(SUPnPGenaServer *, const QStringList &eventUrls, int timeout = 0);
  virtual                       ~EventSession();

  void                          resubscribe(void);
  void                          unsubscribe(void);
  bool                          isActive(void) const;
  inline quint32                currentEventKey(void) const                     { return eventKey; }

  void                          sendEvent(void);

protected:
  virtual void                  handleResponse(const SHttpEngine::ResponseMessage &);

public:
  static const unsigned         maxInstances;
  static const int              minInterval;
  static const int              minTimeout;
  static const int              maxTimeout;
  static const int              defaultTimeout;

public:
  const QString                 sid;
  const int                     timeout;
  const QStringList             eventUrls;
  int                           queued;

private:
  SUPnPGenaServer             * const parent;
  quint32                       eventKey;
  QDateTime                     lastSubscribe;
  QList<SHttpServer::RequestMessage> pendingRequests;
};

struct SUPnPGenaServer::Data
{
  QString                       path;
  SHttpServer                 * httpServer;

  QUuid                         sidBase;
  QAtomicInt                    sidCounter;
  QMap<QString, EventSession *> eventSessions;
  int                           cleanTimer;

  QByteArray                    eventMessage;
  QTimer                        eventTimer;
};

SUPnPGenaServer::SUPnPGenaServer(const QString &basePath, QObject *parent)
  : QObject(parent),
    d(new Data())
{
  d->path = basePath + "event/";
  d->httpServer = NULL;
  d->sidBase = QUuid::createUuid();
  d->sidCounter = 0;
  d->cleanTimer = startTimer(EventSession::minTimeout * 1000);

  d->eventTimer.setSingleShot(true);
  connect(&d->eventTimer, SIGNAL(timeout()), SLOT(emitEvents()));
}

SUPnPGenaServer::~SUPnPGenaServer()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QString SUPnPGenaServer::path(void) const
{
  return d->path + "control";
}

void SUPnPGenaServer::initialize(SHttpServer *httpServer)
{
  d->httpServer = httpServer;

  httpServer->registerCallback(d->path, this);
}

void SUPnPGenaServer::close(void)
{
  for (QMap<QString, EventSession *>::Iterator i = d->eventSessions.begin();
       i != d->eventSessions.end();
       i = d->eventSessions.erase(i))
  {
    Q_ASSERT(((*i)->queued == 0) || ((*i)->currentEventKey() == 0));
    delete *i;
  }

  if (d->httpServer)
    d->httpServer->unregisterCallback(this);

  d->httpServer = NULL;
}

void SUPnPGenaServer::emitEvent(const QDomDocument &doc)
{
  const QByteArray content = QByteArray(SUPnPBase::xmlDeclaration) + '\n' + doc.toByteArray();

  d->eventMessage = content;

  QCoreApplication::postEvent(this, new QEvent(scheduleEventType));
}

void SUPnPGenaServer::customEvent(QEvent *e)
{
  if (e->type() == scheduleEventType)
    d->eventTimer.start(EventSession::minInterval * 1000);
  else
    QObject::customEvent(e);
}

void SUPnPGenaServer::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == d->cleanTimer)
  {
    // Cleanup old sessions.
    for (QMap<QString, EventSession *>::Iterator i=d->eventSessions.begin(); i!=d->eventSessions.end(); )
    if (!(*i)->isActive() && ((*i)->queued == 0))
    {
      delete *i;
      i = d->eventSessions.erase(i);
    }
    else
      i++;
  }
  else
    QObject::timerEvent(e);
}

SHttpServer::SocketOp SUPnPGenaServer::handleHttpRequest(const SHttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  if (request.path() == path())
  {
    if (request.method() == "SUBSCRIBE")
    {
      QMap<QString, EventSession *>::Iterator session = d->eventSessions.end();
      if (request.hasField("SID"))
      { // Update subscription
        session = d->eventSessions.find(request.field("SID").trimmed());
        if (session != d->eventSessions.end())
          (*session)->resubscribe();
      }
      else if (request.hasField("CALLBACK") && (request.field("NT") == "upnp:event") &&
               (d->eventSessions.count() < int(EventSession::maxInstances)))
      { // New subscription
        const QString callback = request.field("Callback");
        QStringList eventUrls;
        for (int b = callback.indexOf('<'), e = callback.indexOf('>', b);
             (b >= 0) && (e > b);
             b = callback.indexOf('<', e), e = callback.indexOf('>', b))
        {
          eventUrls += callback.mid(b + 1, e - (b + 1));
        }

        QString timeout = request.field("TIMEOUT").trimmed();
        timeout = timeout.startsWith("Second-", Qt::CaseInsensitive) ? timeout.mid(7) : QString::null;

        EventSession * const s = new EventSession(this, eventUrls, timeout.toInt());
        s->queued = 1; // Initial event queued, sent below.
        session = d->eventSessions.insert(s->sid, s);
      }
      else
        return SHttpServer::sendResponse(request, socket, SHttpServer::Status_BadRequest, this);

      if (session != d->eventSessions.end())
      {
        SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
        response.setField("SID", (*session)->sid);
        response.setContentLength(0);
        response.setField("TIMEOUT", "Second-" + QString::number((*session)->timeout));

        socket->write(response);

        if ((*session)->currentEventKey() == 0) // Need to send initial event?
        {
          socket->waitForBytesWritten(SUPnPBase::responseTimeout * 1000);

          (*session)->sendEvent();
        }

        return SHttpServer::SocketOp_Close;
      }
    }
    else if (request.method() == "UNSUBSCRIBE")
    {
      if (!request.hasField("CALLBACK") && !request.hasField("NT"))
      {
        QMap<QString, EventSession *>::Iterator session = d->eventSessions.find(request.field("SID").trimmed());
        if (session != d->eventSessions.end())
        {
          (*session)->unsubscribe();
          return SHttpServer::sendResponse(request, socket, SHttpServer::Status_Ok, this);
        }
      }
      else
        return SHttpServer::sendResponse(request, socket, SHttpServer::Status_BadRequest, this);
    }

    return SHttpServer::sendResponse(request, socket, SHttpServer::Status_PreconditionFailed, this);
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

void SUPnPGenaServer::emitEvents(void)
{
  foreach (EventSession *session, d->eventSessions)
  if (session->queued == 0)
  {
    session->queued = 1;
    session->sendEvent();
  }
}

QString SUPnPGenaServer::makeSid(void)
{
  QString sid = "uuid:" + QUuid(
      d->sidBase.data1 + uint(d->sidCounter.fetchAndAddRelaxed(1)),
      d->sidBase.data2, d->sidBase.data3,
      d->sidBase.data4[0], d->sidBase.data4[1], d->sidBase.data4[2], d->sidBase.data4[3],
      d->sidBase.data4[4], d->sidBase.data4[5], d->sidBase.data4[6], d->sidBase.data4[7]).toString();

  return sid.replace("{", "").replace("}", "");
}


const unsigned  SUPnPGenaServer::EventSession::maxInstances = 256;
const int       SUPnPGenaServer::EventSession::minInterval = 2;
const int       SUPnPGenaServer::EventSession::minTimeout = 30;
const int       SUPnPGenaServer::EventSession::maxTimeout = 1800;
const int       SUPnPGenaServer::EventSession::defaultTimeout = 180;

SUPnPGenaServer::EventSession::EventSession(SUPnPGenaServer *parent, const QStringList &eventUrls, int timeout)
  : SHttpClient(parent),
    sid(parent->makeSid()),
    timeout(qBound(minTimeout, (timeout > 0) ? timeout : defaultTimeout, maxTimeout)),
    eventUrls(eventUrls),
    queued(0),
    parent(parent),
    eventKey(0),
    lastSubscribe(QDateTime::currentDateTime())
{
}

SUPnPGenaServer::EventSession::~EventSession()
{
}

void SUPnPGenaServer::EventSession::resubscribe(void)
{
  lastSubscribe = QDateTime::currentDateTime();
}

void SUPnPGenaServer::EventSession::unsubscribe(void)
{
  lastSubscribe = QDateTime();
}

bool SUPnPGenaServer::EventSession::isActive(void) const
{
  if (lastSubscribe.isValid())
    return lastSubscribe.secsTo(QDateTime::currentDateTime()) < (timeout + 30);

  return false;
}

void SUPnPGenaServer::EventSession::sendEvent(void)
{
  if (!lastSubscribe.isValid())
    return;

  // Build the message
  const QByteArray content = parent->d->eventMessage;
  SHttpServer::RequestMessage request(NULL);
  request.setContentType(SUPnPBase::xmlContentType);
  request.setContentLength(content.length());
  request.setField("NT", "upnp:event");
  request.setField("NTS", "upnp:propchange");
  request.setField("SID", sid);
  request.setField("SEQ", QString::number(eventKey));

  request.setContent(content);

  // Make sure the eventKey does not overflow to 0.
  if (++eventKey == 0) eventKey = 1;

  // Try the eventUrls until one succeeds
  foreach (const QUrl url, eventUrls)
  {
    request.setRequest("NOTIFY", url.path().toUtf8());
    request.setHost(url.host() + ":" + QString::number(url.port(80)));

    pendingRequests.append(request);
  }

  sendRequest(pendingRequests.takeFirst());
}

void SUPnPGenaServer::EventSession::handleResponse(const SHttpEngine::ResponseMessage &response)
{
  if ((response.status() != SHttpEngine::Status_Ok) && !pendingRequests.isEmpty())
    sendRequest(pendingRequests.takeFirst());
  else
    queued = 0;
}

} // End of namespace
