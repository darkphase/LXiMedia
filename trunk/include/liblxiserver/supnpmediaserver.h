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

#ifndef LXISERVER_SUPNPMEDIASERVER_H
#define LXISERVER_SUPNPMEDIASERVER_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include "shttpserver.h"
#include "export.h"

namespace LXiServer {

class SSsdpServer;

class LXISERVER_PUBLIC SUPnPMediaServer : public QObject,
                                          protected SHttpServer::Callback
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
  explicit                      SUPnPMediaServer(const QString &basePath, QObject * = NULL);
  virtual                       ~SUPnPMediaServer();

  void                          initialize(SHttpServer *, SSsdpServer *);
  void                          close(void);
  void                          reset(void);

  void                          setDeviceName(const QString &);

  void                          addIcon(const QString &url, unsigned width, unsigned height, unsigned depth);

  void                          registerService(const Service &);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

public:
  static const char             dlnaDeviceNS[];

private:
  static const char             deviceType[];

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
