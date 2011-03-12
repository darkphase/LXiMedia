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

#include "ssdpserver.h"

#if defined(Q_OS_UNIX)
#include <arpa/inet.h>
#include <sys/socket.h>
#elif defined(Q_OS_WIN)
#include <ws2tcpip.h>
#endif
#include "httpserver.h"

namespace LXiServer {

struct SsdpServer::Private
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

  inline Private(void) : lock(QReadWriteLock::Recursive) { }

  QReadWriteLock                lock;
  const HttpServer            * httpServer;
  QMultiMap<QString, Service>   published;
  QTimer                        publishTimer, autoPublishTimer;
};

SsdpServer::SsdpServer(const HttpServer *httpServer)
           :SsdpClient(httpServer->serverUdn()),
            p(new Private())
{
  p->httpServer = httpServer;

  p->publishTimer.setSingleShot(true);
  connect(&(p->publishTimer), SIGNAL(timeout()), SLOT(publishServices()));
  connect(&(p->autoPublishTimer), SIGNAL(timeout()), SLOT(publishServices()));
}

SsdpServer::~SsdpServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SsdpServer::initialize(const QList<QHostAddress> &interfaces)
{
  SsdpClient::initialize(interfaces);

  p->autoPublishTimer.start(((cacheTimeout / 2) - 300) * 1000);
}

void SsdpServer::close(void)
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

  SsdpClient::close();
}

void SsdpServer::publish(const QString &nt, const QString &relativeUrl, unsigned msgCount)
{
  QWriteLocker l(&p->lock);

  p->published.insert(nt, Private::Service(relativeUrl, msgCount));
  p->publishTimer.start(5000);
}

void SsdpServer::parsePacket(SsdpClientInterface *iface, const HttpServer::RequestHeader &header, const QHostAddress &sourceAddress, quint16 sourcePort)
{
  QWriteLocker l(&p->lock);

  if ((header.method().toUpper() == "M-SEARCH") && header.hasField("MX") &&
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
    SsdpClient::parsePacket(iface, header, sourceAddress, sourcePort);
}

void SsdpServer::sendAlive(SsdpClientInterface *iface, const QString &nt, const QString &url) const
{
  HttpServer::RequestHeader request(p->httpServer);
  request.setRequest("NOTIFY", "*", HttpServer::httpVersion);
  request.setServer();
  request.setField("HOST", SsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SsdpServer::ssdpPort));
  request.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  request.setField("CACHE-CONTROL", "max-age=" + QString::number(SsdpServer::cacheTimeout));
  request.setField("LOCATION", url);
  request.setField("NT", nt);
  request.setField("NTS", "ssdp:alive");

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SsdpServer::sendByeBye(SsdpClientInterface *iface, const QString &nt) const
{
  HttpServer::RequestHeader request(p->httpServer);
  request.setRequest("NOTIFY", "*", HttpServer::httpVersion);
  request.setField("HOST", SsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SsdpServer::ssdpPort));
  request.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  request.setField("NT", nt);
  request.setField("NTS", "ssdp:byebye");

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SsdpServer::sendSearchResponse(SsdpClientInterface *iface, const QString &nt, const QString &url, const QHostAddress &toAddr, quint16 toPort) const
{
  HttpServer::ResponseHeader response(p->httpServer);
  response.setResponse(HttpServer::Status_Ok, HttpServer::httpVersion);
  response.setField("USN", serverUdn() + (!nt.startsWith("uuid:") ? ("::" + nt) : QString::null));
  response.setField("CACHE-CONTROL", "max-age=" + QString::number(SsdpServer::cacheTimeout));
  response.setField("EXT", QString::null);
  response.setField("LOCATION", url);
  response.setField("ST", nt);

  sendDatagram(iface, response, toAddr, toPort);
}

void SsdpServer::publishServices(void)
{
  QWriteLocker l(&p->lock);

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
