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
#include <LXiCore>
#include "shttpengine.h"
#include "export.h"

namespace LXiServer {

class LXISERVER_PUBLIC SSandboxServer : public SHttpServerEngine
{
Q_OBJECT
public:
  explicit                      SSandboxServer(QObject * = NULL);
  virtual                       ~SSandboxServer();

  bool                          initialize(const QString &mode);
  void                          close(void);

  QString                       serverName(void) const;

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
