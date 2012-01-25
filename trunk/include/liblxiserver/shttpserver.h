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

#ifndef LXISERVER_SHTTPSERVER_H
#define LXISERVER_SHTTPSERVER_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include "shttpengine.h"
#include "export.h"

namespace LXiServer {

/*! This class provides a basic HTTP server.
 */
class LXISERVER_PUBLIC SHttpServer : public SHttpServerEngine
{
Q_OBJECT
public:
                                SHttpServer(const QString &protocol, const QUuid &serverUuid, QObject * = NULL);
  virtual                       ~SHttpServer();

  void                          initialize(const QList<QHostAddress> &addresses, quint16 port = 0);
  void                          close(void);
  void                          reset(const QList<QHostAddress> &addresses, quint16 port = 0);

  quint16                       defaultPort(void) const;
  quint16                       serverPort(const QHostAddress &) const;
  const QString               & serverUdn(void) const;

signals:
  void                          busy(void);
  void                          idle(void);

private slots:
  void                          newConnection(void);
  void                          closedConnection(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
