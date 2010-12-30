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

#ifndef LXMEDIACENTER_SSDPSERVER_H
#define LXMEDIACENTER_SSDPSERVER_H

#include <QtCore>
#include <QtNetwork>
#include <liblximediacenter/ssdpclient.h>

namespace LXiMediaCenter {

class HttpServer;

class SsdpServer : public SsdpClient
{
Q_OBJECT
public:
  explicit                      SsdpServer(void);
  virtual                       ~SsdpServer();

  virtual void                  initialize(const QList<QHostAddress> &interfaces);
  virtual void                  close(void);

  void                          publish(const QString &nt, const HttpServer *, const QString &url);

protected:
  virtual void                  parsePacket(SsdpClientInterface *, const QHttpRequestHeader &, const QHostAddress &, quint16);

  static void                   sendAlive(SsdpClientInterface *, const QString &nt, const QString &url);
  static void                   sendByeBye(SsdpClientInterface *, const QString &nt);
  static void                   sendSearchResponse(SsdpClientInterface *, const QString &st, const QString &url, const QHostAddress &, quint16);

public:
  static QString                getServerId(void);

private slots:
  void                          publishServices(void);

private:
  struct Private;
  Private               * const p;
};


} // End of namespace

#endif
