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

  const SHttpServer            * httpServer;
  QMultiMap<QString, Service>   published;
  QTimer                        publishTimer, autoPublishTimer;
};

SSsdpServer::SSsdpServer(const SHttpServer *httpServer)
  : SSsdpClient(httpServer->serverUdn()),
    p(new Private())
{
  p->httpServer = httpServer;

  p->publishTimer.setSingleShot(true);
  connect(&(p->publishTimer), SIGNAL(timeout()), SLOT(publishServices()));
  connect(&(p->autoPublishTimer), SIGNAL(timeout()), SLOT(publishServices()));
}

SSsdpServer::~SSsdpServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SSsdpServer::initialize(const QList<QHostAddress> &interfaces)
{
  SSsdpClient::initialize(interfaces);

  p->autoPublishTimer.start(((cacheTimeout / 2) - 300) * 1000);
}

void SSsdpServer::close(void)
{
  for (QMultiMap<QString, Private::Service>::ConstIterator i = p->published.begin();
       i != p->published.end();
       i++)
  {
    for (unsigned j=0; j<i->msgCount; j++)
    foreach (SsdpClientInterface *iface, interfaces())
      sendByeBye(iface, i.key());
  }

  p->autoPublishTimer.stop();

  SSsdpClient::close();
}

void SSsdpServer::publish(const QString &nt, const QString &relativeUrl, unsigned msgCount)
{
  p->published.insert(nt, Private::Service(relativeUrl, msgCount));
  p->publishTimer.start(5000);
}

void SSsdpServer::parsePacket(SsdpClientInterface *iface, const SHttpServer::RequestHeader &header, const QHostAddress &sourceAddress, quint16 sourcePort)
{
  if ((header.method().compare("M-SEARCH", Qt::CaseInsensitive) == 0) && header.hasField("MX") &&
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

void SSsdpServer::sendAlive(SsdpClientInterface *iface, const QString &nt, const QString &url) const
{
  SHttpServer::RequestHeader request(p->httpServer);
  request.setRequest("NOTIFY", "*", SHttpServer::httpVersion);
  request.setField(p->httpServer->senderType(), p->httpServer->senderId());
  request.setField("HOST", SSsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SSsdpServer::ssdpPort));
  request.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  request.setField("CACHE-CONTROL", "max-age=" + QString::number(SSsdpServer::cacheTimeout));
  request.setField("LOCATION", url);
  request.setField("NT", nt);
  request.setField("NTS", "ssdp:alive");

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SSsdpServer::sendByeBye(SsdpClientInterface *iface, const QString &nt) const
{
  SHttpServer::RequestHeader request(p->httpServer);
  request.setRequest("NOTIFY", "*", SHttpServer::httpVersion);
  request.setField("HOST", SSsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SSsdpServer::ssdpPort));
  request.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  request.setField("NT", nt);
  request.setField("NTS", "ssdp:byebye");

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SSsdpServer::sendSearchResponse(SsdpClientInterface *iface, const QString &nt, const QString &url, const QHostAddress &toAddr, quint16 toPort) const
{
  SHttpServer::ResponseHeader response(p->httpServer);
  response.setResponse(SHttpServer::Status_Ok, SHttpServer::httpVersion);
  response.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  response.setField("CACHE-CONTROL", "max-age=" + QString::number(SSsdpServer::cacheTimeout));
  response.setField("EXT", QString::null);
  response.setField("LOCATION", url);
  response.setField("ST", nt);

  sendDatagram(iface, response, toAddr, toPort);
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

} // End of namespace
