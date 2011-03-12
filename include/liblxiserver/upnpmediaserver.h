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

#ifndef LXISERVER_UPNPMEDIASERVER_H
#define LXISERVER_UPNPMEDIASERVER_H

#include <QtCore>
#include <QtNetwork>
#include "httpserver.h"

namespace LXiServer {

class SsdpServer;

class UPnPMediaServer : public QObject,
                        protected HttpServer::Callback
{
Q_OBJECT
public:
  struct Service
  {
    QString                     serviceType;
    QString                     serviceId;
    QString                     descriptionUrl;
    QString                     controlURL;
    QString                     eventSubURL;
  };

public:
  explicit                      UPnPMediaServer(const QString &basePath, QObject * = NULL);
  virtual                       ~UPnPMediaServer();

  void                          initialize(HttpServer *, SsdpServer *);
  void                          close(void);

  void                          addIcon(const QString &url, unsigned width, unsigned height, unsigned depth);

  void                          registerService(const Service &);

protected: // From HttpServer::Callback
  virtual HttpServer::SocketOp  handleHttpRequest(const HttpServer::RequestHeader &, QAbstractSocket *);

public:
  static const char     * const dlnaDeviceNS;

private:
  static const char     * const deviceType;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
