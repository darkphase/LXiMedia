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

const char  SUPnPMediaServer::dlnaDeviceNS[] = "urn:schemas-dlna-org:device-1-0";
const char  SUPnPMediaServer::deviceType[]   = "urn:schemas-upnp-org:device:MediaServer:1";

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
  SSsdpServer                 * ssdpServer;
  QList<Service>                services;
  QList<Icon>                   icons;
  QString                       deviceName;
};

SUPnPMediaServer::SUPnPMediaServer(const QString &basePath, QObject *parent)
    : QObject(parent),
      d(new Data())
{
  d->basePath = basePath + "mediaserver/";
  d->httpServer = NULL;
  d->ssdpServer = NULL;
  d->deviceName = QHostInfo::localHostName() + ": " + qApp->applicationName();
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

  httpServer->registerCallback(d->basePath, this);

  ssdpServer->publish("upnp:rootdevice", d->basePath + "description.xml", 3);
  ssdpServer->publish("urn:schemas-upnp-org:device:MediaServer:1", d->basePath + "description.xml", 2);
}

void SUPnPMediaServer::close(void)
{
  if (d->httpServer)
    d->httpServer->unregisterCallback(this);
}

void SUPnPMediaServer::reset(void)
{
}

void SUPnPMediaServer::setDeviceName(const QString &deviceName)
{
  d->deviceName = deviceName;
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
  d->ssdpServer->publish(service.serviceType, d->basePath + "description.xml", 1);
}

SHttpServer::ResponseMessage SUPnPMediaServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *)
{
  if (request.isGet())
  {
    if (request.path() == (d->basePath + "description.xml"))
    {
      QDomDocument doc;
      QDomElement rootElm = doc.createElement("root");
      rootElm.setAttribute("xmlns", "urn:schemas-upnp-org:device-1-0");
      SUPnPBase::addSpecVersion(doc, rootElm);

      const QString host = request.host();
      if (!host.isEmpty())
        SUPnPBase::addTextElm(doc, rootElm, "URLBase", "http://" + host + "/");

      QDomElement deviceElm = doc.createElement("device");
      SUPnPBase::addTextElm(doc, deviceElm, "deviceType", deviceType);
      SUPnPBase::addTextElm(doc, deviceElm, "friendlyName", d->deviceName);
      SUPnPBase::addTextElm(doc, deviceElm, "manufacturer", qApp->organizationName());
      SUPnPBase::addTextElm(doc, deviceElm, "manufacturerURL", "http://" + qApp->organizationDomain() + "/");
      SUPnPBase::addTextElm(doc, deviceElm, "modelDescription", qApp->applicationName());
      SUPnPBase::addTextElm(doc, deviceElm, "modelName", qApp->applicationName());
      SUPnPBase::addTextElm(doc, deviceElm, "modelNumber", qApp->applicationVersion());
      SUPnPBase::addTextElm(doc, deviceElm, "modelURL", "http://" + qApp->organizationDomain() + "/");
      SUPnPBase::addTextElm(doc, deviceElm, "serialNumber", qApp->applicationVersion());
      SUPnPBase::addTextElm(doc, deviceElm, "UDN", d->httpServer->serverUdn());
      SUPnPBase::addTextElmNS(doc, deviceElm, "dlna:X_DLNADOC", dlnaDeviceNS, "DMS-" + QString(SUPnPBase::dlnaDoc));

      if (!host.isEmpty())
        SUPnPBase::addTextElm(doc, deviceElm, "presentationURL", "http://" + host + "/");

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

      SHttpServer::ResponseMessage response(
          request, SHttpServer::Status_Ok,
          SUPnPBase::xmlDeclaration + doc.toByteArray(-1), SHttpEngine::mimeTextXml);

      response.setCacheControl(-1);
      response.setField("Accept-Ranges", "bytes");
      return response;
    }
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

} // End of namespace
