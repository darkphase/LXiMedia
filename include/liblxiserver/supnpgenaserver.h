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

#ifndef LXISERVER_SUPNPGENASERVER_H
#define LXISERVER_SUPNPGENASERVER_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <LXiCore>
#include "shttpserver.h"
#include "export.h"

namespace LXiServer {

class LXISERVER_PUBLIC SUPnPGenaServer : public QObject,
                                         protected SHttpServer::Callback
{
Q_OBJECT
public:
                                SUPnPGenaServer(const QString &basePath, QObject * = NULL);
  virtual                       ~SUPnPGenaServer();

  QString                       path(void) const;

  void                          initialize(SHttpServer *);
  void                          close(void);
  void                          reset(void);

public slots:
  void                          emitEvent(const QDomDocument &doc);

protected:
  virtual void                  customEvent(QEvent *);
  virtual void                  timerEvent(QTimerEvent *);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);
  virtual SHttpServer::ResponseMessage httpOptions(const SHttpServer::RequestMessage &);

private slots:
  void                          emitEvents(void);

private:
  QString                       makeSid(void);

public:
  static const char             eventNS[];

private:
  static const QEvent::Type     scheduleEventType;

  class EventSession;
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
