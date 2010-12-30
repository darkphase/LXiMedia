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

#ifndef LXMEDIACENTER_SSDPCLIENT_H
#define LXMEDIACENTER_SSDPCLIENT_H

#include <QtCore>
#include <QtNetwork>

namespace LXiMediaCenter {

class SsdpClientInterface;

class SsdpClient : public QObject
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
  explicit                      SsdpClient(void);
  virtual                       ~SsdpClient();

  virtual void                  initialize(const QList<QHostAddress> &interfaces);
  virtual void                  close(void);

  void                          sendSearch(const QString &st);
  QList<Node>                   searchResults(const QString &st) const;

signals:
  void                          searchUpdated(void);

public:
  static QString                getUuid(void);

protected:
  const QList<SsdpClientInterface *> & interfaces(void) const;
  virtual void                  parsePacket(SsdpClientInterface *, const QHttpRequestHeader &, const QHostAddress &, quint16);
  virtual void                  parsePacket(SsdpClientInterface *, const QHttpResponseHeader &, const QHostAddress &, quint16);

  static void                   sendDatagram(SsdpClientInterface *, const QHttpHeader &, const QHostAddress &, quint16);
  static void                   sendSearch(SsdpClientInterface *, const QString &st);

private:
  void                          addNode(const QHttpHeader &, const QString &);
  void                          removeNode(const QHttpHeader &);

public:
  static const QHostAddress     ssdpAddressIPv4;
  static const QHostAddress     ssdpAddressIPv6;
  static const quint16          ssdpPort;
  static const int              cacheTimeout;

private:
  struct Private;
  Private               * const p;
};


class SsdpClientInterface : public QObject
{
Q_OBJECT
friend class SsdpClient;
private:
                                SsdpClientInterface(const QHostAddress &, SsdpClient *);

  bool                          joinMulticastGroup(QUdpSocket &, const QHostAddress &);

private slots:
  void                          ssdpDatagramReady(void);
  void                          privateDatagramReady(void);

public:
  SsdpClient            * const parent;
  const QHostAddress            interfaceAddr;

private:
  QUdpSocket                    ssdpSocket;
  QUdpSocket                    privateSocket;
};


} // End of namespace

#endif
