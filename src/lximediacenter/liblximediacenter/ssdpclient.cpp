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

#include "ssdpclient.h"

#if defined(Q_OS_UNIX)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#elif defined(Q_OS_WIN)
#include <ws2tcpip.h>
#endif
#include <liblximediacenter/globalsettings.h>

namespace LXiMediaCenter {


struct SsdpClient::Private
{
  QList<SsdpClientInterface *>  interfaces;
  QMultiMap<QString, Node>      nodes;
  QTimer                        updateTimer;
};

const QHostAddress  SsdpClient::ssdpAddressIPv4("239.255.255.250");
const QHostAddress  SsdpClient::ssdpAddressIPv6("FF02::C");
const quint16       SsdpClient::ssdpPort = 1900;
const int           SsdpClient::cacheTimeout = 300000;


SsdpClient::SsdpClient(void)
           :QObject(),
            p(new Private())
{
  p->updateTimer.setSingleShot(true);
  connect(&p->updateTimer, SIGNAL(timeout()), SIGNAL(searchUpdated()));
}

SsdpClient::~SsdpClient()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SsdpClient::initialize(const QList<QHostAddress> &interfaces)
{
  foreach (const QHostAddress &address, interfaces)
    p->interfaces += new SsdpClientInterface(address, this);
}

void SsdpClient::close(void)
{
  foreach (SsdpClientInterface *iface, p->interfaces)
    delete iface;

  p->interfaces.clear();
}

void SsdpClient::sendSearch(const QString &st)
{
  foreach (SsdpClientInterface *iface, interfaces())
    sendSearch(iface, st);
}

QList<SsdpClient::Node> SsdpClient::searchResults(const QString &st) const
{
  // If multiple urls are returned for the same UUID, which happens for servers
  // running on the local host, the localhost based url is preferred over the
  // LAN based url, which in term is preferred over an internet based URL.
  QMap<QString, QPair<QString, int> > uniqueValues;
  foreach (const Node &node, p->nodes.values(st))
  {
    int pref = 0;
    const QHostAddress host(QUrl(node.location).host());
    if (host.protocol() == QAbstractSocket::IPv4Protocol)
    {
      const quint32 addr = host.toIPv4Address();
      if ((addr & 0xFF000000u) == 0x7F000000u) //127.0.0.0/8
        pref = 5;
      else if ((addr & 0xFFFF0000u) == 0xC0A80000u) //192.168.0.0/16
        pref = 4;
      else if ((addr & 0xFF000000u) == 0x0A000000u) //10.0.0.0/8
        pref = 3;
      else if ((addr & 0xFFF00000u) == 0xAC100000u) //172.16.0.0/12
        pref = 2;
      else if ((addr & 0xFFFF0000u) == 0xA9FE0000u) //169.254.0.0/16
        pref = 1;
    }

    QMap<QString, QPair<QString, int> >::Iterator i = uniqueValues.find(node.uuid);
    if (i != uniqueValues.end())
    {
      if (i->second < pref)
      {
        i->first = node.location;
        i->second = pref;
      }
    }
    else
      i = uniqueValues.insert(node.uuid, QPair<QString, int>(node.location, pref));
  }

  QList<Node> result;
  for (QMap<QString, QPair<QString, int> >::ConstIterator i = uniqueValues.begin();
       i != uniqueValues.end();
       i++)
  {
    result += Node(i.key(), i->first);
  }

  return result;
}

QString SsdpClient::getUuid(void)
{
  QString uuid = "00000000-0000-0000-0000-000000000000";

  GlobalSettings settings;
  settings.beginGroup("SSDP");

  if (settings.contains("UUID"))
    return settings.value("UUID", uuid).toString();

  uuid = QUuid::createUuid();
  settings.setValue("UUID", uuid);
  return uuid;
}

const QList<SsdpClientInterface *> & SsdpClient::interfaces(void) const
{
  return p->interfaces;
}

void SsdpClient::parsePacket(SsdpClientInterface *, const QHttpRequestHeader &header, const QHostAddress &, quint16)
{
  if (header.method().toUpper() == "NOTIFY")
  {
    const QString nts = header.value("NTS");

    if (nts == "ssdp:alive")
      addNode(header, "NT");
    else if (nts == "ssdp:byebye")
      removeNode(header);
  }
}

void SsdpClient::parsePacket(SsdpClientInterface *, const QHttpResponseHeader &header, const QHostAddress &, quint16)
{
  addNode(header, "ST");
}

void SsdpClient::sendDatagram(SsdpClientInterface *iface, const QHttpHeader &header, const QHostAddress &address, quint16 port)
{
  iface->privateSocket.writeDatagram(header.toString().toUtf8(), address, port);
}

void SsdpClient::sendSearch(SsdpClientInterface *iface, const QString &st)
{
  QHttpRequestHeader request;
  request.setRequest("M-SEARCH", "*", 1, 1);
  request.setValue("HOST", SsdpClient::ssdpAddressIPv4.toString() + ":" + QString::number(SsdpClient::ssdpPort));
  request.setValue("MAN", "ssdp:discover");
  request.setValue("ST", st);
  request.setValue("MX", "3");

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SsdpClient::addNode(const QHttpHeader &header, const QString &tagName)
{
  const QString tag = header.value(tagName);
  const QString location = header.value("LOCATION");
  const QString uuid = header.value("USN").split("::").first();
  if (!tag.isEmpty() && !location.isEmpty() && !uuid.isEmpty())
  {
    const Node entry(uuid, location);
    if (!p->nodes.values(tag).contains(entry))
    {
      p->nodes.insert(tag, entry);
      p->updateTimer.start(250);
    }
  }
}

void SsdpClient::removeNode(const QHttpHeader &header)
{
  bool updated = false;

  const QString tag = header.value("NT");
  const QString uuid = header.value("USN").split("::").first();
  if (!tag.isEmpty() && !uuid.isEmpty())
  {
    for (QMultiMap<QString, Node>::Iterator i = p->nodes.lowerBound(tag);
         (i != p->nodes.end()) && (i.key() == tag);
         )
    {
      if (i->uuid == uuid)
      {
        i = p->nodes.erase(i);
        updated = true;
      }
      else
        i++;
    }
  }

  if (updated)
    p->updateTimer.start(250);
}


SsdpClientInterface::SsdpClientInterface(const QHostAddress &interfaceAddr, SsdpClient *parent)
    : QObject(parent),
      parent(parent),
      interfaceAddr(interfaceAddr),
      ssdpSocket(this),
      privateSocket(this)
{
  connect(&ssdpSocket, SIGNAL(readyRead()), SLOT(ssdpDatagramReady()));
  connect(&privateSocket, SIGNAL(readyRead()), SLOT(privateDatagramReady()));

  if (ssdpSocket.bind(QHostAddress::Any, SsdpClient::ssdpPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
  {
    if (!joinMulticastGroup(ssdpSocket, SsdpClient::ssdpAddressIPv4))
      qWarning() << "Failed to join multicast group on SSDP port for interface" << interfaceAddr.toString();
  }
  else
    qWarning() << "Failed to bind the SSDP port for interface" << interfaceAddr.toString();

  if (privateSocket.bind(interfaceAddr, 0))
  {
    if (!joinMulticastGroup(privateSocket, SsdpClient::ssdpAddressIPv4))
      qWarning() << "Failed to join multicast group on private port for interface" << interfaceAddr.toString();
  }
  else
    qWarning() << "Failed to bind the private port for interface" << interfaceAddr.toString();
}

bool SsdpClientInterface::joinMulticastGroup(QUdpSocket &sock, const QHostAddress &address)
{
  struct ip_mreq mreq;
  memset(&mreq, 0, sizeof(mreq));
  mreq.imr_multiaddr.s_addr = inet_addr(address.toString().toAscii().data());
  mreq.imr_interface.s_addr = inet_addr(interfaceAddr.toString().toAscii().data());

  if (setsockopt(sock.socketDescriptor(),
                 IPPROTO_IP, IP_ADD_MEMBERSHIP,
                 reinterpret_cast<const char *>(&mreq), sizeof(mreq)) != -1)
  {
    const int ttl = 4;

    if (setsockopt(sock.socketDescriptor(),
#ifndef Q_OS_WIN
                   IPPROTO_IP, IP_TTL,
#else
                   IPPROTO_IP, IP_MULTICAST_TTL,
#endif
                   reinterpret_cast<const char *>(&ttl), sizeof(ttl)) != -1)
    {
      return true;
    }

    return true; // TTL is not that important
  }

#ifdef Q_OS_WIN
  qDebug() << "setsockopt failed with code" << WSAGetLastError() << "on socket" << sock.socketDescriptor();
#endif

  return false;
}

void SsdpClientInterface::ssdpDatagramReady(void)
{
  while (ssdpSocket.hasPendingDatagrams())
  {
    QHostAddress sourceAddress;
    quint16 sourcePort = 0;
    char buffer[65536];
    const qint64 size = ssdpSocket.readDatagram(buffer, sizeof(buffer), &sourceAddress, &sourcePort);

    if (size > 0)
      parent->parsePacket(this, QHttpRequestHeader(QString::fromUtf8(buffer, size)), sourceAddress, sourcePort);
  }
}

void SsdpClientInterface::privateDatagramReady(void)
{
  while (privateSocket.hasPendingDatagrams())
  {
    QHostAddress sourceAddress;
    quint16 sourcePort = 0;
    char buffer[65536];
    const qint64 size = privateSocket.readDatagram(buffer, sizeof(buffer), &sourceAddress, &sourcePort);

    if (size > 0)
    {
      const QString txt = QString::fromUtf8(buffer, size);
      if (txt.startsWith("HTTP"))
        parent->parsePacket(this, QHttpResponseHeader(txt), sourceAddress, sourcePort);
    }
    else if (size < 0)
      break;
  }
}


} // End of namespace
