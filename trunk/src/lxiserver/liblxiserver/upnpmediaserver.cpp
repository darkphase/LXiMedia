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
#include "ssdpserver.h"

namespace LXiServer {

const char  * const UPnPMediaServer::deviceType = "urn:schemas-upnp-org:device:MediaServer:1";

struct UPnPMediaServer::Data
{
  inline                        Data(void) : lock(QReadWriteLock::Recursive) { }

  QReadWriteLock                lock;
  HttpServer                  * httpServer;
  SsdpServer                  * ssdpServer;
  QList<Service>                services;
};

UPnPMediaServer::UPnPMediaServer(QObject *parent)
    : QObject(parent),
      d(new Data())
{
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

  httpServer->registerCallback("/upnp/mediaserver/", this);

  ssdpServer->publish("upnp:rootdevice", httpServer, "/upnp/mediaserver/description.xml");
  ssdpServer->publish("urn:schemas-upnp-org:device:MediaServer:1", httpServer, "/upnp/mediaserver/description.xml");
}

void UPnPMediaServer::close(void)
{
  QWriteLocker l(&d->lock);

  if (d->httpServer)
    d->httpServer->unregisterCallback(this);
}

const QString & UPnPMediaServer::serverId(void) const
{
  return d->ssdpServer->serverId();
}

const QUuid & UPnPMediaServer::serverUuid(void) const
{
  return d->ssdpServer->serverUuid();
}

void UPnPMediaServer::registerService(const Service &service)
{
  QWriteLocker l(&d->lock);

  d->services += service;
  d->ssdpServer->publish(service.serviceType, d->httpServer, service.descriptionUrl);
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
    UPnPBase::addTextElm(doc, deviceElm, "friendlyName", QHostInfo::localHostName() + ", " + qApp->applicationName());
    UPnPBase::addTextElm(doc, deviceElm, "manufacturer", qApp->organizationName());
    UPnPBase::addTextElm(doc, deviceElm, "manufacturerURL", qApp->organizationDomain());
    UPnPBase::addTextElm(doc, deviceElm, "modelDescription", qApp->applicationName());
    UPnPBase::addTextElm(doc, deviceElm, "modelName", qApp->applicationName());
    UPnPBase::addTextElm(doc, deviceElm, "modelNumber", qApp->applicationVersion());
    UPnPBase::addTextElm(doc, deviceElm, "modelURL", qApp->organizationDomain());
    UPnPBase::addTextElm(doc, deviceElm, "serialNumber", qApp->applicationVersion());
    UPnPBase::addTextElm(doc, deviceElm, "UDN", "uuid:" + serverUuid());
    UPnPBase::addTextElm(doc, deviceElm, "UPC", "");

    QReadLocker l(&d->lock);

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

    const QByteArray content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + doc.toByteArray();
    HttpServer::ResponseHeader response(HttpServer::Status_Ok);
    response.setContentType("text/xml;charset=utf-8");
    response.setContentLength(content.length());
    response.setField("Cache-Control", "no-cache");
    response.setField("Accept-Ranges", "bytes");
    response.setField("Connection", "close");
    response.setField("contentFeatures.dlna.org", "");
    response.setField("Server", serverId());
    socket->write(response);
    socket->write(content);
    return HttpServer::SocketOp_Close;
  }

  qWarning() << "UPnPMediaServer: Could not handle request:" << request.method() << request.path();
  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}

} // End of namespace
