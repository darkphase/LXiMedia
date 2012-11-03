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

#if defined(Q_OS_UNIX)
#include <arpa/inet.h>
#include <sys/socket.h>
#elif defined(Q_OS_WIN)
#include <ws2tcpip.h>
#endif
#include "shttpserver.h"

namespace LXiServer {

struct SSsdpServer::Private
{
  struct Service
  {
    inline Service(const QString &relativeUrl = QString::null, unsigned msgCount = 0)
      : relativeUrl(relativeUrl), msgCount(msgCount)
    {
    }

    QString                     relativeUrl;
    unsigned                    msgCount;
  };

  QPointer<SHttpServer>         httpServer;
  qint32                        oldbootid;
  qint32                        bootid;
  qint32                        configid;
  QMultiMap<QString, Service>   published;
  QTimer                        updateTimer;
  QTimer                        publishTimer;
  QTimer                        rePublishTimer;
};

SSsdpServer::SSsdpServer(SHttpServer *httpServer)
  : SSsdpClient(httpServer->serverUdn()),
    p(new Private())
{
  p->httpServer = httpServer;
  p->oldbootid = 0;
  p->bootid = 0;
  p->configid = 0;

  p->updateTimer.setSingleShot(true);
  p->publishTimer.setSingleShot(true);
  connect(&(p->updateTimer), SIGNAL(timeout()), SLOT(updateServices()));
  connect(&(p->publishTimer), SIGNAL(timeout()), SLOT(publishServices()));
  connect(&(p->rePublishTimer), SIGNAL(timeout()), SLOT(publishServices()));
}

SSsdpServer::~SSsdpServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SSsdpServer::initialize(const QList<QHostAddress> &interfaces)
{
  QSettings settings;
  settings.beginGroup("SSDP");

  p->oldbootid = settings.value("BOOTID.UPNP.ORG").toUInt();
  p->bootid = qint32(QDateTime::currentDateTime().toTime_t() & 0x7FFFFFFF);
  p->configid = p->bootid & 0x00FFFFFF;

  settings.setValue("BOOTID.UPNP.ORG", p->bootid);

  SSsdpClient::initialize(interfaces);

  p->rePublishTimer.start(((cacheTimeout / 2) - 300) * 1000);
}

void SSsdpServer::close(void)
{
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
  p->configid = ((p->configid + 1) & 0x00FFFFFF);

  p->rePublishTimer.stop();
  unpublishServices();

  settings.setValue("BOOTID.UPNP.ORG", p->bootid);

  p->updateTimer.start(3000 + (qrand() % 100));
  p->publishTimer.start(3500 + (qrand() % 100));
  p->rePublishTimer.start(((cacheTimeout / 2) - 300) * 1000);
}

void SSsdpServer::publish(const QString &nt, const QString &relativeUrl, unsigned msgCount)
{
  p->published.insert(nt, Private::Service(relativeUrl, msgCount));
  p->updateTimer.start(3000 + (qrand() % 100));
  p->publishTimer.start(3500 + (qrand() % 100));
}

void SSsdpServer::parsePacket(SsdpClientInterface *iface, const SHttpServer::RequestHeader &header, const QHostAddress &sourceAddress, quint16 sourcePort)
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
      const quint16 port = p->httpServer->serverPort(iface->interfaceAddr);
      if (port > 0)
      {
        const QString url = "http://" + iface->interfaceAddr.toString() + ":" +
                            QString::number(port) + i->relativeUrl;

        for (unsigned j=0; j<(findAll ? i->msgCount : 1); j++)
          sendSearchResponse(iface, i.key(), url, sourceAddress, sourcePort);
      }
    }
  }
  else
    SSsdpClient::parsePacket(iface, header, sourceAddress, sourcePort);
}

void SSsdpServer::sendUpdate(SsdpClientInterface *iface, const QString &nt, const QString &url) const
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
    request.setField("BOOTID.UPNP.ORG", QString::number(p->oldbootid));
    request.setField("CONFIGID.UPNP.ORG", QString::number(p->configid));
    request.setField("NEXTBOOTID.UPNP.ORG", QString::number(p->bootid));

    sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
  }
}

void SSsdpServer::sendAlive(SsdpClientInterface *iface, const QString &nt, const QString &url) const
{
  SHttpServer::RequestHeader request(NULL);
  request.setRequest("NOTIFY", "*", SHttpServer::httpVersion);
  request.setField("SERVER", SHttpServer::osName() + " UPnP/1.1 " + qApp->applicationName() + '/' + qApp->applicationVersion());
  request.setField("HOST", SSsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SSsdpServer::ssdpPort));
  request.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  request.setField("CACHE-CONTROL", "max-age=" + QString::number(SSsdpServer::cacheTimeout));
  request.setField("LOCATION", url);
  request.setField("NT", nt);
  request.setField("NTS", "ssdp:alive");
  request.setField("BOOTID.UPNP.ORG", QString::number(p->bootid));
  request.setField("CONFIGID.UPNP.ORG", QString::number(p->configid));

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SSsdpServer::sendByeBye(SsdpClientInterface *iface, const QString &nt) const
{
  SHttpServer::RequestHeader request(NULL);
  request.setRequest("NOTIFY", "*", SHttpServer::httpVersion);
  request.setField("HOST", SSsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SSsdpServer::ssdpPort));
  request.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  request.setField("NT", nt);
  request.setField("NTS", "ssdp:byebye");
  request.setField("BOOTID.UPNP.ORG", QString::number(p->bootid));
  request.setField("CONFIGID.UPNP.ORG", QString::number(p->configid));

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SSsdpServer::sendSearchResponse(SsdpClientInterface *iface, const QString &nt, const QString &url, const QHostAddress &toAddr, quint16 toPort) const
{
  SHttpServer::ResponseHeader response(NULL);
  response.setResponse(SHttpServer::Status_Ok, SHttpServer::httpVersion);
  response.setDate();
  response.setField("SERVER", SHttpServer::osName() + " UPnP/1.1 " + qApp->applicationName() + '/' + qApp->applicationVersion());
  response.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  response.setField("CACHE-CONTROL", "max-age=" + QString::number(SSsdpServer::cacheTimeout));
  response.setField("EXT", QString::null);
  response.setField("LOCATION", url);
  response.setField("ST", nt);
  response.setField("BOOTID.UPNP.ORG", QString::number(p->bootid));
  response.setField("CONFIGID.UPNP.ORG", QString::number(p->configid));

  sendDatagram(iface, response, toAddr, toPort);
}

void SSsdpServer::updateServices(void)
{
  for (QMultiMap<QString, Private::Service>::ConstIterator i = p->published.begin();
       i != p->published.end();
       i++)
  {
    for (unsigned j=0; j<i->msgCount; j++)
    foreach (SsdpClientInterface *iface, interfaces())
    {
      const quint16 port = p->httpServer->serverPort(iface->interfaceAddr);
      if (port > 0)
      {
        const QString url = "http://" + iface->interfaceAddr.toString() + ":" +
                            QString::number(port) + i->relativeUrl;

        sendUpdate(iface, i.key(), url);
      }
    }
  }
}

void SSsdpServer::publishServices(void)
{
  for (QMultiMap<QString, Private::Service>::ConstIterator i = p->published.begin();
       i != p->published.end();
       i++)
  {
    for (unsigned j=0; j<i->msgCount; j++)
    foreach (SsdpClientInterface *iface, interfaces())
    {
      const quint16 port = p->httpServer->serverPort(iface->interfaceAddr);
      if (port > 0)
      {
        const QString url = "http://" + iface->interfaceAddr.toString() + ":" +
                            QString::number(port) + i->relativeUrl;

        sendAlive(iface, i.key(), url);
      }
    }
  }
}

void SSsdpServer::unpublishServices(void)
{
  for (QMultiMap<QString, Private::Service>::ConstIterator i = p->published.begin();
       i != p->published.end();
       i++)
  {
    for (unsigned j=0; j<i->msgCount; j++)
    foreach (SsdpClientInterface *iface, interfaces())
      sendByeBye(iface, i.key());
  }
}

} // End of namespace
