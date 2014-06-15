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

#include <upnp/upnp.h>
#include "rootdevice.h"
#include "ixmlstructures.h"

namespace LXiMediaCenter {

struct RootDevice::Data
{
  static const char deviceDescriptionFile[];
  static const char serviceDescriptionFile[];
  static const char serviceControlFile[];
  static const char serviceEventFile[];

  UPnP *upnp;
  QUuid uuid;
  QByteArray deviceType;
  QString deviceName;
  QStringList icons;

  QSet<QByteArray> serviceExts;
  QMap<QByteArray, QPair<Service *, QByteArray> > services;

  bool rootDeviceRegistred;
  QMap<QByteArray, ::UpnpDevice_Handle> rootDeviceHandle;

  QTimer initialAdvertisementTimer;
  static const int advertisementDelay = 3; // Seconds
  static const int advertisementExpiration = 1800; // Seconds

  QByteArray baseDir;
};

const char  RootDevice::serviceTypeConnectionManager[]      = "urn:schemas-upnp-org:service:ConnectionManager:1";
const char  RootDevice::serviceTypeContentDirectory[]       = "urn:schemas-upnp-org:service:ContentDirectory:1";
const char  RootDevice::serviceTypeMediaReceiverRegistrar[] = "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1";

const char  RootDevice::Data::deviceDescriptionFile[]       = "device";
const char  RootDevice::Data::serviceDescriptionFile[]      = "service-";
const char  RootDevice::Data::serviceControlFile[]          = "control-";
const char  RootDevice::Data::serviceEventFile[]            = "event-";

RootDevice::RootDevice(UPnP *upnp, const QUuid &uuid, const QByteArray &deviceType)
  : QObject(upnp),
    d(new Data())
{
  d->upnp = upnp;
  d->uuid = uuid;
  d->deviceType = deviceType;
  d->rootDeviceRegistred = false;

  connect(&d->initialAdvertisementTimer, SIGNAL(timeout()), SLOT(sendAdvertisements()));
  d->initialAdvertisementTimer.setTimerType(Qt::VeryCoarseTimer);
  d->initialAdvertisementTimer.setSingleShot(true);

  d->baseDir = upnp->httpBaseDir() + "/" + uuid.toString().replace("{", "").replace("}", "").toLatin1() + "/";
}

RootDevice::~RootDevice()
{
  RootDevice::close();

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

UPnP * RootDevice::upnp()
{
  return d->upnp;
}

QByteArray RootDevice::httpBaseDir() const
{
  return d->baseDir;
}

void RootDevice::setDeviceName(const QString &deviceName)
{
  d->deviceName = deviceName;
}

void RootDevice::addIcon(const QString &path)
{
  d->icons += path;
}

void RootDevice::registerService(const QByteArray &serviceId, Service *service)
{
  QByteArray ext;
  for (int lc = qMax(serviceId.lastIndexOf(':'), serviceId.lastIndexOf('_')); (lc >= 0) && (lc < serviceId.size()); lc++)
  if ((serviceId[lc] >= 'A') && (serviceId[lc] <= 'Z'))
    ext += (serviceId[lc] + ('a' - 'A'));

  static const char id[] = "abcdefghijklmnopqrstuvwxyz";
  int n = 0;
  while (id[n] && (d->serviceExts.find(ext + QByteArray::number(n)) != d->serviceExts.end()))
    n++;

  if (id[n])
    d->services.insert(serviceId, qMakePair(service, ext + id[n]));
}

void RootDevice::unregisterService(const QByteArray &serviceId)
{
  d->services.remove(serviceId);
}

bool RootDevice::initialize()
{
  d->upnp->registerHttpCallback(d->baseDir, this);

  for (QMap<QByteArray, QPair<Service *, QByteArray> >::Iterator i = d->services.begin();
       i != d->services.end();
       i++)
  {
    i->first->initialize();
  }

  return enableRootDevice();
}

void RootDevice::close(void)
{
  d->upnp->unregisterHttpCallback(this);

  if (d->rootDeviceRegistred)
  {
    d->rootDeviceRegistred = false;
    d->initialAdvertisementTimer.stop();

    foreach(const ::UpnpDevice_Handle &handle, d->rootDeviceHandle)
      ::UpnpUnRegisterRootDevice(handle);
  }

  for (QMap<QByteArray, QPair<Service *, QByteArray> >::Iterator i = d->services.begin();
       i != d->services.end();
       i++)
  {
    i->first->close();
  }
}

void RootDevice::emitEvent(const QByteArray &serviceId)
{
  if (d->services.contains(serviceId))
  {
    IXMLStructures::EventablePropertySet propertyset;
    handleEvent(serviceId, propertyset);

    foreach(const ::UpnpDevice_Handle &handle, d->rootDeviceHandle)
    {
      ::UpnpNotifyExt(
            handle,
            udn(),
            serviceId,
            propertyset.doc);
    }
  }
}

QByteArray RootDevice::udn() const
{
  return QString("uuid:" + d->uuid.toString()).replace("{", "").replace("}", "").toUtf8();
}

void RootDevice::handleEvent(const QByteArray &serviceId, EventablePropertySet &propset)
{
  QMap<QByteArray, QPair<Service *, QByteArray> >::Iterator i = d->services.find(serviceId);
  if (i != d->services.end())
    i->first->writeEventableStateVariables(propset);
}

void RootDevice::writeDeviceDescription(DeviceDescription &desc)
{
  desc.setDeviceType(d->deviceType, "DMS-1.50");
  desc.setFriendlyName(d->deviceName);
  desc.setManufacturer(qApp->organizationName(), "http://" + qApp->organizationDomain() + "/");
  desc.setModel(qApp->applicationName(), qApp->applicationName(), "http://" + qApp->organizationDomain() + "/", qApp->applicationVersion());
  desc.setSerialNumber(d->uuid.toString().replace("{", "").replace("}", "").toUtf8());
  desc.setUDN(udn());

  foreach (const QString &path, d->icons)
  {
    const QImage icon(':' + path);
    if (!icon.isNull())
      desc.addIcon(path, UPnP::toMimeType(path), icon.width(), icon.height(), icon.depth());
  }

  desc.setPresentationURL("/");
}

HttpStatus RootDevice::httpRequest(const QUrl &request, const UPnP::HttpRequestInfo &requestInfo, QByteArray &contentType, QIODevice *&response)
{
  QByteArray host = request.host().toLatin1();
  if (request.port() > 0)
    host += ':' + QByteArray::number(request.port());

  if (request.path().startsWith(d->baseDir + d->deviceDescriptionFile))
  {
    IXMLStructures::DeviceDescription desc(host, d->baseDir);
    writeDeviceDescription(desc);

    for (QMap<QByteArray, QPair<Service *, QByteArray> >::Iterator i = d->services.begin();
         i != d->services.end();
         i++)
    {
      desc.addService(
            i->first->serviceType(),
            i.key(),
            d->serviceDescriptionFile + i->second + ".xml",
            d->serviceControlFile + i->second,
            d->serviceEventFile + i->second);
    }

    QBuffer * const buffer = new QBuffer();
    DOMString s = ixmlDocumenttoString(desc.doc);
    buffer->setData(s);
    ixmlFreeDOMString(s);
    contentType = UPnP::mimeTextXml;
    response = buffer;

    return HttpStatus_Ok;
  }
  else if (request.path().startsWith(d->baseDir + d->serviceDescriptionFile))
  {
    const uint l = d->baseDir.length() + qstrlen(d->serviceDescriptionFile);
    const QByteArray path = request.path().toUtf8();
    const QByteArray ext = path.mid(l, path.length() - 4 - l);

    for (QMap<QByteArray, QPair<Service *, QByteArray> >::Iterator i = d->services.begin();
         i != d->services.end();
         i++)
    {
      if (i->second == ext)
      {
        IXMLStructures::ServiceDescription desc;
        i->first->writeServiceDescription(desc);

        QBuffer * const buffer = new QBuffer();
        DOMString s = ixmlDocumenttoString(desc.doc);
        buffer->setData(s);
        ixmlFreeDOMString(s);
        contentType = UPnP::mimeTextXml;
        response = buffer;

        return HttpStatus_Ok;
      }
    }
  }
  else foreach (const QString &icon, d->icons)
  {
    if (request.path().startsWith(d->baseDir + icon))
      return d->upnp->handleHttpRequest("http://" + host + "/" + icon, requestInfo, contentType, response);
  }

  return HttpStatus_NotFound;
}

static void splitName(const QByteArray &fullName, QByteArray &prefix, QByteArray &name)
{
  const int colon = fullName.indexOf(':');
  if (colon >= 0)
  {
    prefix = fullName.left(colon);
    name = fullName.mid(colon + 1);
  }
  else
    name = fullName;
}

bool RootDevice::enableRootDevice(void)
{
  struct T : QThread
  {
    static int callback(Upnp_EventType eventType, void *event, void *cookie)
    {
      RootDevice * const me = reinterpret_cast<RootDevice *>(cookie);

      if (eventType == UPNP_CONTROL_ACTION_REQUEST)
      {
        struct F : UPnP::Functor
        {
          F(RootDevice *me, Upnp_Action_Request *request) : me(me), request(request) { }

          void operator()()
          {
            QMap<QByteArray, QPair<Service *, QByteArray> >::Iterator i = me->d->services.find(request->ServiceID);
            if ((i != me->d->services.end()) && me->d->rootDeviceRegistred)
            {
              UPnP::HttpRequestInfo requestInfo;
              requestInfo.host = request->RequestInfo.host;
              requestInfo.userAgent = request->RequestInfo.userAgent;
              requestInfo.sourceAddress = request->RequestInfo.sourceAddress;

              if ((strcmp(i->first->serviceType(), serviceTypeConnectionManager) == 0))
              {
                ConnectionManager * const service = static_cast<ConnectionManager *>(i->first);

                IXML_NodeList * const children = ixmlNode_getChildNodes(&request->ActionRequest->n);
                for (IXML_NodeList *i = children; i; i = i->next)
                {
                  QByteArray prefix = "ns0", name;
                  splitName(i->nodeItem->nodeName, prefix, name);

                  if (name == "GetCurrentConnectionIDs")
                  {
                    IXMLStructures::ActionGetCurrentConnectionIDs action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                  else if (name == "GetCurrentConnectionInfo")
                  {
                    IXMLStructures::ActionGetCurrentConnectionInfo action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                  else if (name == "GetProtocolInfo")
                  {
                    IXMLStructures::ActionGetProtocolInfo action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                }

                ixmlNodeList_free(children);
              }
              else if ((strcmp(i->first->serviceType(), serviceTypeContentDirectory) == 0))
              {
                ContentDirectory * const service = static_cast<ContentDirectory *>(i->first);

                IXML_NodeList * const children = ixmlNode_getChildNodes(&request->ActionRequest->n);
                for (IXML_NodeList *i = children; i; i = i->next)
                {
                  QByteArray prefix = "ns0", name;
                  splitName(i->nodeItem->nodeName, prefix, name);

                  if (name == "Browse")
                  {
                    IXMLStructures::ActionBrowse action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                  else if (name == "Search")
                  {
                    IXMLStructures::ActionSearch action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                  else if (name == "GetSearchCapabilities")
                  {
                    IXMLStructures::ActionGetSearchCapabilities action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                  else if (name == "GetSortCapabilities")
                  {
                    IXMLStructures::ActionGetSortCapabilities action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                  else if (name == "GetSystemUpdateID")
                  {
                    IXMLStructures::ActionGetSystemUpdateID action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                  else if (name == "X_GetFeatureList")
                  {
                    IXMLStructures::ActionGetFeatureList action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                }

                ixmlNodeList_free(children);
              }
              else if ((strcmp(i->first->serviceType(), serviceTypeMediaReceiverRegistrar) == 0))
              {
                MediaReceiverRegistrar * const service = static_cast<MediaReceiverRegistrar *>(i->first);

                IXML_NodeList * const children = ixmlNode_getChildNodes(&request->ActionRequest->n);
                for (IXML_NodeList *i = children; i; i = i->next)
                {
                  QByteArray prefix = "ns0", name;
                  splitName(i->nodeItem->nodeName, prefix, name);

                  if (name == "IsAuthorized")
                  {
                    IXMLStructures::ActionIsAuthorized action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                  else if (name == "IsValidated")
                  {
                    IXMLStructures::ActionIsValidated action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                  else if (name == "RegisterDevice")
                  {
                    IXMLStructures::ActionRegisterDevice action(i->nodeItem, request->ActionResult, prefix);
                    service->handleAction(requestInfo, action);
                  }
                }

                ixmlNodeList_free(children);
              }
            }

            emit me->handledEvent(QByteArray());
          }

          RootDevice * const me;
          Upnp_Action_Request * const request;
        } f(me, reinterpret_cast<Upnp_Action_Request *>(event));
        me->d->upnp->send(f);

        return 0;
      }
      else if (eventType == UPNP_EVENT_SUBSCRIPTION_REQUEST)
      {
        struct F : UPnP::Functor
        {
          F(RootDevice *me, Upnp_Subscription_Request *request) : me(me), request(request) { }

          void operator()()
          {
            if (me->d->rootDeviceRegistred)
            {
              udn = me->udn();
              me->handleEvent(request->ServiceId, propertyset);

              QMap<QByteArray, ::UpnpDevice_Handle>::Iterator i = me->d->rootDeviceHandle.find(request->Host);
              if (i != me->d->rootDeviceHandle.end())
                ::UpnpAcceptSubscriptionExt(*i, udn, request->ServiceId, propertyset.doc, request->Sid);
            }

            emit me->handledEvent(QByteArray());
          }

          RootDevice * const me;
          Upnp_Subscription_Request * const request;
          IXMLStructures::EventablePropertySet propertyset;
          QByteArray udn;
        } f(me, reinterpret_cast<Upnp_Subscription_Request *>(event));
        me->d->upnp->send(f);

        return 0;
      }

      qDebug() << "enableRootDevice::callback Unsupported eventType" << eventType;
      return -1;
    }

    T(RootDevice *me, const QByteArray &path) : me(me), path(path), result(UPNP_E_INTERNAL_ERROR) { }

    virtual void run()
    {
      for (char **i = ::UpnpGetServerIpAddresses(); i && *i; i++)
      {
        const QByteArray host = QByteArray(*i) + ":" + QByteArray::number(::UpnpGetServerPort());
        if (::UpnpRegisterRootDevice(
              "http://" + host + path,
              &T::callback, me,
              &(me->d->rootDeviceHandle[host])) == UPNP_E_SUCCESS)
        {
          result = UPNP_E_SUCCESS;
        }
      }
    }

    RootDevice * const me;
    const QByteArray path;
    volatile int result;
  } t(this, QByteArray(d->baseDir + d->deviceDescriptionFile) + ".xml");

  // Ugly, but needed as UpnpRegisterRootDevice retrieves files from the HTTP server.
  t.start();
  while (!t.isFinished()) { qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents, 16); }
  t.wait();

  if (t.result == UPNP_E_SUCCESS)
  {
    d->rootDeviceRegistred = true;
    d->initialAdvertisementTimer.start(d->advertisementDelay * 1000);

    return true;
  }
  else
    qWarning() << "UpnpRegisterRootDevice" << t.path << "failed:" << t.result;

  return false;
}

void RootDevice::sendAdvertisements()
{
  if (d->rootDeviceRegistred)
  foreach(const ::UpnpDevice_Handle &handle, d->rootDeviceHandle)
    ::UpnpSendAdvertisement(handle, d->advertisementExpiration);
}

} // End of namespace
