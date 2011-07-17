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

#ifndef LXISERVER_SUPNPMEDIARECEIVERREGISTRAR_H
#define LXISERVER_SUPNPMEDIARECEIVERREGISTRAR_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <LXiCore>
#include "shttpserver.h"
#include "supnpbase.h"
#include "export.h"

namespace LXiServer {

class SUPnPMediaServer;

class LXISERVER_PUBLIC SUPnPMediaReceiverRegistrar : public SUPnPBase
{
Q_OBJECT
public:
  explicit                      SUPnPMediaReceiverRegistrar(const QString &basePath, QObject * = NULL);
  virtual                       ~SUPnPMediaReceiverRegistrar();

  void                          initialize(SHttpServer *, SUPnPMediaServer *);
  void                          close(void);

protected: // From SUPnPBase
  virtual void                  buildDescription(QDomDocument &, QDomElement &);
  virtual void                  handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestMessage &, const QHostAddress &);

private:
  _lxi_internal void            emitEvent(void);

public:
  static const char             mediaReceiverRegistrarNS[];

private:
  _lxi_internal static const char datatypesNS[];

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
