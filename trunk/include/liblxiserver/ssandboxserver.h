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

  bool                          initialize(const QString &mode, const QString &name);
  void                          close(void);

  QString                       serverName(void) const;

protected:
  virtual void                  customEvent(QEvent *);

signals:
  void                          started(void);
  void                          finished(void);
  void                          busy(void);
  void                          idle(void);

private slots:
  void                          newConnection(void);
  void                          closedConnection(void);

private:
  static const QEvent::Type     closeServerEventType;

  class ReadThread;
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
