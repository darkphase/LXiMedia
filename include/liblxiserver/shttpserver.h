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

#ifndef LXISERVER_SHTTPSERVER_H
#define LXISERVER_SHTTPSERVER_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include "shttpengine.h"
#include "export.h"

namespace LXiServer {

class LXISERVER_PUBLIC SHttpServer : public SHttpServerEngine
{
Q_OBJECT
public:
                                SHttpServer(const QString &protocol, const QUuid &serverUuid, QObject * = NULL);
  virtual                       ~SHttpServer();

  void                          initialize(const QList<QHostAddress> &addresses, quint16 port = 0);
  void                          close(void);

  quint16                       serverPort(const QHostAddress &) const;
  const QString               & serverUdn(void) const;

protected: // From HttpServerEngine
  virtual QIODevice           * openSocket(quintptr);
  virtual void                  closeSocket(QIODevice *, bool canReuse);

private:
  class Socket;
  class Interface;
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
