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
#include "client.h"

namespace LXiMediaCenter {

struct Client::Data
{
  UPnP                        * upnp;

  bool                          clientEnabled;
  UpnpClient_Handle             clientHandle;
};

Client::Client(UPnP *upnp)
  : QObject(upnp),
    d(new Data())
{
  d->upnp = upnp;
  d->clientEnabled = false;
}

Client::~Client()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

UPnP * Client::upnp()
{
  return d->upnp;
}

bool Client::initialize()
{
  return enableClient();
}

void Client::close(void)
{
  if (d->clientEnabled)
  {
    d->clientEnabled = false;
    ::UpnpUnRegisterClient(d->clientHandle);
  }
}

void Client::startSearch(const QByteArray &target, int mx)
{
  if (d->clientEnabled)
    ::UpnpSearchAsync(d->clientHandle, mx, target, this);
}

static IXML_Node * getElement(_IXML_Node *from, const char *name)
{
  IXML_Node *result = NULL;

  IXML_NodeList * const children = ixmlNode_getChildNodes(from);
  for (IXML_NodeList *i = children; i && !result; i = i->next)
  {
    const char *n = strchr(i->nodeItem->nodeName, ':');
    if (n == NULL)
      n = i->nodeItem->nodeName;
    else
      n++;

    if (strcmp(n, name) == 0)
      result = i->nodeItem;
  }

  ixmlNodeList_free(children);

  return result;
}

static QByteArray getText(_IXML_Node *from)
{
  QByteArray result;

  if (from)
  {
    IXML_NodeList * const children = ixmlNode_getChildNodes(from);
    for (IXML_NodeList *i = children; i; i = i->next)
    if (i->nodeItem->nodeValue)
      result += i->nodeItem->nodeValue;

    ixmlNodeList_free(children);
  }

  return result;
}

bool Client::getDeviceDescription(const QByteArray &location, DeviceDescription &description)
{
  char *buffer = NULL;
  char contentType[LINE_SIZE];
  if (::UpnpDownloadUrlItem(location, &buffer, contentType) == UPNP_E_SUCCESS)
  {
    IXML_Document * const doc = ixmlParseBuffer(buffer);
    if (doc)
    {
      IXML_Node * const rootElm = getElement(&doc->n, "root");
      if (rootElm)
      {
        IXML_Node * const deviceElm = getElement(rootElm, "device");
        if (deviceElm)
        {
          description.deviceType = getText(getElement(deviceElm, "deviceType"));
          description.friendlyName = QString::fromUtf8(getText(getElement(deviceElm, "friendlyName")));
          description.manufacturer = QString::fromUtf8(getText(getElement(deviceElm, "manufacturer")));
          description.modelName = QString::fromUtf8(getText(getElement(deviceElm, "modelName")));
          description.udn = getText(getElement(deviceElm, "UDN"));

          const QUrl presentationURL = QUrl::fromEncoded(getText(getElement(deviceElm, "presentationURL")));
          description.presentationURL = QUrl::fromEncoded(location);
          description.presentationURL.setPath(presentationURL.path());
          description.presentationURL.setQuery(presentationURL.query());

          IXML_Node * const iconListElm = getElement(deviceElm, "iconList");
          if (iconListElm)
          {
            IXML_Node * const iconElm = getElement(iconListElm, "icon");
            if (iconElm)
            {
              const QUrl iconURL = QUrl::fromEncoded(getText(getElement(iconElm, "url")));
              description.iconURL = QUrl::fromEncoded(location);
              description.iconURL.setPath(iconURL.path());
              description.iconURL.setQuery(iconURL.query());
            }
          }

          ixmlDocument_free(doc);
        }
      }
    }

    ::free(buffer);
    return true;
  }

  return false;
}

bool Client::enableClient()
{
  struct T
  {
    static int callback(Upnp_EventType eventType, void *event, void *cookie)
    {
      Client * const me = reinterpret_cast<Client *>(cookie);

      if ((eventType == UPNP_DISCOVERY_ADVERTISEMENT_ALIVE) ||
          (eventType == UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE) ||
          (eventType == UPNP_DISCOVERY_SEARCH_RESULT))
      {
        struct F : UPnP::Functor
        {
          F(Client *me, Upnp_EventType eventType, Upnp_Discovery *discovery) : me(me), eventType(eventType), discovery(discovery) { }

          void operator()()
          {
            if (me->d->clientEnabled)
            {
              if ((eventType == UPNP_DISCOVERY_ADVERTISEMENT_ALIVE) ||
                  (eventType == UPNP_DISCOVERY_SEARCH_RESULT))
              {
                emit me->deviceDiscovered(discovery->DeviceId, discovery->Location);
              }
              else if (eventType == UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE)
                emit me->deviceClosed(discovery->DeviceId);
            }
          }

          Client * const me;
          const Upnp_EventType eventType;
          Upnp_Discovery * const discovery;
        } f(me, eventType, reinterpret_cast<Upnp_Discovery *>(event));
        me->d->upnp->send(f);
      }

      return UPNP_E_SUCCESS;
    }
  };

  if (::UpnpRegisterClient(&T::callback, this, &d->clientHandle) == UPNP_E_SUCCESS)
  {
    d->clientEnabled = true;
    return true;
  }

  return false;
}

} // End of namespace
