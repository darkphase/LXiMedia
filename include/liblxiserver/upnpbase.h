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

#ifndef LXISERVER_UPNPBASE_H
#define LXISERVER_UPNPBASE_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include "httpserver.h"

namespace LXiServer {

class UPnPMediaServer;

class UPnPBase : public QObject,
                 protected HttpServer::Callback
{
Q_OBJECT
public:
                                UPnPBase(const QString &basePath, const QString &serviceType, const QString &serviceId, QObject * = NULL);
  virtual                       ~UPnPBase();

  virtual void                  initialize(HttpServer *, UPnPMediaServer *);
  virtual void                  close(void);

public slots:
  virtual void                  emitEvent(void);

protected:
  virtual void                  timerEvent(QTimerEvent *);

protected: // From HttpServer::Callback
  virtual HttpServer::SocketOp  handleHttpRequest(const HttpServer::RequestHeader &, QAbstractSocket *);

protected:
  virtual HttpServer::SocketOp  handleControl(const HttpServer::RequestHeader &, QAbstractSocket *);
  virtual HttpServer::SocketOp  handleEventSub(const HttpServer::RequestHeader &, QAbstractSocket *);
  virtual HttpServer::SocketOp  handleDescription(const HttpServer::RequestHeader &, QAbstractSocket *);

  virtual void                  buildDescription(QDomDocument &, QDomElement &) = 0;
  virtual void                  handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const HttpServer::RequestHeader &, const QHostAddress &) = 0;
  virtual void                  addEventProperties(QDomDocument &, QDomElement &) = 0;

protected:
  QReadWriteLock              * lock(void) const;
  HttpServer                  * httpServer(void) const;
  UPnPMediaServer             * mediaServer(void) const;

public:
  static void                   addTextElm(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &value);
  static void                   addSpecVersion(QDomDocument &doc, QDomElement &elm, int major = 1, int minor = 0);
  static void                   addActionArgument(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &direction, const QString &relatedStateVariable);
  static void                   addStateVariable(QDomDocument &doc, QDomElement &elm, bool sendEvents, const QString &name, const QString &dataType, const QStringList &allowedValues = QStringList());

  static QDomElement            makeSoapMessage(QDomDocument &doc);
  static QByteArray             serializeSoapMessage(const QDomDocument &doc);
  static QDomElement            parseSoapMessage(QDomDocument &doc, const QByteArray &data);

public:
  static const char     * const didlNS;
  static const char     * const dublinCoreNS;
  static const char     * const eventNS;
  static const char     * const metadataNS;
  static const char     * const soapNS;

private:
  class EventSession;
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
