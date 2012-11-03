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

  virtual void                  initialize(const QList<QHostAddress> &interfaces);
  virtual void                  close(void);
  void                          reset(void);

  void                          publish(const QString &nt, const QString &relativeUrl, unsigned msgCount);

protected:
  virtual void                  parsePacket(SsdpClientInterface *, const SHttpServer::RequestHeader &, const QHostAddress &, quint16);

  void                          sendUpdate(SsdpClientInterface *, const QString &nt, const QString &url) const;
  void                          sendAlive(SsdpClientInterface *, const QString &nt, const QString &url) const;
  void                          sendByeBye(SsdpClientInterface *, const QString &nt) const;
  void                          sendSearchResponse(SsdpClientInterface *, const QString &st, const QString &url, const QHostAddress &, quint16) const;

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
