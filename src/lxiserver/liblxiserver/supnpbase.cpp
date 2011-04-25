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

#include "supnpbase.h"
#include "supnpmediaserver.h"

#ifdef QT_NO_DEBUG
#define PERMISSIVE
#endif

namespace LXiServer {

const int           SUPnPBase::majorVersion = 1, SUPnPBase::minorVersion = 0;
const int           SUPnPBase::responseTimeout  = 30; // Seconds
const char          SUPnPBase::xmlDeclaration[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
const char          SUPnPBase::xmlContentType[] = "text/xml; charset=\"utf-8\" ";
const char          SUPnPBase::dlnaNS[]         = "urn:schemas-dlna-org:metadata-1-0/";
const char          SUPnPBase::didlNS[]         = "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/";
const char          SUPnPBase::dublinCoreNS[]   = "http://purl.org/dc/elements/1.1/";
const char          SUPnPBase::metadataNS[]     = "urn:schemas-upnp-org:metadata-1-0/upnp/";
const char          SUPnPBase::soapNS[]         = "http://schemas.xmlsoap.org/soap/envelope/";

struct SUPnPBase::Data
{
  QString                       basePath;
  SHttpServer                  * httpServer;
};

SUPnPBase::SUPnPBase(const QString &basePath, QObject *parent)
  : QObject(parent),
    d(new Data())
{
  d->basePath = basePath;
  d->httpServer = NULL;
}

SUPnPBase::~SUPnPBase()
{
  if (d->httpServer)
    d->httpServer->unregisterCallback(this);

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SUPnPBase::initialize(SHttpServer *httpServer, SUPnPMediaServer::Service &service)
{
  d->httpServer = httpServer;

  httpServer->registerCallback(d->basePath, this);

  service.descriptionUrl = d->basePath + "description.xml";
  service.controlURL = d->basePath + "control";
}

void SUPnPBase::close(void)
{
  if (d->httpServer)
    d->httpServer->unregisterCallback(this);

  d->httpServer = NULL;
}

SHttpServer::SocketOp SUPnPBase::handleHttpRequest(const SHttpServer::RequestMessage &request, QAbstractSocket *socket)
{
  if ((request.path() == d->basePath + "control") && (request.method() == "POST"))
    return handleControl(request, socket);
  else if ((request.path() == d->basePath + "description.xml") && ((request.method() == "GET") || (request.method() == "HEAD")))
    return handleDescription(request, socket);

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

void SUPnPBase::handleHttpOptions(SHttpServer::ResponseHeader &response)
{
  response.setField("Allow", response.field("Allow") + ",GET,HEAD,POST");
}

SHttpServer::SocketOp SUPnPBase::handleControl(const SHttpServer::RequestMessage &request, QAbstractSocket *socket)
{
  QTime timer;
  timer.start();

  if (!request.content().isEmpty())
  {
    QDomDocument doc;
    const QDomElement body = parseSoapMessage(doc, request.content());
    if (!body.isNull())
    {
      QDomDocument responseDoc;
      QDomElement responseBody = makeSoapMessage(responseDoc, body);

      QHostAddress peerAddress;
      QAbstractSocket * const abstractSocket = qobject_cast<QAbstractSocket *>(socket);
      if (abstractSocket)
        peerAddress = abstractSocket->peerAddress();

      handleSoapMessage(body, responseDoc, responseBody, request, peerAddress);

      const QByteArray content = serializeSoapMessage(responseDoc);
      SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
      response.setContentType(xmlContentType);
      response.setContentLength(content.length());
      response.setField("Cache-Control", "no-cache");
      response.setField("Accept-Ranges", "bytes");
      response.setField("Connection", "close");
      response.setField("contentFeatures.dlna.org", "");
      socket->write(response);
      socket->write(content);

      return SHttpServer::SocketOp_Close;
    }
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

SHttpServer::SocketOp SUPnPBase::handleDescription(const SHttpServer::RequestMessage &request, QAbstractSocket *socket)
{
  QDomDocument doc;
  QDomElement scpdElm = doc.createElement("scpd");
  scpdElm.setAttribute("xmlns", "urn:schemas-upnp-org:service-1-0");
  addSpecVersion(doc, scpdElm);

  buildDescription(doc, scpdElm);

  doc.appendChild(scpdElm);

  const QByteArray content = QByteArray(xmlDeclaration) + '\n' + doc.toByteArray();
  SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
  response.setContentType(xmlContentType);
  response.setContentLength(content.length());
  response.setField("Cache-Control", "no-cache");
  response.setField("Accept-Ranges", "bytes");
  response.setField("Connection", "close");
  response.setField("contentFeatures.dlna.org", "");
  socket->write(response);
  socket->write(content);

  return SHttpServer::SocketOp_Close;
}

const QString & SUPnPBase::basePath(void) const
{
  return d->basePath;
}

SHttpServer * SUPnPBase::httpServer(void) const
{
  return d->httpServer;
}

QString SUPnPBase::protocol(void)
{
  return
      "UPnP/" + QString::number(majorVersion) + "." + QString::number(minorVersion) +
      " DLNADOC/1.00";
}

void SUPnPBase::addTextElm(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &value)
{
  QDomElement subElm = doc.createElement(name);
  subElm.appendChild(doc.createTextNode(value));
  elm.appendChild(subElm);
}

void SUPnPBase::addTextElmNS(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &nsUri, const QString &value)
{
  QDomElement subElm = doc.createElementNS(nsUri, name);
  subElm.appendChild(doc.createTextNode(value));
  elm.appendChild(subElm);
}

void SUPnPBase::addSpecVersion(QDomDocument &doc, QDomElement &elm)
{
  QDomElement specVersionElm = doc.createElement("specVersion");
  addTextElm(doc, specVersionElm, "major", QString::number(majorVersion));
  addTextElm(doc, specVersionElm, "minor", QString::number(minorVersion));
  elm.appendChild(specVersionElm);
}

void SUPnPBase::addActionArgument(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &direction, const QString &relatedStateVariable)
{
  QDomElement argumentElm = doc.createElement("argument");
  addTextElm(doc, argumentElm, "name", name);
  addTextElm(doc, argumentElm, "direction", direction);
  addTextElm(doc, argumentElm, "relatedStateVariable", relatedStateVariable);
  elm.appendChild(argumentElm);
}

void SUPnPBase::addStateVariable(QDomDocument &doc, QDomElement &elm, bool sendEvents, const QString &name, const QString &dataType, const QStringList &allowedValues)
{
  QDomElement stateVariableElm = doc.createElement("stateVariable");
  stateVariableElm.setAttribute("sendEvents", sendEvents ? "yes" : "no");
  addTextElm(doc, stateVariableElm, "name", name);
  addTextElm(doc, stateVariableElm, "dataType", dataType);

  if (!allowedValues.isEmpty())
  {
    QDomElement allowedValueListElm = doc.createElement("allowedValueList");

    foreach (const QString &allowedValue, allowedValues)
      addTextElm(doc, allowedValueListElm, "allowedValue", allowedValue);

    stateVariableElm.appendChild(allowedValueListElm);
  }

  elm.appendChild(stateVariableElm);
}

QDomElement SUPnPBase::createElementNS(QDomDocument &doc, const QDomElement &nsElm, const QString &localName)
{
  if (!nsElm.isNull())
  if (!nsElm.prefix().isEmpty() && !nsElm.namespaceURI().isEmpty())
    return doc.createElementNS(nsElm.namespaceURI(), nsElm.prefix() + ":" + localName);

  return doc.createElement(localName);
}

QDomElement SUPnPBase::makeSoapMessage(QDomDocument &doc, const QDomElement &nsElm)
{
  QDomElement root = createElementNS(doc, nsElm, "Envelope");

  if (!nsElm.isNull() && !nsElm.prefix().isEmpty() && !nsElm.namespaceURI().isEmpty())
    root.setAttributeNS(nsElm.namespaceURI(), nsElm.prefix() + ":" + "encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");
  else
    root.setAttribute("encodingStyle", "http://schemas.xmlsoap.org/soap/encoding/");

  doc.appendChild(root);

  QDomElement body = createElementNS(doc, nsElm, "Body");
  root.appendChild(body);

  return body;
}

QByteArray SUPnPBase::serializeSoapMessage(const QDomDocument &doc)
{
  return xmlDeclaration + doc.toByteArray(-1);
}

QDomElement SUPnPBase::firstChildElementNS(const QDomElement &elm, const QString &nsURI, const QString &localName)
{
  for (QDomNode i=elm.firstChild(); !i.isNull(); i=i.nextSibling())
  if ((i.localName() == localName) && (i.namespaceURI() == nsURI) && !i.toElement().isNull())
    return i.toElement();

#ifdef PERMISSIVE
  // Ok, none found, check if an element with the same localname but a
  // different namespace is present.
  for (QDomNode i=elm.firstChild(); !i.isNull(); i=i.nextSibling())
  if ((i.localName() == localName) && !i.toElement().isNull())
    return i.toElement();
#endif

  return QDomElement();
}

QDomElement SUPnPBase::parseSoapMessage(QDomDocument &doc, const QByteArray &data)
{
  doc = QDomDocument("Envelope");
  if (doc.setContent(data, true))
    return firstChildElementNS(doc.documentElement(), soapNS, "Body");

  return QDomElement();
}

QString SUPnPBase::Protocol::toString(bool brief) const
{
  QString result = protocol + ":" + network + ":" + contentFormat + ":";
  if (!brief)
  {
    if (!profile.isEmpty())
      result += profile + ";";

    result +=
        "DLNA.ORG_PS=" + QString::number(playSpeed ? 1 : 0) + ";"
        "DLNA.ORG_CI=" + QString::number(conversionIndicator ? 1 : 0) + ";"
        "DLNA.ORG_OP=" + QString::number(operationsTimeSeek ? 1 : 0) +
                         QString::number(operationsRange ? 1 : 0);

    if (!flags.isEmpty())
      result += ";DLNA.ORG_FLAGS=" + flags;
  }
  else
    result += !profile.isEmpty() ? profile : "*";

  return result;
}

} // End of namespace
