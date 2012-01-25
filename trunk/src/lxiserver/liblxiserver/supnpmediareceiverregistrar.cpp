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

#include "supnpmediareceiverregistrar.h"
#include "supnpgenaserver.h"

namespace LXiServer {

const char  SUPnPMediaReceiverRegistrar::mediaReceiverRegistrarNS[] = "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1";
const char  SUPnPMediaReceiverRegistrar::datatypesNS[] = "urn:schemas-microsoft-com:datatypes";

struct SUPnPMediaReceiverRegistrar::Data
{
  SUPnPGenaServer             * genaServer;

  quint32                       authorizationGrantedUpdateID;
  quint32                       authorizationDeniedUpdateID;
  quint32                       validationSucceededUpdateID;
  quint32                       validationRevokedUpdateID;
};

SUPnPMediaReceiverRegistrar::SUPnPMediaReceiverRegistrar(const QString &basePath, QObject *parent)
  : SUPnPBase(basePath + "mediareceiverregistrar/", parent),
    d(new Data())
{
  d->genaServer = new SUPnPGenaServer(SUPnPBase::basePath(), this);

  d->authorizationGrantedUpdateID = 0;
  d->authorizationDeniedUpdateID = 0;
  d->validationSucceededUpdateID = 0;
  d->validationRevokedUpdateID = 0;
}

SUPnPMediaReceiverRegistrar::~SUPnPMediaReceiverRegistrar()
{
  delete d->genaServer;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SUPnPMediaReceiverRegistrar::initialize(SHttpServer *httpServer, SUPnPMediaServer *mediaServer)
{
  d->genaServer->initialize(httpServer);
  emitEvent(); // To build the initial event message

  SUPnPMediaServer::Service service;
  SUPnPBase::initialize(httpServer, service);
  service.serviceType = mediaReceiverRegistrarNS;
  service.serviceId = "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar";
  service.eventSubURL = d->genaServer->path();
  mediaServer->registerService(service);
}

void SUPnPMediaReceiverRegistrar::close(void)
{
  SUPnPBase::close();
  d->genaServer->close();
}

void SUPnPMediaReceiverRegistrar::emitEvent(void)
{
  QDomDocument doc;
  QDomElement propertySet = doc.createElementNS(d->genaServer->eventNS, "e:propertyset");

  QDomElement property = createElementNS(doc, propertySet, "property");
  addTextElm(doc, property, "AuthorizationGrantedUpdateID", QString::number(d->authorizationGrantedUpdateID));
  propertySet.appendChild(property);

  property = createElementNS(doc, propertySet, "property");
  addTextElm(doc, property, "AuthorizationDeniedUpdateID", QString::number(d->authorizationDeniedUpdateID));
  propertySet.appendChild(property);

  property = createElementNS(doc, propertySet, "property");
  addTextElm(doc, property, "ValidationSucceededUpdateID", QString::number(d->validationSucceededUpdateID));
  propertySet.appendChild(property);

  property = createElementNS(doc, propertySet, "property");
  addTextElm(doc, property, "ValidationRevokedUpdateID", QString::number(d->validationRevokedUpdateID));
  propertySet.appendChild(property);

  doc.appendChild(propertySet);

  d->genaServer->emitEvent(doc);
}

void SUPnPMediaReceiverRegistrar::buildDescription(QDomDocument &doc, QDomElement &scpdElm)
{
  QDomElement actionListElm = doc.createElement("actionList");
  {
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "IsAuthorized");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "DeviceID", "in", "A_ARG_TYPE_DeviceID");
    addActionArgument(doc, argumentListElm, "Result", "out", "A_ARG_TYPE_Result");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }
  {
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "IsValidated");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "DeviceID", "in", "A_ARG_TYPE_DeviceID");
    addActionArgument(doc, argumentListElm, "Result", "out", "A_ARG_TYPE_Result");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }
  /*{
    QDomElement actionElm = doc.createElement("action");
    addTextElm(doc, actionElm, "name", "RegisterDevice");
    QDomElement argumentListElm = doc.createElement("argumentList");
    addActionArgument(doc, argumentListElm, "RegistrationReqMsg", "in", "A_ARG_TYPE_RegistrationReqMsg");
    addActionArgument(doc, argumentListElm, "RegistrationRespMsg", "out", "A_ARG_TYPE_RegistrationRespMsg");
    actionElm.appendChild(argumentListElm);
    actionListElm.appendChild(actionElm);
  }*/
  scpdElm.appendChild(actionListElm);

  QDomElement serviceStateTableElm = doc.createElement("serviceStateTable");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_DeviceID", "string");
  addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_Result", "int");
  //addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_RegistrationReqMsg", "bin.base64");
  //addStateVariable(doc, serviceStateTableElm, false, "A_ARG_TYPE_RegistrationRespMsg", "bin.base64");
  addStateVariable(doc, serviceStateTableElm, true, "AuthorizationGrantedUpdateID", "ui4");
  addStateVariable(doc, serviceStateTableElm, true, "AuthorizationDeniedUpdateID", "ui4");
  addStateVariable(doc, serviceStateTableElm, true, "ValidationSucceededUpdateID", "ui4");
  addStateVariable(doc, serviceStateTableElm, true, "ValidationRevokedUpdateID", "ui4");
  scpdElm.appendChild(serviceStateTableElm);
}

SHttpServer::Status SUPnPMediaReceiverRegistrar::handleSoapMessage(const QDomElement &body, QDomDocument &responseDoc, QDomElement &responseBody, const SHttpServer::RequestMessage &, const QHostAddress &)
{
  SHttpServer::Status status = SHttpServer::Status(402, "Invalid args");

  const QDomElement isAuthorizedElem = firstChildElementNS(body, mediaReceiverRegistrarNS, "IsAuthorized");
  if (!isAuthorizedElem.isNull())
  {
    QDomElement responseElm = createElementNS(responseDoc, isAuthorizedElem, "IsAuthorizedResponse");
    QDomElement resultElm = addTextElm(responseDoc, responseElm, "Result", "1");
    resultElm.setAttribute("xmlns:dt", datatypesNS);
    resultElm.setAttribute("dt:dt", "int");
    responseBody.appendChild(responseElm);
    status = SHttpServer::Status_Ok;
  }

  const QDomElement isValidatedElem = firstChildElementNS(body, mediaReceiverRegistrarNS, "IsValidated");
  if (!isValidatedElem.isNull())
  {
    QDomElement responseElm = createElementNS(responseDoc, isValidatedElem, "IsValidatedResponse");
    QDomElement resultElm = addTextElm(responseDoc, responseElm, "Result", "1");
    resultElm.setAttribute("xmlns:dt", datatypesNS);
    resultElm.setAttribute("dt:dt", "int");
    responseBody.appendChild(responseElm);
    status = SHttpServer::Status_Ok;
  }

  return status;
}

} // End of namespace
