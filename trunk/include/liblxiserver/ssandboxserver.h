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

#ifndef LXISERVER_SSANDBOXSERVER_H
#define LXISERVER_SSANDBOXSERVER_H

#include <QtCore>
#include "shttpengine.h"

namespace LXiServer {

class SSandboxServer : public SHttpServerEngine
{
Q_OBJECT
public:
  explicit                      SSandboxServer(QObject * = NULL);
  virtual                       ~SSandboxServer();

  void                          initialize(const QString &name, const QString &mode);
  void                          close(void);

signals:
  void                          busy(void);
  void                          idle(void);

protected: // From HttpServerEngine
  virtual QIODevice           * openSocket(quintptr);
  virtual void                  closeSocket(QIODevice *, bool canReuse);

private:
  class Socket;
  class Server;
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
