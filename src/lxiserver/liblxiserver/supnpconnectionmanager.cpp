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

#include "supnpconnectionmanager.h"
#include "supnpgenaserver.h"

namespace LXiServer {

const char  * const SUPnPConnectionManager::connectionManagerNS = "urn:schemas-upnp-org:service:ConnectionManager:1";

struct SUPnPConnectionManager::Data
{
  SUPnPGenaServer             * genaServer;

  ProtocolList                  sourceProtocols;
  ProtocolList                  sinkProtocols;
};

SUPnPConnectionManager::SUPnPConnectionManager(const QString &basePath, QObject *parent)
  : SUPnPBase(basePath + "connectionmanager/", parent),
    d(new Data())
{
  d->genaServer = new SUPnPGenaServer(SUPnPBase::basePath(), this);
}

SUPnPConnectionManager::~SUPnPConnectionManager()
{
  delete d->genaServer;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SUPnPConnectionManager::initialize(SHttpServer *httpServer, SUPnPMediaServer *mediaServer)
{
  d->genaServer->initialize(httpServer);
  emitEvent(); // To build the initial event message

  SUPnPMediaServer::Service service;
  SUPnPBase::initialize(httpServer, service);
  service.serviceType = connectionManagerNS;
  service.serviceId = "urn:upnp-org:serviceId:ConnectionManager";
  service.eventSubURL = d->genaServer->path();
  mediaServer->registerService(service);
}

void SUPnPConnectionManager::close(void)
{
  SUPnPBase::close();
  d->genaServer->close();
}

void SUPnPConnectionManager::setSourceProtocols(const ProtocolList &sourceProtocols)
{
  QWriteLocker l(lock());
    d->sourceProtocols = sourceProtocols;
  l.unlock();

  emitEvent();
}

void SUPnPConnectionManager::setSinkProtocols(const ProtocolList &sinkProtocols)
{
  QWriteLocker l(lock());
    d->sinkProtocols = sinkProtocols;
  l.unlock();

  emitEvent();
}

void SUPnPConnectionManager::emitEvent(void)
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

QString SUPnPConnectionManager::listSourceProtocols(void) const
{
  QReadLocker l(lock());

  QString result;
  foreach (const Protocol &protocol, d->sourceProtocols)
    result += "," + protocol.toString(true);

  return result.isEmpty() ? result : result.mid(1);
}

QString SUPnPConnectionManager::listSinkProtocols(void) const
{
  QReadLocker l(lock());

  QString result;
  foreach (const Protocol &protocol, d->sinkProtocols)
    result += "," + protocol.toString(true);

  return result.isEmpty() ? result : result.mid(1);
}

void SUPnPConnectionManager::buildDescription(QDomDocument &doc, QDomElement &scpdElm)
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

void SUPnPConnectionManager::handleSoapMessage(const QDomElement &body, QDomDocument &responseDoc, QDomElement &responseBody, const SHttpServer::RequestHeader &, const QHostAddress &)
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
