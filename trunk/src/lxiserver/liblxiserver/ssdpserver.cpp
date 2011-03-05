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
#include <sys/utsname.h>
#elif defined(Q_OS_WIN)
#include <ws2tcpip.h>
#endif
#include "httpserver.h"

namespace LXiServer {


struct SsdpServer::Private
{
  static const int              publishTime = 300;

  QString                       serverId;
  QMultiMap<QString, QPair<const HttpServer *, QString> > published;
  QTimer                        publishTimer;
};


SsdpServer::SsdpServer(const QUuid &serverUuid, const QString &serverId)
           :SsdpClient(serverUuid),
            p(new Private())
{
  if (serverId.isEmpty())
  {
#if defined(Q_OS_UNIX)
    struct utsname osname;
    if (uname(&osname) >= 0)
      p->serverId = QString(osname.sysname) + "/" + QString(osname.release);
    else
      p->serverId = "Unix";
#elif defined(Q_OS_WIN)
    p->serverId = "Windows";
#endif

    p->serverId += ", UPnP/1.0";
    p->serverId += ", " + qApp->applicationName() + "/" + qApp->applicationVersion();
  }
  else
    p->serverId = serverId;

  connect(&(p->publishTimer), SIGNAL(timeout()), SLOT(publishServices()));
}

SsdpServer::~SsdpServer()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SsdpServer::initialize(const QList<QHostAddress> &interfaces)
{
  SsdpClient::initialize(interfaces);

  p->publishTimer.start(p->publishTime * 1000 / 2);
}

void SsdpServer::close(void)
{
  for (QMultiMap<QString, QPair<const HttpServer *, QString> >::ConstIterator i = p->published.begin();
       i != p->published.end();
       i++)
  {
    foreach (SsdpClientInterface *iface, interfaces())
      sendByeBye(iface, i.key());
  }

  SsdpClient::close();
}

void SsdpServer::publish(const QString &nt, const HttpServer *httpServer, const QString &url)
{
  p->published.insert(nt, QPair<const HttpServer *, QString>(httpServer, url));
  QTimer::singleShot(1000, this, SLOT(publishServices()));
}

void SsdpServer::parsePacket(SsdpClientInterface *iface, const HttpServer::RequestHeader &header, const QHostAddress &sourceAddress, quint16 sourcePort)
{
  if ((header.method().toUpper() == "M-SEARCH") && !sourceAddress.isNull() && sourcePort)
  {
    const QString st = header.field("ST");
    const bool findAll = st == "ssdp:all";

    for (QMultiMap<QString, QPair<const HttpServer *, QString> >::ConstIterator i = !findAll ? p->published.find(st) : p->published.begin();
         (i != p->published.end()) && (findAll || (i.key() == st));
         i++)
    {
      const quint16 port = i.value().first->serverPort(iface->interfaceAddr);
      if (port > 0)
      {
        const QString url = "http://" + iface->interfaceAddr.toString() + ":" +
                            QString::number(port) + i.value().second;

        sendSearchResponse(iface, i.key(), url, sourceAddress, sourcePort);
      }
    }
  }
  else
    SsdpClient::parsePacket(iface, header, sourceAddress, sourcePort);
}

void SsdpServer::sendAlive(SsdpClientInterface *iface, const QString &nt, const QString &url) const
{
  HttpServer::RequestHeader request;
  request.setRequest("NOTIFY", "*", "HTTP/1.1");
  request.setField("SERVER", p->serverId);
  request.setField("USN", "uuid:" + serverUuid() + "::" + nt);
  request.setField("CACHE-CONTROL", "max-age=" + QString::number((SsdpServer::cacheTimeout / 1000) + 30));
  request.setField("LOCATION", url);
  request.setField("NTS", "ssdp:alive");
  request.setField("HOST", SsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SsdpServer::ssdpPort));
  request.setField("NT", nt);

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SsdpServer::sendByeBye(SsdpClientInterface *iface, const QString &nt) const
{
  HttpServer::RequestHeader request;
  request.setRequest("NOTIFY", "*", "HTTP/1.1");
  request.setField("SERVER", p->serverId);
  request.setField("USN", "uuid:" + serverUuid() + "::" + nt);
  request.setField("NTS", "ssdp:byebye");
  request.setField("HOST", SsdpServer::ssdpAddressIPv4.toString() + ":" + QString::number(SsdpServer::ssdpPort));
  request.setField("NT", nt);

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SsdpServer::sendSearchResponse(SsdpClientInterface *iface, const QString &nt, const QString &url, const QHostAddress &toAddr, quint16 toPort) const
{
  HttpServer::ResponseHeader response;
  response.setResponse(HttpServer::Status_Ok, "HTTP/1.1");
  response.setField("SERVER", p->serverId);
  response.setField("S", "uuid:" + serverUuid());
  response.setField("USN", "uuid:" + serverUuid() + "::" + nt);
  response.setField("CACHE-CONTROL", "max-age=" + QString::number((SsdpServer::cacheTimeout / 1000) + 30));
  response.setField("LOCATION", url);
  response.setField("ST", nt);

  sendDatagram(iface, response, toAddr, toPort);
}

const QString & SsdpServer::serverId(void) const
{
  return p->serverId;
}

void SsdpServer::publishServices(void)
{
  for (QMultiMap<QString, QPair<const HttpServer *, QString> >::ConstIterator i = p->published.begin();
       i != p->published.end();
       i++)
  {
    foreach (SsdpClientInterface *iface, interfaces())
    {
      const quint16 port = i.value().first->serverPort(iface->interfaceAddr);
      if (port > 0)
      {
        const QString url = "http://" + iface->interfaceAddr.toString() + ":" +
                            QString::number(port) + i.value().second;

        sendAlive(iface, i.key(), url);
      }
    }
  }
}

} // End of namespace
