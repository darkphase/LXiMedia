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

#include "sssdpclient.h"

namespace LXiServer {


struct SSsdpClient::Private
{
  QString                       serverUdn;
  QMultiMap<QString, QPointer<SsdpClientInterface> > interfaces;
  QMultiMap<QString, Node>      nodes;
  QTimer                        updateTimer;
};

const QHostAddress  SSsdpClient::ssdpAddressIPv4("239.255.255.250");
const QHostAddress  SSsdpClient::ssdpAddressIPv6("FF02::C");
const quint16       SSsdpClient::ssdpPort = 1900;
const int           SSsdpClient::cacheTimeout = 1800; // Alive messages are sent every (cacheTimeout/2)-300 seconds.

QList<QHostAddress> SSsdpClient::localAddresses(void)
{
  QList<QHostAddress> result;

  foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces())
  {
    QList<QNetworkAddressEntry> entries = interface.addressEntries();
    foreach (const QNetworkAddressEntry &entry, entries)
    {
      const QHostAddress address = entry.ip();
      if (address.protocol() == QAbstractSocket::IPv4Protocol)
      {
        const quint32 addr = address.toIPv4Address();
        if (((addr & 0xFF000000u) == 0x0A000000u) || //10.0.0.0/8
            ((addr & 0xFF000000u) == 0x7F000000u) || //127.0.0.0/8
            ((addr & 0xFFFF0000u) == 0xA9FE0000u) || //169.254.0.0/16
            ((addr & 0xFFF00000u) == 0xAC100000u) || //172.16.0.0/12
            ((addr & 0xFFFF0000u) == 0xC0A80000u))   //192.168.0.0/16
        {
          result += address;
        }
      }
    }
  }

  return result;
}

SSsdpClient::SSsdpClient(const QString &serverUdn)
           :QObject(),
            p(new Private())
{
  p->serverUdn = serverUdn;
  p->updateTimer.setSingleShot(true);
  connect(&p->updateTimer, SIGNAL(timeout()), SIGNAL(searchUpdated()));
}

SSsdpClient::~SSsdpClient()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SSsdpClient::initialize()
{
}

void SSsdpClient::close(void)
{
  foreach (SsdpClientInterface *iface, p->interfaces)
    delete iface;

  p->interfaces.clear();
}

bool SSsdpClient::bind(const QHostAddress &address)
{
  foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces())
  {
    QList<QNetworkAddressEntry> entries = interface.addressEntries();
    foreach (const QNetworkAddressEntry &entry, entries)
    if (entry.ip() == address)
    {
      p->interfaces.insert(
            address.toString(),
            new SsdpClientInterface(interface, entry.ip(), entry.netmask(), this));

      return true;
    }
  }

  return false;
}

void SSsdpClient::release(const QHostAddress &address)
{
  forever
  {
    QMultiMap<QString, QPointer<SsdpClientInterface> >::Iterator i = p->interfaces.find(address.toString());
    if (i != p->interfaces.end())
    {
      delete (*i);
      p->interfaces.erase(i);
    }
    else
      break;
  }
}

void SSsdpClient::sendSearch(const QString &st, unsigned msgCount)
{
  for (unsigned i=0; i<msgCount; i++)
  foreach (SsdpClientInterface *iface, interfaces())
    sendSearch(iface, st);
}

QList<SSsdpClient::Node> SSsdpClient::searchResults(const QString &st) const
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

const QString & SSsdpClient::serverUdn(void) const
{
  return p->serverUdn;
}

const QMultiMap<QString, QPointer<SsdpClientInterface> > & SSsdpClient::interfaces(void) const
{
  return p->interfaces;
}

void SSsdpClient::parsePacket(SsdpClientInterface *, const SHttpServer::RequestHeader &header, const QHostAddress &, quint16)
{
  if (header.method() == "NOTIFY")
  {
    const QString nts = header.field("NTS");

    if (nts == "ssdp:alive")
      addNode(header, "NT");
    else if (nts == "ssdp:byebye")
      removeNode(header);
  }
}

void SSsdpClient::parsePacket(SsdpClientInterface *, const SHttpServer::ResponseHeader &header, const QHostAddress &, quint16)
{
  addNode(header, "ST");
}

void SSsdpClient::sendDatagram(SsdpClientInterface *iface, const QByteArray &datagram, const QHostAddress &address, quint16 port)
{
  iface->privateSocket.writeDatagram(datagram, address, port);
}

void SSsdpClient::sendSearch(SsdpClientInterface *iface, const QString &st, unsigned mx)
{
  SHttpServer::RequestHeader request(NULL);
  request.setRequest("M-SEARCH", "*", SHttpServer::httpVersion);
  request.setField("USER-AGENT", SHttpServer::osName() + ' ' + SUPnPBase::protocol() + ' ' + qApp->applicationName() + '/' + qApp->applicationVersion());
  request.setField("HOST", SSsdpClient::ssdpAddressIPv4.toString() + ":" + QString::number(SSsdpClient::ssdpPort));
  request.setField("MAN", "\"ssdp:discover\"");
  request.setField("MX", QString::number(mx));
  request.setField("ST", st);

  sendDatagram(iface, request, ssdpAddressIPv4, ssdpPort);
}

void SSsdpClient::addNode(const SHttpServer::Header &header, const QString &tagName)
{
  const QString tag = header.field(tagName);
  const QString location = header.field("LOCATION");
  const QString uuid = header.field("USN").split("::").first();
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

void SSsdpClient::removeNode(const SHttpServer::Header &header)
{
  bool updated = false;

  const QString tag = header.field("NT");
  const QString uuid = header.field("USN").split("::").first();
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


SsdpClientInterface::SsdpClientInterface(const QNetworkInterface &interface, const QHostAddress &address, const QHostAddress &netmask, SSsdpClient *parent)
    : QObject(parent),
      parent(parent),
      interface(interface),
      address(address),
      netmask(netmask),
      ssdpSocket(this),
      privateSocket(this)
{
  connect(&ssdpSocket, SIGNAL(readyRead()), SLOT(ssdpDatagramReady()));
  connect(&privateSocket, SIGNAL(readyRead()), SLOT(privateDatagramReady()));

  if (ssdpSocket.bind(address, SSsdpClient::ssdpPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
  {
    if (!ssdpSocket.joinMulticastGroup(SSsdpClient::ssdpAddressIPv4, interface))
      qWarning() << "Failed to join multicast group on SSDP port for interface" << address.toString() << ssdpSocket.errorString();
  }
  else
    qWarning() << "Failed to bind the SSDP port for interface" << address.toString() << ssdpSocket.errorString();

  if (privateSocket.bind(address, 0))
  {
    privateSocket.setMulticastInterface(interface);
    privateSocket.setSocketOption(QAbstractSocket::MulticastTtlOption, 4);
    privateSocket.setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
  }
  else
    qWarning() << "Failed to bind the private port for interface" << address.toString() << privateSocket.errorString();
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
    {
      SHttpServer::RequestHeader request(NULL);
      request.parse(QByteArray(buffer, size));

      parent->parsePacket(this, request, sourceAddress, sourcePort);
    }
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
      const QByteArray txt(buffer, size);
      if (txt.startsWith("HTTP"))
      {
        SHttpServer::ResponseHeader response(NULL);
        response.parse(txt);

        parent->parsePacket(this, response, sourceAddress, sourcePort);
      }
    }
    else if (size < 0)
      break;
  }
}


} // End of namespace
