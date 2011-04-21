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

#ifndef LXISERVER_SUPNPCONNECTIONMANAGER_H
#define LXISERVER_SUPNPCONNECTIONMANAGER_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <LXiCore>
#include "shttpserver.h"
#include "supnpbase.h"
#include "export.h"

namespace LXiServer {

class SUPnPMediaServer;

class LXISERVER_PUBLIC SUPnPConnectionManager : public SUPnPBase
{
Q_OBJECT
public:
  explicit                      SUPnPConnectionManager(const QString &basePath, QObject * = NULL);
  virtual                       ~SUPnPConnectionManager();

  void                          initialize(SHttpServer *, SUPnPMediaServer *);
  void                          close(void);

  void                          setSourceProtocols(const ProtocolList &);
  void                          setSinkProtocols(const ProtocolList &);

protected: // From SUPnPBase
  virtual void                  buildDescription(QDomDocument &, QDomElement &);
  virtual void                  handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestMessage &, const QHostAddress &);

private:
  _lxi_internal void            emitEvent(void);
  _lxi_internal QString         listSourceProtocols(void) const;
  _lxi_internal QString         listSinkProtocols(void) const;

public:
  static const char             connectionManagerNS[];

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
