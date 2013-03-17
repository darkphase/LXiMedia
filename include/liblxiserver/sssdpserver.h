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

#ifndef LXISERVER_SSSDPSERVER_H
#define LXISERVER_SSSDPSERVER_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include "sssdpclient.h"
#include "export.h"

namespace LXiServer {

class SHttpServer;

class LXISERVER_PUBLIC SSsdpServer : public SSsdpClient
{
Q_OBJECT
public:
  explicit                      SSsdpServer(SHttpServer *);
  virtual                       ~SSsdpServer();

  virtual void                  initialize();
  virtual void                  close(void);
  void                          reset(void);
  virtual bool                  bind(const QHostAddress &address);
  virtual void                  release(const QHostAddress &address);

  void                          publish(const QString &nt, const QString &relativeUrl, unsigned msgCount);

protected:
  virtual void                  parsePacket(const QHostAddress &iface, const SHttpServer::RequestHeader &, const QHostAddress &, quint16);

  void                          sendUpdate(const QHostAddress &iface, const QString &nt, const QString &url) const;
  void                          sendAlive(const QHostAddress &iface, const QString &nt, const QString &url) const;
  void                          sendByeBye(const QHostAddress &iface, const QString &nt) const;
  void                          sendSearchResponse(const QHostAddress &iface, const QString &st, const QString &url, const QHostAddress &, quint16) const;

private:
  void                          publishServices(const QHostAddress &iface);
  void                          unpublishServices(const QHostAddress &iface);

private slots:
  void                          updateServices(void);
  void                          publishServices(void);
  void                          unpublishServices(void);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
