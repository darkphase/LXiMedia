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

#include "supnpmediaserver.h"
#include <QtXml>
#include "sssdpserver.h"
#include "supnpbase.h"

namespace LXiServer {

const char  * const SUPnPMediaServer::dlnaDeviceNS = "urn:schemas-dlna-org:device-1-0";
const char  * const SUPnPMediaServer::deviceType   = "urn:schemas-upnp-org:device:MediaServer:1";

struct SUPnPMediaServer::Data
{
  struct Icon
  {
    QString                     url;
    QString                     mimetype;
    unsigned                    width;
    unsigned                    height;
    unsigned                    depth;
  };

  QString                       basePath;
  SHttpServer                 * httpServer;
  SSsdpServer                  * ssdpServer;
  QList<Service>                services;
  QList<Icon>                   icons;
};

SUPnPMediaServer::SUPnPMediaServer(const QString &basePath, QObject *parent)
    : QObject(parent),
      d(new Data())
{
  d->basePath = basePath;
  d->httpServer = NULL;
  d->ssdpServer = NULL;
}

SUPnPMediaServer::~SUPnPMediaServer()
{
  if (d->httpServer)
    d->httpServer->unregisterCallback(this);

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SUPnPMediaServer::initialize(SHttpServer *httpServer, SSsdpServer *ssdpServer)
{
  d->httpServer = httpServer;
  d->ssdpServer = ssdpServer;

  httpServer->registerCallback(d->basePath + "mediaserver/", this);

  ssdpServer->publish("upnp:rootdevice", "/upnp/mediaserver/description.xml", 3);
  ssdpServer->publish("urn:schemas-upnp-org:device:MediaServer:1", "/upnp/mediaserver/description.xml", 2);
}

void SUPnPMediaServer::close(void)
{
  if (d->httpServer)
    d->httpServer->unregisterCallback(this);
}

void SUPnPMediaServer::addIcon(const QString &url, unsigned width, unsigned height, unsigned depth)
{
  Data::Icon icon;
  icon.url = url;
  icon.mimetype = d->httpServer->toMimeType(url);
  icon.width = width;
  icon.height = height;
  icon.depth = depth;

  d->icons += icon;
}

void SUPnPMediaServer::registerService(const Service &service)
{
  d->services += service;
  d->ssdpServer->publish(service.serviceType, service.descriptionUrl, 1);
}

SHttpServer::SocketOp SUPnPMediaServer::handleHttpRequest(const SHttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  if (request.path() == "/upnp/mediaserver/description.xml")
  {
    QDomDocument doc;
    QDomElement rootElm = doc.createElement("root");
    rootElm.setAttribute("xmlns", "urn:schemas-upnp-org:device-1-0");
    SUPnPBase::addSpecVersion(doc, rootElm);

    QDomElement deviceElm = doc.createElement("device");
    SUPnPBase::addTextElm(doc, deviceElm, "deviceType", deviceType);
    SUPnPBase::addTextElm(doc, deviceElm, "friendlyName", QHostInfo::localHostName() + ": " + qApp->applicationName());
    SUPnPBase::addTextElm(doc, deviceElm, "manufacturer", qApp->organizationName());
    SUPnPBase::addTextElm(doc, deviceElm, "manufacturerURL", "http://" + qApp->organizationDomain() + "/");
    SUPnPBase::addTextElm(doc, deviceElm, "modelDescription", qApp->applicationName());
    SUPnPBase::addTextElm(doc, deviceElm, "modelName", qApp->applicationName());
    SUPnPBase::addTextElm(doc, deviceElm, "modelNumber", qApp->applicationVersion());
    SUPnPBase::addTextElm(doc, deviceElm, "modelURL", "http://" + qApp->organizationDomain() + "/");
    SUPnPBase::addTextElm(doc, deviceElm, "serialNumber", qApp->applicationVersion());
    SUPnPBase::addTextElm(doc, deviceElm, "UDN", d->httpServer->serverUdn());
    SUPnPBase::addTextElmNS(doc, deviceElm, "dlna:X_DLNADOC", dlnaDeviceNS, "DMS-1.00");

    QString host = request.host();
    if (!host.isEmpty())
      SUPnPBase::addTextElm(doc, deviceElm, "presentationURL", "http://" + request.host() + "/");

    if (!d->icons.isEmpty())
    {
      QDomElement iconListElm = doc.createElement("iconList");
      foreach (const Data::Icon &icon, d->icons)
      {
        QDomElement iconElm = doc.createElement("icon");
        SUPnPBase::addTextElm(doc, iconElm, "url", icon.url);
        SUPnPBase::addTextElm(doc, iconElm, "mimetype", icon.mimetype);
        SUPnPBase::addTextElm(doc, iconElm, "width", QString::number(icon.width));
        SUPnPBase::addTextElm(doc, iconElm, "height", QString::number(icon.height));
        SUPnPBase::addTextElm(doc, iconElm, "depth", QString::number(icon.depth));
        iconListElm.appendChild(iconElm);
      }
      deviceElm.appendChild(iconListElm);
    }

    QDomElement serviceListElm = doc.createElement("serviceList");
    foreach (const Service &service, d->services)
    {
      QDomElement serviceElm = doc.createElement("service");
      SUPnPBase::addTextElm(doc, serviceElm, "serviceType", service.serviceType);
      SUPnPBase::addTextElm(doc, serviceElm, "serviceId", service.serviceId);
      SUPnPBase::addTextElm(doc, serviceElm, "SCPDURL", service.descriptionUrl);
      SUPnPBase::addTextElm(doc, serviceElm, "controlURL", service.controlURL);
      SUPnPBase::addTextElm(doc, serviceElm, "eventSubURL", service.eventSubURL);
      serviceListElm.appendChild(serviceElm);
    }
    deviceElm.appendChild(serviceListElm);

    rootElm.appendChild(deviceElm);
    doc.appendChild(rootElm);

    const QByteArray content = QByteArray(SUPnPBase::xmlDeclaration) + '\n' + doc.toByteArray();
    SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
    response.setContentType(SUPnPBase::xmlContentType);
    response.setContentLength(content.length());
    response.setField("Cache-Control", "no-cache");
    response.setField("Accept-Ranges", "bytes");
    response.setField("Connection", "close");
    response.setField("contentFeatures.dlna.org", "");
    socket->write(response);
    socket->write(content);
    return SHttpServer::SocketOp_Close;
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

} // End of namespace
