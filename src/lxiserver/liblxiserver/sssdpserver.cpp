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

#include "sssdpserver.h"
#include "shttpserver.h"

namespace LXiServer {

struct SSsdpServer::Private
{
  struct Service
  {
    inline Service(const QString &relativeUrl = QString::null, unsigned msgCount = 0)
      : relativeUrl(relativeUrl), msgCount(msgCount), publishCount(0)
    {
    }

    QString                     relativeUrl;
    unsigned                    msgCount;
    unsigned                    publishCount;
  };

  static const int              cacheTimeout = 1800; // Sec
  static const int              advertisementInterval = 5; // Sec

  QPointer<SHttpServer>         httpServer;
  qint32                        oldbootid;
  qint32                        bootid;
  qint32                        oldconfigid;
  qint32                        configid;
  QMultiMap<QString, Service>   published;
  QTimer                        updateTimer;
  QTimer                        rePublishTimer;
};

SSsdpServer::SSsdpServer(SHttpServer *httpServer)
  : SSsdpClient(httpServer->serverUdn()),
    p(new Private())
{
  p->httpServer = httpServer;
  p->oldbootid = 0;
  p->bootid = 0;
  p->oldconfigid = 0;
  p->configid = 0;

  p->updateTimer.setSingleShot(true);
  connect(&(p->updateTimer), SIGNAL(timeout()), SLOT(updateServices()));
  connect(&(p->rePublishTimer), SIGNAL(timeout()), SLOT(publishServices()));
}

SSsdpServer::~SSsdpServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SSsdpServer::initialize()
{
  QSettings settings;
  settings.beginGroup("SSDP");

  p->oldbootid = settings.value("BOOTID.UPNP.ORG").toUInt();
  p->bootid = qint32(QDateTime::currentDateTime().toTime_t() & 0x7FFFFFFF);
  p->oldconfigid = settings.value("CONFIGID.UPNP.ORG").toUInt();
  p->configid = p->bootid & 0x00FFFFFF;

  settings.setValue("BOOTID.UPNP.ORG", p->bootid);
  settings.setValue("CONFIGID.UPNP.ORG", p->configid);

  SSsdpClient::initialize();

  p->rePublishTimer.start(p->advertisementInterval * 1000);
}

void SSsdpServer::close(void)
{
  p->oldbootid = p->bootid;
  p->bootid = 0;
  p->oldconfigid = p->configid;
  p->configid = 0;

  p->rePublishTimer.stop();
  unpublishServices();

  SSsdpClient::close();
}

void SSsdpServer::reset(void)
{
  QSettings settings;
  settings.beginGroup("SSDP");

  p->oldbootid = p->bootid;
  p->bootid = qMax(qint32(QDateTime::currentDateTime().toTime_t() & 0x7FFFFFFF), p->bootid + 1);
  p->oldconfigid = p->configid;
  p->configid = ((p->configid + 1) & 0x00FFFFFF);

  p->rePublishTimer.stop();
  unpublishServices();

  settings.setValue("BOOTID.UPNP.ORG", p->bootid);
  settings.setValue("CONFIGID.UPNP.ORG", p->configid);

  p->updateTimer.start((p->advertisementInterval * 500) + (qrand() % 100));
  p->rePublishTimer.start((p->advertisementInterval * 1000) + (qrand() % 100));
}

bool SSsdpServer::bind(const QHostAddress &address)
{
  if (SSsdpClient::bind(address))
  {
    publishServices(address);

    return true;
  }

  return false;
}

void SSsdpServer::release(const QHostAddress &address)
{
  unpublishServices(address);

  SSsdpClient::release(address);
}

void SSsdpServer::publish(const QString &nt, const QString &relativeUrl, unsigned msgCount)
{
  p->published.insert(nt, Private::Service(relativeUrl, msgCount));
  p->updateTimer.start((p->advertisementInterval * 500) + (qrand() % 100));

  for (unsigned i=0; i<msgCount; i++)
  foreach (const QHostAddress &iface, interfaces())
    sendByeBye(iface, nt);
}

void SSsdpServer::parsePacket(const QHostAddress &iface, const SHttpServer::RequestHeader &header, const QHostAddress &sourceAddress, quint16 sourcePort)
{
  if ((header.method() == "M-SEARCH") && header.hasField("MX") &&
      !sourceAddress.isNull() && sourcePort)
  {
    const QString st = header.field("ST");
    const bool findAll = (st == "ssdp:all");

    for (QMultiMap<QString, Private::Service>::ConstIterator i = !findAll ? p->published.find(st) : p->published.begin();
         (i != p->published.end()) && (findAll || (i.key() == st));
         i++)
    {
      const quint16 port = p->httpServer->serverPort(iface);
      if (port > 0)
      {
        const QString url = "http://" + iface.toString() + ":" +
                            QString::number(port) + i->relativeUrl;

        for (unsigned j=0; j<(findAll ? i->msgCount : 1); j++)
          sendSearchResponse(iface, i.key(), url, sourceAddress, sourcePort);
      }
    }
  }
  else
    SSsdpClient::parsePacket(iface, header, sourceAddress, sourcePort);
}

void SSsdpServer::sendUpdate(const QHostAddress &iface, const QString &nt, const QString &url) const
{
  if (p->oldbootid != 0)
  {
    SHttpServer::RequestHeader request(NULL);
    request.setRequest("NOTIFY", "*", SHttpServer::httpVersion);
    request.setField("HOST", SSsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SSsdpServer::ssdpPort));
    request.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
    request.setField("LOCATION", url);
    request.setField("NT", nt);
    request.setField("NTS", "ssdp:update");
    if (p->oldbootid) request.setField("BOOTID.UPNP.ORG", QString::number(p->oldbootid));
    if (p->configid)  request.setField("CONFIGID.UPNP.ORG", QString::number(p->configid));
    if (p->bootid)    request.setField("NEXTBOOTID.UPNP.ORG", QString::number(p->bootid));

    sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
  }
}

void SSsdpServer::sendAlive(const QHostAddress &iface, const QString &nt, const QString &url) const
{
  SHttpServer::RequestHeader request(NULL);
  request.setRequest("NOTIFY", "*", SHttpServer::httpVersion);
  request.setField("SERVER", SHttpServer::osName() + ' ' + SUPnPBase::protocol() + ' ' + qApp->applicationName() + '/' + qApp->applicationVersion());
  request.setField("HOST", SSsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SSsdpServer::ssdpPort));
  request.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  request.setField("CACHE-CONTROL", "max-age=" + QString::number(p->cacheTimeout));
  request.setField("LOCATION", url);
  request.setField("NT", nt);
  request.setField("NTS", "ssdp:alive");
  if (p->bootid)   request.setField("BOOTID.UPNP.ORG", QString::number(p->bootid));
  if (p->configid) request.setField("CONFIGID.UPNP.ORG", QString::number(p->configid));

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SSsdpServer::sendByeBye(const QHostAddress &iface, const QString &nt) const
{
  SHttpServer::RequestHeader request(NULL);
  request.setRequest("NOTIFY", "*", SHttpServer::httpVersion);
  request.setField("HOST", SSsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SSsdpServer::ssdpPort));
  request.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  request.setField("NT", nt);
  request.setField("NTS", "ssdp:byebye");
  if (p->oldbootid)   request.setField("BOOTID.UPNP.ORG", QString::number(p->oldbootid));
  if (p->oldconfigid) request.setField("CONFIGID.UPNP.ORG", QString::number(p->oldconfigid));

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SSsdpServer::sendSearchResponse(const QHostAddress &iface, const QString &nt, const QString &url, const QHostAddress &toAddr, quint16 toPort) const
{
  SHttpServer::ResponseHeader response(NULL);
  response.setResponse(SHttpServer::Status_Ok, SHttpServer::httpVersion);
  response.setDate();
  response.setField("SERVER", SHttpServer::osName() + ' ' + SUPnPBase::protocol() + ' ' + qApp->applicationName() + '/' + qApp->applicationVersion());
  response.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  response.setField("CACHE-CONTROL", "max-age=" + QString::number(p->cacheTimeout));
  response.setField("EXT", QString::null);
  response.setField("LOCATION", url);
  response.setField("ST", nt);
  if (p->bootid)   response.setField("BOOTID.UPNP.ORG", QString::number(p->bootid));
  if (p->configid) response.setField("CONFIGID.UPNP.ORG", QString::number(p->configid));

  sendDatagram(iface, response, toAddr, toPort);
}

void SSsdpServer::publishServices(const QHostAddress &iface)
{
  for (QMultiMap<QString, Private::Service>::Iterator i = p->published.begin();
       i != p->published.end();
       i++)
  {
    for (unsigned j = 0; j < ((i->publishCount == 0) ? i->msgCount : 1); j++)
    {
      const quint16 port = p->httpServer->serverPort(iface);
      if (port > 0)
      {
        const QString url = "http://" + iface.toString() + ":" +
                            QString::number(port) + i->relativeUrl;

        sendAlive(iface, i.key(), url);
      }
    }

    i->publishCount++;
  }
}

void SSsdpServer::unpublishServices(const QHostAddress &iface)
{
  for (QMultiMap<QString, Private::Service>::ConstIterator i = p->published.begin();
       i != p->published.end();
       i++)
  {
    for (unsigned j=0; j<i->msgCount; j++)
      sendByeBye(iface, i.key());
  }
}

void SSsdpServer::updateServices(void)
{
  for (QMultiMap<QString, Private::Service>::ConstIterator i = p->published.begin();
       i != p->published.end();
       i++)
  {
    for (unsigned j=0; j<i->msgCount; j++)
    foreach (const QHostAddress &iface, interfaces())
    {
      const quint16 port = p->httpServer->serverPort(iface);
      if (port > 0)
      {
        const QString url = "http://" + iface.toString() + ":" +
                            QString::number(port) + i->relativeUrl;

        sendUpdate(iface, i.key(), url);
      }
    }
  }
}

void SSsdpServer::publishServices(void)
{
  foreach (const QHostAddress &iface, interfaces())
    publishServices(iface);
}

void SSsdpServer::unpublishServices(void)
{
  foreach (const QHostAddress &iface, interfaces())
    unpublishServices(iface);
}

} // End of namespace
