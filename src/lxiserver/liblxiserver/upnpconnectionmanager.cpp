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

#include "upnpconnectionmanager.h"
#include "upnpgenaserver.h"

namespace LXiServer {

const char  * const UPnPConnectionManager::connectionManagerNS = "urn:schemas-upnp-org:service:ConnectionManager:1";

struct UPnPConnectionManager::Data
{
  UPnPGenaServer              * genaServer;

  ProtocolList                  sourceProtocols;
  ProtocolList                  sinkProtocols;
};

UPnPConnectionManager::UPnPConnectionManager(const QString &basePath, QObject *parent)
  : UPnPBase(basePath + "connectionmanager/", parent),
    d(new Data())
{
  d->genaServer = new UPnPGenaServer(UPnPBase::basePath(), this);
}

UPnPConnectionManager::~UPnPConnectionManager()
{
  delete d->genaServer;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void UPnPConnectionManager::initialize(HttpServer *httpServer, UPnPMediaServer *mediaServer)
{
  d->genaServer->initialize(httpServer);
  emitEvent(); // To build the initial event message

  UPnPMediaServer::Service service;
  UPnPBase::initialize(httpServer, service);
  service.serviceType = connectionManagerNS;
  service.serviceId = "urn:upnp-org:serviceId:ConnectionManager";
  service.eventSubURL = d->genaServer->path();
  mediaServer->registerService(service);
}

void UPnPConnectionManager::close(void)
{
  UPnPBase::close();
  d->genaServer->close();
}

void UPnPConnectionManager::setSourceProtocols(const ProtocolList &sourceProtocols)
{
  QWriteLocker l(lock());
    d->sourceProtocols = sourceProtocols;
  l.unlock();

  emitEvent();
}

void UPnPConnectionManager::setSinkProtocols(const ProtocolList &sinkProtocols)
{
  QWriteLocker l(lock());
    d->sinkProtocols = sinkProtocols;
  l.unlock();

  emitEvent();
}

void UPnPConnectionManager::emitEvent(void)
{
  QDomDocument doc;
  QDomElement propertySet = doc.createElementNS(d->genaServer->eventNS, "e:propertyset");

  QDomElement property = createElementNS(doc, propertySet, "property");
  addTextElm(doc, property, "SourceProtocolInfo", listSourceProtocols());
  propertySet.appendChild(property);

  property = createElementNS(doc, propertySet, "property");
  addTextElm(doc, property, "SinkProtocolInfo", listSinkProtocols());
  propertySet.appendChild(property);

  property = createElementNS(doc, propertySet, "property");
  addTextElm(doc, property, "CurrentConnectionIDs", QString::null);
  propertySet.appendChild(property);

  doc.appendChild(propertySet);

  d->genaServer->emitEvent(doc);
}

QString UPnPConnectionManager::listSourceProtocols(void) const
{
  QReadLocker l(lock());

  QString result;
  foreach (const Protocol &protocol, d->sourceProtocols)
    result += "," + protocol.toString(true);

  return result.isEmpty() ? result : result.mid(1);
}

QString UPnPConnectionManager::listSinkProtocols(void) const
{
  QReadLocker l(lock());

  QString result;
  foreach (const Protocol &protocol, d->sinkProtocols)
    result += "," + protocol.toString(true);

  return result.isEmpty() ? result : result.mid(1);
}

void UPnPConnectionManager::buildDescription(QDomDocument &doc, QDomElement &scpdElm)
{
  QDomElement actionListElm = doc.createElement("actionList");
  {
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "GetProtocolInfo");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "Source", "out", "SourceProtocolInfo");
    addActionArgument(doc, argumentListElm, "Sink", "out", "SinkProtocolInfo");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }
  {
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "GetCurrentConnectionIDs");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "ConnectionIDs", "out", "CurrentConnectionIDs");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }
  {
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "GetCurrentConnectionInfo");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "ConnectionID", "in", "A_ARG_TYPE_ConnectionID");
    addActionArgument(doc, argumentListElm, "RcsID", "out", "A_ARG_TYPE_RcsID");
    addActionArgument(doc, argumentListElm, "AVTransportID", "out", "A_ARG_TYPE_AVTransportID");
    addActionArgument(doc, argumentListElm, "ProtocolInfo", "out", "A_ARG_TYPE_ProtocolInfo");
    addActionArgument(doc, argumentListElm, "PeerConnectionManager", "out", "A_ARG_TYPE_ConnectionManager");
    addActionArgument(doc, argumentListElm, "PeerConnectionID", "out", "A_ARG_TYPE_ConnectionID");
    addActionArgument(doc, argumentListElm, "Direction", "out", "A_ARG_TYPE_Direction");
    addActionArgument(doc, argumentListElm, "Status", "out", "A_ARG_TYPE_ConnectionStatus");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }
  scpdElm.appendChild(actionListElm);

  QDomElement serviceStateTableElm = doc.createElement("serviceStateTable");
  addStateVariable(doc, serviceStateTableElm, true, "SourceProtocolInfo", "string");
  addStateVariable(doc, serviceStateTableElm, true, "SinkProtocolInfo", "string");
  addStateVariable(doc, serviceStateTableElm, true, "CurrentConnectionIDs", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_ConnectionStatus", "string", QStringList() << "OK" << "ContentFormatMismatch" << "InsufficientBandwidth" << "UnreliableChannel" << "Unknown");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_ConnectionManager", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Direction", "string", QStringList() << "Input" << "Output");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_ProtocolInfo", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_ConnectionID", "i4");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_AVTransportID", "i4");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_RcsID", "i4");
  scpdElm.appendChild(serviceStateTableElm);
}

void UPnPConnectionManager::handleSoapMessage(const QDomElement &body, QDomDocument &responseDoc, QDomElement &responseBody, const HttpServer::RequestHeader &, const QHostAddress &)
{
  const QDomElement getProtocolInfoElem = firstChildElementNS(body, connectionManagerNS, "GetProtocolInfo");
  if (!getProtocolInfoElem.isNull())
  {
    QDomElement response = createElementNS(responseDoc, getProtocolInfoElem, "GetProtocolInfoResponse");
    addTextElm(responseDoc, response, "Source", listSourceProtocols());
    addTextElm(responseDoc, response, "Sink", listSinkProtocols());
    responseBody.appendChild(response);
  }

  const QDomElement getCurrentConnectionIDsElem = firstChildElementNS(body, connectionManagerNS, "GetCurrentConnectionIDs");
  if (!getCurrentConnectionIDsElem.isNull())
  {
    QDomElement response = createElementNS(responseDoc, getCurrentConnectionIDsElem, "GetCurrentConnectionIDsResponse");
    addTextElm(responseDoc, response, "ConnectionIDs", QString::null);
    responseBody.appendChild(response);
  }
}

} // End of namespace
