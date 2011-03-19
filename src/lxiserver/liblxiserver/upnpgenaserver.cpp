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

#include "upnpgenaserver.h"
#include "upnpbase.h"

namespace LXiServer {

const char  * const UPnPGenaServer::eventNS       = "urn:schemas-upnp-org:event-1-0";
const QEvent::Type  UPnPGenaServer::scheduleEventType = QEvent::Type(QEvent::registerEventType());

class UPnPGenaServer::EventSession : public QRunnable
{
public:
                                EventSession(UPnPGenaServer *, const QStringList &eventUrls, int timeout = 0);
  virtual                       ~EventSession();

  void                          resubscribe(void);
  void                          unsubscribe(void);
  bool                          isActive(void) const;
  inline quint32                currentEventKey(void) const                     { return eventKey; }

protected:
  virtual void                  run(void);

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
  QAtomicInt                    queued;

private:
  UPnPGenaServer              * const parent;
  quint32                       eventKey;
  QDateTime                     lastSubscribe;
};

struct UPnPGenaServer::Data
{
  inline                        Data(void) : lock(QReadWriteLock::Recursive) { }

  QReadWriteLock                lock;
  QString                       path;
  HttpServer                  * httpServer;

  QUuid                         sidBase;
  QAtomicInt                    sidCounter;
  QMap<QString, EventSession *> eventSessions;
  int                           cleanTimer;

  QByteArray                    eventMessage;
  QTimer                        eventTimer;
};

UPnPGenaServer::UPnPGenaServer(const QString &basePath, QObject *parent)
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

UPnPGenaServer::~UPnPGenaServer()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QString UPnPGenaServer::path(void) const
{
  return d->path + "control";
}

void UPnPGenaServer::initialize(HttpServer *httpServer)
{
  QWriteLocker l(&d->lock);

  d->httpServer = httpServer;

  httpServer->registerCallback(d->path, this);
}

void UPnPGenaServer::close(void)
{
  if (d->httpServer)
    d->httpServer->threadPool()->waitForDone();

  QWriteLocker l(&d->lock);

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

void UPnPGenaServer::emitEvent(const QDomDocument &doc)
{
  const QByteArray content = QByteArray(UPnPBase::xmlDeclaration) + '\n' + doc.toByteArray();

  QWriteLocker l(&d->lock);

  d->eventMessage = content;

  QCoreApplication::postEvent(this, new QEvent(scheduleEventType));
}

void UPnPGenaServer::customEvent(QEvent *e)
{
  if (e->type() == scheduleEventType)
    d->eventTimer.start(EventSession::minInterval * 1000);
  else
    QObject::customEvent(e);
}

void UPnPGenaServer::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == d->cleanTimer)
  {
    if (d->lock.tryLockForWrite(0))
    {
      // Cleanup old sessions.
      for (QMap<QString, EventSession *>::Iterator i=d->eventSessions.begin(); i!=d->eventSessions.end(); )
      if (!(*i)->isActive() && (*i)->queued.testAndSetRelaxed(0, -1))
      {
        delete *i;
        i = d->eventSessions.erase(i);
      }
      else
        i++;

      d->lock.unlock();
    }
  }
  else
    QObject::timerEvent(e);
}

HttpServer::SocketOp UPnPGenaServer::handleHttpRequest(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  if (request.path() == path())
  {
    if (request.method() == "SUBSCRIBE")
    {
      QWriteLocker l(&d->lock);

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
        return HttpServer::sendResponse(request, socket, HttpServer::Status_BadRequest, this);

      if (session != d->eventSessions.end())
      {
        HttpServer::ResponseHeader response(request, HttpServer::Status_Ok);
        response.setField("SID", (*session)->sid);
        response.setContentLength(0);
        response.setField("TIMEOUT", "Second-" + QString::number((*session)->timeout));

        socket->write(response);

        if ((*session)->currentEventKey() == 0) // Need to send initial event?
        {
          EventSession * const s = *session;

          l.unlock();

          socket->waitForBytesWritten(UPnPBase::responseTimeout * 1000);
          d->httpServer->threadPool()->start(s, 1);
        }

        return HttpServer::SocketOp_Close;
      }
    }
    else if (request.method() == "UNSUBSCRIBE")
    {
      QWriteLocker l(&d->lock);

      if (!request.hasField("CALLBACK") && !request.hasField("NT"))
      {
        QMap<QString, EventSession *>::Iterator session = d->eventSessions.find(request.field("SID").trimmed());
        if (session != d->eventSessions.end())
        {
          (*session)->unsubscribe();
          return HttpServer::sendResponse(request, socket, HttpServer::Status_Ok, this);
        }
      }
      else
        return HttpServer::sendResponse(request, socket, HttpServer::Status_BadRequest, this);
    }

    return HttpServer::sendResponse(request, socket, HttpServer::Status_PreconditionFailed, this);
  }

  return HttpServer::sendResponse(request, socket, HttpServer::Status_NotFound, this);
}

void UPnPGenaServer::emitEvents(void)
{
  QWriteLocker l(&d->lock);

  foreach (EventSession *session, d->eventSessions)
  if (session->queued.testAndSetRelaxed(0, 1))
    d->httpServer->threadPool()->start(session, -1); // Lower priority than handling incoming requests
}

QString UPnPGenaServer::makeSid(void)
{
  QString sid = "uuid:" + QUuid(
      d->sidBase.data1 + uint(d->sidCounter.fetchAndAddRelaxed(1)),
      d->sidBase.data2, d->sidBase.data3,
      d->sidBase.data4[0], d->sidBase.data4[1], d->sidBase.data4[2], d->sidBase.data4[3],
      d->sidBase.data4[4], d->sidBase.data4[5], d->sidBase.data4[6], d->sidBase.data4[7]).toString();

  return sid.replace("{", "").replace("}", "");
}


const unsigned  UPnPGenaServer::EventSession::maxInstances = 256;
const int       UPnPGenaServer::EventSession::minInterval = 2;
const int       UPnPGenaServer::EventSession::minTimeout = 30;
const int       UPnPGenaServer::EventSession::maxTimeout = 1800;
const int       UPnPGenaServer::EventSession::defaultTimeout = 180;

UPnPGenaServer::EventSession::EventSession(UPnPGenaServer *parent, const QStringList &eventUrls, int timeout)
    : sid(parent->makeSid()),
      timeout(qBound(minTimeout, (timeout > 0) ? timeout : defaultTimeout, maxTimeout)),
      eventUrls(eventUrls),
      queued(0),
      parent(parent),
      eventKey(0),
      lastSubscribe(QDateTime::currentDateTime())
{
  QRunnable::setAutoDelete(false);
}

UPnPGenaServer::EventSession::~EventSession()
{
}

void UPnPGenaServer::EventSession::resubscribe(void)
{
  QWriteLocker l(&parent->d->lock);

  lastSubscribe = QDateTime::currentDateTime();
}

void UPnPGenaServer::EventSession::unsubscribe(void)
{
  QWriteLocker l(&parent->d->lock);

  lastSubscribe = QDateTime();
}

bool UPnPGenaServer::EventSession::isActive(void) const
{
  QWriteLocker l(&parent->d->lock);

  if (lastSubscribe.isValid())
    return lastSubscribe.secsTo(QDateTime::currentDateTime()) < (timeout + 30);

  return false;
}

void UPnPGenaServer::EventSession::run(void)
{
  QWriteLocker l(&parent->d->lock);

  if (!lastSubscribe.isValid())
    return;

  // Build the message
  const QByteArray content = parent->d->eventMessage;
  HttpServer::RequestHeader header(NULL);
  header.setContentType(UPnPBase::xmlContentType);
  header.setContentLength(content.length());
  header.setField("NT", "upnp:event");
  header.setField("NTS", "upnp:propchange");
  header.setField("SID", sid);
  header.setField("SEQ", QString::number(eventKey));

  // Make sure the eventKey does not overflow to 0.
  if (++eventKey == 0) eventKey = 1;

  l.unlock();

  static const int maxRequestTime = UPnPBase::responseTimeout * 1000;
  QTime timer; timer.start();

  // Try the eventUrls until one succeeds
  foreach (const QUrl url, eventUrls)
  {
    header.setRequest("NOTIFY", url.path().toUtf8());
    header.setHost(url.host() + ":" + QString::number(url.port(80)));

    QTcpSocket socket;
    socket.connectToHost(url.host(), url.port(80));
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
            socket.close();

            const HttpServer::ResponseHeader responseHeader(response, parent->d->httpServer);
            if (responseHeader.status() == HttpServer::Status_Ok)
            {
              queued.deref();
              return; // Event delivered.
            }
            else
            {
              qDebug() << "UPnPGenaServer::EventSession: got error response:" << response;

              break; // Failed, try next URL.
            }
          }
        }
      }
    }

    if (timer.elapsed() > maxRequestTime)
      break;
  }

  queued.deref();
}

} // End of namespace
