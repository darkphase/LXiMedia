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

#ifndef LXISERVER_SSSDPCLIENT_H
#define LXISERVER_SSSDPCLIENT_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include "shttpserver.h"
#include "export.h"

namespace LXiServer {

class LXISERVER_PUBLIC SSsdpClient : public QObject
{
Q_OBJECT
public:
  struct Node
  {
    inline Node(void) { }
    inline Node(QString uuid, QString location) : uuid(uuid), location(location) { }
    inline bool operator==(const Node &c) { return (uuid == c.uuid) && (location == c.location); }
    inline bool operator!=(const Node &c) { return !operator==(c); }

    QString                     uuid;
    QString                     location;
  };

public:
  /*! Returns a list of local IP addresses (i.e. in 10.0.0.0/8, 127.0.0.0/8,
      169.254.0.0/16, 172.16.0.0/12 or 192.168.0.0/16). This list can be used to
      prevent binding internet interfaces.
   */
  static QList<QHostAddress>    localAddresses(void);

  explicit                      SSsdpClient(const QString &serverUdn);
  virtual                       ~SSsdpClient();

  virtual void                  initialize();
  virtual void                  close(void);
  virtual bool                  bind(const QHostAddress &address);
  virtual void                  release(const QHostAddress &address);

  void                          sendSearch(const QString &st, unsigned msgCount = 3);
  QList<Node>                   searchResults(const QString &st) const;

signals:
  void                          searchUpdated(void);

protected:
  const QString               & serverUdn(void) const;
  QList<QHostAddress>           interfaces(void) const;
  virtual void                  parsePacket(const QHostAddress &iface, const SHttpServer::RequestHeader &, const QHostAddress &, quint16);
  virtual void                  parsePacket(const QHostAddress &iface, const SHttpServer::ResponseHeader &, const QHostAddress &, quint16);

  void                          sendDatagram(const QHostAddress &iface, const QByteArray &, const QHostAddress &, quint16) const;
  void                          sendSearch(const QHostAddress &iface, const QString &st, unsigned mx = 5);

private:
  void                          addNode(const SHttpServer::Header &, const QString &);
  void                          removeNode(const SHttpServer::Header &);

private slots:
  void                          ssdpDatagramReady(void);
  void                          privateDatagramReady(void);

public:
  static const QHostAddress     ssdpAddressIPv4;
  static const QHostAddress     ssdpAddressIPv6;
  static const quint16          ssdpPort;
  static const int              cacheTimeout;

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
