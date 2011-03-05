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

#ifndef LXISERVER_UPNPCONNECTIONMANAGER_H
#define LXISERVER_UPNPCONNECTIONMANAGER_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include "httpserver.h"
#include "upnpbase.h"

namespace LXiServer {

class UPnPMediaServer;

class UPnPConnectionManager : public UPnPBase
{
Q_OBJECT
public:
  explicit                      UPnPConnectionManager(QObject * = NULL);
  virtual                       ~UPnPConnectionManager();

  void                          setSourceProtocols(const QMap<QByteArray, QList<QByteArray> > &);
  void                          setSinkProtocols(const QMap<QByteArray, QList<QByteArray> > &);

protected: // From UPnPBase
  virtual void                  buildDescription(QDomDocument &, QDomElement &);
  virtual void                  handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const HttpServer::RequestHeader &, const QHostAddress &);
  virtual void                  addEventProperties(QDomDocument &, QDomElement &);

private:
  QByteArray                    listSourceProtocols(void) const;
  QByteArray                    listSinkProtocols(void) const;

public:
  static const char     * const connectionManagerNS;

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
