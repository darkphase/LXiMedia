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

#ifndef LXISERVER_SSSDPCLIENT_H
#define LXISERVER_SSSDPCLIENT_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include "shttpserver.h"
#include "export.h"

namespace LXiServer {

class SsdpClientInterface;

class LXISERVER_PUBLIC SSsdpClient : public QObject
{
Q_OBJECT
friend class SsdpClientInterface;
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
  explicit                      SSsdpClient(const QString &serverUdn);
  virtual                       ~SSsdpClient();

  virtual void                  initialize(const QList<QHostAddress> &interfaces);
  virtual void                  close(void);

  void                          sendSearch(const QString &st, unsigned msgCount = 3);
  QList<Node>                   searchResults(const QString &st) const;

signals:
  void                          searchUpdated(void);

protected:
  const QString               & serverUdn(void) const;
  const QList<SsdpClientInterface *> & interfaces(void) const;
  virtual void                  parsePacket(SsdpClientInterface *, const SHttpServer::RequestHeader &, const QHostAddress &, quint16);
  virtual void                  parsePacket(SsdpClientInterface *, const SHttpServer::ResponseHeader &, const QHostAddress &, quint16);

  static void                   sendDatagram(SsdpClientInterface *, const QByteArray &, const QHostAddress &, quint16);
  static void                   sendSearch(SsdpClientInterface *, const QString &st, unsigned mx = 5);

private:
  _lxi_internal void            addNode(const SHttpServer::Header &, const QString &);
  _lxi_internal void            removeNode(const SHttpServer::Header &);

public:
  static const QHostAddress     ssdpAddressIPv4;
  static const QHostAddress     ssdpAddressIPv6;
  static const quint16          ssdpPort;
  static const int              cacheTimeout;

private:
  struct Private;
  Private               * const p;
};


class LXISERVER_PUBLIC SsdpClientInterface : public QObject
{
Q_OBJECT
friend class SSsdpClient;
private:
                                SsdpClientInterface(const QHostAddress &, SSsdpClient *);

  bool                          joinMulticastGroup(QUdpSocket &, const QHostAddress &);

private slots:
  void                          ssdpDatagramReady(void);
  void                          privateDatagramReady(void);

public:
  SSsdpClient            * const parent;
  const QHostAddress            interfaceAddr;

private:
  QUdpSocket                    ssdpSocket;
  QUdpSocket                    privateSocket;
};


} // End of namespace

#endif
