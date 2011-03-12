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

#include "upnpmediaserver.h"
#include <QtXml>
#include "ssdpserver.h"
#include "upnpbase.h"

namespace LXiServer {

const char  * const UPnPMediaServer::dlnaDeviceNS = "urn:schemas-dlna-org:device-1-0";
const char  * const UPnPMediaServer::deviceType   = "urn:schemas-upnp-org:device:MediaServer:1";

struct UPnPMediaServer::Data
{
  struct Icon
  {
    QString                     url;
    QString                     mimetype;
    unsigned                    width;
    unsigned                    height;
    unsigned                    depth;
  };

  inline                        Data(void) : lock(QReadWriteLock::Recursive) { }

  QReadWriteLock                lock;
  QString                       basePath;
  HttpServer                  * httpServer;
  SsdpServer                  * ssdpServer;
  QList<Service>                services;
  QList<Icon>                   icons;
};

UPnPMediaServer::UPnPMediaServer(const QString &basePath, QObject *parent)
    : QObject(parent),
      d(new Data())
{
  d->basePath = basePath;
  d->httpServer = NULL;
  d->ssdpServer = NULL;
}

UPnPMediaServer::~UPnPMediaServer()
{
  if (d->httpServer)
    d->httpServer->unregisterCallback(this);

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void UPnPMediaServer::initialize(HttpServer *httpServer, SsdpServer *ssdpServer)
{
  d->httpServer = httpServer;
  d->ssdpServer = ssdpServer;

  httpServer->registerCallback(d->basePath + "mediaserver/", this);

  ssdpServer->publish("upnp:rootdevice", "/upnp/mediaserver/description.xml", 3);
  ssdpServer->publish("urn:schemas-upnp-org:device:MediaServer:1", "/upnp/mediaserver/description.xml", 2);
}

void UPnPMediaServer::close(void)
{
  QWriteLocker l(&d->lock);

  if (d->httpServer)
    d->httpServer->unregisterCallback(this);
}

void UPnPMediaServer::addIcon(const QString &url, unsigned width, unsigned height, unsigned depth)
{
  Data::Icon icon;
  icon.url = url;
  icon.mimetype = d->httpServer->toMimeType(url);
  icon.width = width;
  icon.height = height;
  icon.depth = depth;

  d->icons += icon;
}

void UPnPMediaServer::registerService(const Service &service)
{
  QWriteLocker l(&d->lock);

  d->services += service;
  d->ssdpServer->publish(service.serviceType, service.descriptionUrl, 1);
}

HttpServer::SocketOp UPnPMediaServer::handleHttpRequest(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  if (request.path() == "/upnp/mediaserver/description.xml")
  {
    QDomDocument doc;
    QDomElement rootElm = doc.createElement("root");
    rootElm.setAttribute("xmlns", "urn:schemas-upnp-org:device-1-0");
    UPnPBase::addSpecVersion(doc, rootElm);

    QDomElement deviceElm = doc.createElement("device");
    UPnPBase::addTextElm(doc, deviceElm, "deviceType", deviceType);
    UPnPBase::addTextElm(doc, deviceElm, "friendlyName", QHostInfo::localHostName() + ": " + qApp->applicationName());
    UPnPBase::addTextElm(doc, deviceElm, "manufacturer", qApp->organizationName());
    UPnPBase::addTextElm(doc, deviceElm, "manufacturerURL", "http://" + qApp->organizationDomain() + "/");
    UPnPBase::addTextElm(doc, deviceElm, "modelDescription", qApp->applicationName());
    UPnPBase::addTextElm(doc, deviceElm, "modelName", qApp->applicationName());
    UPnPBase::addTextElm(doc, deviceElm, "modelNumber", qApp->applicationVersion());
    UPnPBase::addTextElm(doc, deviceElm, "modelURL", "http://" + qApp->organizationDomain() + "/");
    UPnPBase::addTextElm(doc, deviceElm, "serialNumber", qApp->applicationVersion());
    UPnPBase::addTextElm(doc, deviceElm, "UDN", d->httpServer->serverUdn());
    UPnPBase::addTextElmNS(doc, deviceElm, "dlna:X_DLNADOC", dlnaDeviceNS, "DMS-1.50");

    QString host = request.host();
    if (!host.isEmpty())
      UPnPBase::addTextElm(doc, deviceElm, "presentationURL", "http://" + request.host() + "/");

    QReadLocker l(&d->lock);

    if (!d->icons.isEmpty())
    {
      QDomElement iconListElm = doc.createElement("iconList");
      foreach (const Data::Icon &icon, d->icons)
      {
        QDomElement iconElm = doc.createElement("icon");
        UPnPBase::addTextElm(doc, iconElm, "url", icon.url);
        UPnPBase::addTextElm(doc, iconElm, "mimetype", icon.mimetype);
        UPnPBase::addTextElm(doc, iconElm, "width", QString::number(icon.width));
        UPnPBase::addTextElm(doc, iconElm, "height", QString::number(icon.height));
        UPnPBase::addTextElm(doc, iconElm, "depth", QString::number(icon.depth));
        iconListElm.appendChild(iconElm);
      }
      deviceElm.appendChild(iconListElm);
    }

    QDomElement serviceListElm = doc.createElement("serviceList");
    foreach (const Service &service, d->services)
    {
      QDomElement serviceElm = doc.createElement("service");
      UPnPBase::addTextElm(doc, serviceElm, "serviceType", service.serviceType);
      UPnPBase::addTextElm(doc, serviceElm, "serviceId", service.serviceId);
      UPnPBase::addTextElm(doc, serviceElm, "SCPDURL", service.descriptionUrl);
      UPnPBase::addTextElm(doc, serviceElm, "controlURL", service.controlURL);
      UPnPBase::addTextElm(doc, serviceElm, "eventSubURL", service.eventSubURL);
      serviceListElm.appendChild(serviceElm);
    }
    deviceElm.appendChild(serviceListElm);

    l.unlock();

    rootElm.appendChild(deviceElm);
    doc.appendChild(rootElm);

    const QByteArray content = QByteArray(UPnPBase::xmlDeclaration) + '\n' + doc.toByteArray();
    HttpServer::ResponseHeader response(request, HttpServer::Status_Ok);
    response.setContentType(UPnPBase::xmlContentType);
    response.setContentLength(content.length());
    response.setField("Cache-Control", "no-cache");
    response.setField("Accept-Ranges", "bytes");
    response.setField("Connection", "close");
    response.setField("contentFeatures.dlna.org", "");
    socket->write(response);
    socket->write(content);
    return HttpServer::SocketOp_Close;
  }


  return HttpServer::sendResponse(request, socket, HttpServer::Status_NotFound, this);
}

} // End of namespace
