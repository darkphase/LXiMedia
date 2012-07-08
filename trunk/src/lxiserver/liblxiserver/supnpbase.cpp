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

#include "supnpbase.h"
#include "supnpmediaserver.h"

#ifdef QT_NO_DEBUG
#define PERMISSIVE
#endif

namespace LXiServer {

const int           SUPnPBase::majorVersion     = 1;
const int           SUPnPBase::minorVersion     = 0;
const int           SUPnPBase::responseTimeout  = 30; // Seconds
const char          SUPnPBase::dlnaDoc[]        = "1.50";
const char          SUPnPBase::xmlDeclaration[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
const char          SUPnPBase::dlnaNS[]         = "urn:schemas-dlna-org:metadata-1-0/";
const char          SUPnPBase::didlNS[]         = "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/";
const char          SUPnPBase::dublinCoreNS[]   = "http://purl.org/dc/elements/1.1/";
const char          SUPnPBase::metadataNS[]     = "urn:schemas-upnp-org:metadata-1-0/upnp/";
const char          SUPnPBase::soapNS[]         = "http://schemas.xmlsoap.org/soap/envelope/";

struct SUPnPBase::Data
{
  QString                       basePath;
  QPointer<SHttpServer>         httpServer;
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

void SUPnPBase::reset(void)
{
}

SHttpServer::ResponseMessage SUPnPBase::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if ((request.path() == d->basePath + "control") && request.isPost())
    return handleControl(request, socket);
  else if ((request.path() == d->basePath + "description.xml") && request.isGet())
    return handleDescription(request);

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

SHttpServer::ResponseMessage SUPnPBase::handleControl(const SHttpServer::RequestMessage &request, QIODevice *socket)
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

      const SHttpServer::Status status =
          handleSoapMessage(body, responseDoc, responseBody, request, peerAddress);

      if (status == SHttpServer::Status_Ok)
      {
        SHttpServer::ResponseMessage response(
            request, SHttpServer::Status_Ok,
            serializeSoapMessage(responseDoc), SHttpEngine::mimeTextXml);

        response.setCacheControl(-1);
        return response;
      }
      else
        return SHttpServer::ResponseMessage(request, SHttpServer::Status_Ok);
    }
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

SHttpServer::ResponseMessage SUPnPBase::handleDescription(const SHttpServer::RequestMessage &request)
{
  QDomDocument doc;
  QDomElement scpdElm = doc.createElement("scpd");
  scpdElm.setAttribute("xmlns", "urn:schemas-upnp-org:service-1-0");
  addSpecVersion(doc, scpdElm);

  buildDescription(doc, scpdElm);

  doc.appendChild(scpdElm);

  SHttpServer::ResponseMessage response(
      request, SHttpServer::Status_Ok,
      QByteArray(xmlDeclaration) + doc.toByteArray(-1), SHttpEngine::mimeTextXml);

  response.setCacheControl(-1);
  response.setField("Accept-Ranges", "bytes");
  return response;
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
      "DLNADOC/" + QString(dlnaDoc) + " UPnP/" +
      QString::number(majorVersion) + '.' + QString::number(minorVersion);
}

QString SUPnPBase::toClientString(const QHostAddress &peerAddress, const SHttpServer::RequestMessage &request)
{
  QString userAgent;
  foreach (const QString &tag, request.userAgent().split(' '))
  if (!tag.startsWith("DLNADOC/", Qt::CaseInsensitive) &&
      !tag.startsWith("dma/", Qt::CaseInsensitive) &&
      !tag.startsWith("UPnP/", Qt::CaseInsensitive))
  {
    userAgent.reserve(tag.length());

    for (QString::ConstIterator i=tag.begin(); i!=tag.end(); i++)
    if ((*i == '-') || (*i == '/') || (*i == '.'))
      userAgent += *i;
    else if (i->isLetterOrNumber() || (*i == '-') || (*i == '/') || (*i == '.'))
      userAgent += SStringParser::toBasicLatin(*i);
    else if (!userAgent.isEmpty() && (userAgent[userAgent.length()-1] != ' '))
      userAgent += ' ';

    userAgent = userAgent.trimmed().replace(' ', '_');
    break;
  }

  return userAgent + "@" + peerAddress.toString();
}

QDomElement SUPnPBase::addTextElm(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &value)
{
  QDomElement subElm = doc.createElement(name);
  subElm.appendChild(doc.createTextNode(value));
  elm.appendChild(subElm);

  return subElm;
}

QDomElement SUPnPBase::addTextElmNS(QDomDocument &doc, QDomElement &elm, const QString &name, const QString &nsUri, const QString &value)
{
  QDomElement subElm = doc.createElementNS(nsUri, name);
  subElm.appendChild(doc.createTextNode(value));
  elm.appendChild(subElm);

  return subElm;
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

  QDomElement body = doc.createElement(nsElm.prefix() + ":" + "Body");
  root.appendChild(body);

  return body;
}

QByteArray SUPnPBase::serializeSoapMessage(const QDomDocument &doc)
{
  return xmlDeclaration + doc.toByteArray(-1).replace("&amp;gt;", "&gt;"); // Crude hack for non-compliant XML parsers
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

QByteArray SUPnPBase::Protocol::toByteArray(bool brief) const
{
  QByteArray result = protocol + ":" + network + ":" + contentFormat + ":";

  if (!profile.isEmpty())
    result += brief ? ("DLNA.ORG_PN=" + profile) : contentFeatures();
  else
    result += "*";

  return result;
}

QByteArray SUPnPBase::Protocol::contentFeatures(void) const
{
  QByteArray result;

  if (!profile.isEmpty())
    result += "DLNA.ORG_PN=" + profile + ";";

  if (!contentFormat.startsWith("image/"))
  {
    result += "DLNA.ORG_PS=" + QByteArray::number(playSpeed ? 1 : 0) + ";" +
              "DLNA.ORG_OP=" + QByteArray::number(operationsTimeSeek ? 1 : 0) +
                               QByteArray::number(operationsRange ? 1 : 0) + ";";
  }

  result +=
      "DLNA.ORG_CI=" + QByteArray::number(conversionIndicator ? 1 : 0);

  if (!flags.isEmpty())
    result += ";DLNA.ORG_FLAGS=" + flags + "000000000000000000000000";

  return result;
}

} // End of namespace
