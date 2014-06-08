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

#include "ixmlstructures.h"
#include <ixml.h>

namespace IXMLStructures {

XmlStructure::XmlStructure()
  : doc(ixmlDocument_createDocument()),
    dest(NULL)
{
}

XmlStructure::XmlStructure(_IXML_Document *&dest)
  : doc(dest ? dest : ixmlDocument_createDocument()),
    dest(&dest)
{
}

XmlStructure::~XmlStructure()
{
  if (dest)
    *dest = doc;
  else
    ixmlDocument_free(doc);
}

IXML_Element * XmlStructure::addElement(_IXML_Node *to, const char *name)
{
  IXML_Element *e = ixmlDocument_createElement(doc, name);
  ixmlNode_appendChild(to, &e->n);
  return e;
}

IXML_Element * XmlStructure::addElement(_IXML_Node *to, const char *ns, const char *name)
{
  IXML_Element *e = ixmlDocument_createElement(doc, name);

  char prefix[32];
  prefix[sizeof(prefix) - 1] = '\0';
  strncpy(prefix, name, sizeof(prefix) - 1);
  char * const colon = strchr(prefix, ':');
  if (colon)
  {
    *colon = '\0';
    char xmlns[sizeof(prefix) + 8];
    strcpy(xmlns, "xmlns:");
    strcat(xmlns, prefix);
    ixmlElement_setAttribute(e, xmlns, ns);
  }

  ixmlNode_appendChild(to, &e->n);
  return e;
}

IXML_Element * XmlStructure::addTextElement(IXML_Node *to, const char *name, const char *value)
{
  IXML_Element *e = addElement(to, name);
  IXML_Node *n = ixmlDocument_createTextNode(doc, value);
  ixmlNode_appendChild(&e->n, n);
  return e;
}

IXML_Element * XmlStructure::addTextElement(IXML_Node *to, const char *ns, const char *name, const char *value)
{
  IXML_Element *e = addElement(to, ns, name);
  IXML_Node *n = ixmlDocument_createTextNode(doc, value);
  ixmlNode_appendChild(&e->n, n);
  return e;
}

void XmlStructure::setAttribute(_IXML_Element *to, const char *name, const char *value)
{
  ixmlElement_setAttribute(to, name, value);
}

QByteArray XmlStructure::getTextElement(_IXML_Node *from, const char *name) const
{
  QByteArray result;

  IXML_NodeList * const children = ixmlNode_getChildNodes(from);
  for (IXML_NodeList *i = children; i; i = i->next)
  {
    const char *n = strchr(i->nodeItem->nodeName, ':');
    if (n == NULL)
      n = i->nodeItem->nodeName;
    else
      n++;

    if (strcmp(n, name) == 0)
    {
      IXML_NodeList * const children = ixmlNode_getChildNodes(i->nodeItem);
      for (IXML_NodeList *i = children; i; i = i->next)
      if (i->nodeItem->nodeValue)
        result += i->nodeItem->nodeValue;

      ixmlNodeList_free(children);

      break;
    }
  }

  ixmlNodeList_free(children);

  return result;
}


DeviceDescription::DeviceDescription(const QByteArray &host, const QByteArray &baseDir)
  : XmlStructure(),
    root(addElement(&doc->n, "root")),
    device(NULL),
    iconList(NULL),
    serviceList(NULL),
    host(host)
{
  setAttribute(root, "xmlns", "urn:schemas-upnp-org:device-1-0");

  IXML_Element * const specVersion = addElement(&root->n, "specVersion");
  addTextElement(&specVersion->n, "major", "1");
  addTextElement(&specVersion->n, "minor", "0");

  addTextElement(&root->n, "URLBase", "http://" + host + baseDir);

  device = addElement(&root->n, "device");
}

void DeviceDescription::setDeviceType(const QByteArray &deviceType, const QByteArray &dlnaDoc)
{
  addTextElement(&device->n, "deviceType", deviceType);
  if (!dlnaDoc.isEmpty())
    addTextElement(&device->n, "urn:schemas-dlna-org:device-1-0", "dlna:X_DLNADOC", dlnaDoc);
}

void DeviceDescription::setFriendlyName(const QString &friendlyName)
{
  addTextElement(&device->n, "friendlyName", friendlyName.toUtf8());
}

void DeviceDescription::setManufacturer(const QString &manufacturer, const QString &url)
{
  addTextElement(&device->n, "manufacturer", manufacturer.toUtf8());
  addTextElement(&device->n, "manufacturerURL", url.toUtf8());
}

void DeviceDescription::setModel(const QString &description, const QString &name, const QString &url, const QString &number)
{
  addTextElement(&device->n, "modelDescription", description.toUtf8());
  addTextElement(&device->n, "modelName", name.toUtf8());
  addTextElement(&device->n, "modelNumber", number.toUtf8());
  addTextElement(&device->n, "modelURL", url.toUtf8());
}

void DeviceDescription::setSerialNumber(const QByteArray &serialNumber)
{
  addTextElement(&device->n, "serialNumber", serialNumber);
}

void DeviceDescription::setUDN(const QByteArray &udn)
{
  addTextElement(&device->n, "UDN", udn);
}

void DeviceDescription::setPresentationURL(const QByteArray &presentationURL)
{
  if (serviceList == NULL)
    serviceList = addElement(&device->n, "serviceList");

  addTextElement(&device->n, "presentationURL", "http://" + host + presentationURL);
}

void DeviceDescription::addIcon(const QString &url, const char *mimetype, int width, int height, int depth)
{
  if (iconList == NULL)
    iconList = addElement(&device->n, "iconList");

  IXML_Element * const icon = addElement(&iconList->n, "icon");
  addTextElement(&icon->n, "mimetype", mimetype);
  addTextElement(&icon->n, "width", QByteArray::number(width));
  addTextElement(&icon->n, "height", QByteArray::number(height));
  addTextElement(&icon->n, "depth", QByteArray::number(depth));
  addTextElement(&icon->n, "url", url.toUtf8());
}

void DeviceDescription::addService(const QByteArray &serviceType, const QByteArray &serviceId, const QByteArray &descriptionFile, const QByteArray &controlFile, const QByteArray &eventFile)
{
  if (serviceList == NULL)
    serviceList = addElement(&device->n, "serviceList");

  IXML_Element * const service = addElement(&serviceList->n, "service");
  addTextElement(&service->n, "serviceType", serviceType);
  addTextElement(&service->n, "serviceId", serviceId);
  addTextElement(&service->n, "SCPDURL", descriptionFile);
  addTextElement(&service->n, "controlURL", controlFile);
  addTextElement(&service->n, "eventSubURL", eventFile);
}


ServiceDescription::ServiceDescription()
  : XmlStructure(),
    scpd(addElement(&doc->n, "scpd")),
    actionList(NULL),
    serviceStateTable(NULL)
{
  setAttribute(scpd, "xmlns", "urn:schemas-upnp-org:service-1-0");

  IXML_Element * const specVersion = addElement(&scpd->n, "specVersion");
  addTextElement(&specVersion->n, "major", "1");
  addTextElement(&specVersion->n, "minor", "0");
}

void ServiceDescription::addAction(const char *name, const char * const *argname, const char * const *argdir, const char * const *argvar, int argcount)
{
  if (actionList == NULL)
    actionList = addElement(&scpd->n, "actionList");

  IXML_Element * const action = addElement(&actionList->n, "action");
  addTextElement(&action->n, "name", name);

  if (argcount > 0)
  {
    IXML_Element * const argumentList = addElement(&action->n, "argumentList");
    for (int i=0; i<argcount; i++)
    {
      IXML_Element * const argument = addElement(&argumentList->n, "argument");
      addTextElement(&argument->n, "name", argname[i]);
      addTextElement(&argument->n, "direction", argdir[i]);
      addTextElement(&argument->n, "relatedStateVariable", argvar[i]);
    }
  }
}

void ServiceDescription::addStateVariable(const char *name, const char *type, bool sendEvents, const char * const *values, int valcount)
{
  if (serviceStateTable == NULL)
    serviceStateTable = addElement(&scpd->n, "serviceStateTable");

  IXML_Element * const stateVariable = addElement(&serviceStateTable->n, "stateVariable");
  setAttribute(stateVariable, "sendEvents", sendEvents ? "yes" : "no");
  addTextElement(&stateVariable->n, "name", name);
  addTextElement(&stateVariable->n, "dataType", type);

  if (valcount > 0)
  {
    IXML_Element * const allowedValueList = addElement(&stateVariable->n, "allowedValueList");
    for (int i=0; i<valcount; i++)
      addTextElement(&allowedValueList->n, "allowedValue", values[i]);
  }
}


EventablePropertySet::EventablePropertySet()
  : XmlStructure(),
    propertySet(addElement(&doc->n, "propertyset"))
{
  setAttribute(propertySet, "xmlns", "urn:schemas-upnp-org:event-1-0");
}

void EventablePropertySet::addProperty(const QString &name, const QString &value)
{
  IXML_Element * const property = addElement(&propertySet->n, "property");
  addTextElement(&property->n, name.toUtf8(), value.toUtf8());
}


ActionGetCurrentConnectionIDs::ActionGetCurrentConnectionIDs(IXML_Node *, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
//    src(src),
    prefix(prefix)
{
}

void ActionGetCurrentConnectionIDs::setResponse(const QList<qint32> &ids)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":GetCurrentConnectionIDsResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeConnectionManager);

  QByteArray result;
  foreach (qint32 id, ids)
    result += "," + QByteArray::number(id);

  addTextElement(&response->n, "ConnectionIDs", result.isEmpty() ? result : result.mid(1));
}


ActionGetCurrentConnectionInfo::ActionGetCurrentConnectionInfo(IXML_Node *src, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
    src(src),
    prefix(prefix)
{
}

qint32 ActionGetCurrentConnectionInfo::getConnectionID() const
{
  return getTextElement(src, "ConnectionID").toInt();
}

void ActionGetCurrentConnectionInfo::setResponse(const ConnectionManager::ConnectionInfo &info)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":GetCurrentConnectionInfoResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeConnectionManager);
  addTextElement(&response->n, "RcsID", QByteArray::number(info.rcsID));
  addTextElement(&response->n, "AVTransportID", QByteArray::number(info.avTransportID));
  addTextElement(&response->n, "ProtocolInfo", info.protocolInfo);
  addTextElement(&response->n, "PeerConnectionManager", info.peerConnectionManager);
  addTextElement(&response->n, "PeerConnectionID", QByteArray::number(info.peerConnectionID));

  const char *direction = NULL;
  switch (info.direction)
  {
  case ConnectionManager::ConnectionInfo::Input:   direction = "Input";  break;
  case ConnectionManager::ConnectionInfo::Output:  direction = "Output"; break;
  }

  if (direction)
    addTextElement(&response->n, "Direction", direction);

  const char *status = NULL;
  switch (info.status)
  {
  case ConnectionManager::ConnectionInfo::OK:                    status = "OK";                    break;
  case ConnectionManager::ConnectionInfo::ContentFormatMismatch: status = "ContentFormatMismatch"; break;
  case ConnectionManager::ConnectionInfo::InsufficientBandwidth: status = "InsufficientBandwidth"; break;
  case ConnectionManager::ConnectionInfo::UnreliableChannel:     status = "UnreliableChannel";     break;
  case ConnectionManager::ConnectionInfo::Unknown:               status = "Unknown";               break;
  }

  if (status)
    addTextElement(&response->n, "Status", status);
}


ActionGetProtocolInfo::ActionGetProtocolInfo(IXML_Node *, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
//    src(src),
    prefix(prefix)
{
}

void ActionGetProtocolInfo::setResponse(const QByteArray &source, const QByteArray &sink)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":GetProtocolInfoResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeConnectionManager);
  addTextElement(&response->n, "Source", source);
  addTextElement(&response->n, "Sink", sink);
}


ActionBrowse::ActionBrowse(IXML_Node *src, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
    src(src),
    prefix(prefix),
    result(),
    didl(result.addElement(&result.doc->n, "DIDL-Lite")),
    numberReturned(0)
{
  result.setAttribute(didl, "xmlns", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
  result.setAttribute(didl, "xmlns:dc", "http://purl.org/dc/elements/1.1/");
  result.setAttribute(didl, "xmlns:dlna", "urn:schemas-dlna-org:metadata-1-0/");
  result.setAttribute(didl, "xmlns:upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
}

QByteArray ActionBrowse::getObjectID() const
{
  return getTextElement(src, "ObjectID");
}

ActionBrowse::BrowseFlag ActionBrowse::getBrowseFlag() const
{
  const QByteArray f = getTextElement(src, "BrowseFlag");
  if (f == "BrowseMetadata")
    return ActionBrowse::BrowseMetadata;
  else if (f == "BrowseDirectChildren")
    return ActionBrowse::BrowseDirectChildren;

  return BrowseFlag(-1);
}

QByteArray ActionBrowse::getFilter() const
{
  return getTextElement(src, "Filter");
}

quint32 ActionBrowse::getStartingIndex() const
{
  return getTextElement(src, "StartingIndex").toUInt();
}

quint32 ActionBrowse::getRequestedCount() const
{
  return getTextElement(src, "RequestedCount").toUInt();
}

QByteArray ActionBrowse::getSortCriteria() const
{
  return getTextElement(src, "SortCriteria");
}

void ActionBrowse::addItem(const ContentDirectory::BrowseItem &browseItem)
{
  IXML_Element * const item = result.addElement(&didl->n, "item");
  result.setAttribute(item, "id", browseItem.id);
  result.setAttribute(item, "parentID", browseItem.parentID);
  result.setAttribute(item, "restricted", browseItem.restricted ? "1" : "0");

  result.addTextElement(&item->n, "dc:title", browseItem.title);

  typedef QPair<QByteArray, QByteArray> Attribute;
  foreach (const Attribute &attribute, browseItem.attributes)
  {
    IXML_Element * const e = result.addTextElement(&item->n, attribute.first, attribute.second);
    if (attribute.first == "upnp:albumArtURI")
    {
      if (attribute.second.endsWith(".jpeg") || attribute.second.endsWith(".jpeg"))
        result.setAttribute(e, "dlna:profileID", "JPEG_TN");
      else if (attribute.second.endsWith(".png"))
        result.setAttribute(e, "dlna:profileID", "PNG_SM");
    }
  }

  typedef QPair<QByteArray, ConnectionManager::Protocol> File;
  foreach (const File &file, browseItem.files)
  {
    IXML_Element * const res = result.addTextElement(&item->n, "res", file.first);
    result.setAttribute(res, "protocolInfo", file.second.toByteArray());

    if (browseItem.duration > 0)
      result.setAttribute(res, "duration", QTime(0, 0).addSecs(browseItem.duration).toString("h:mm:ss.zzz").toUtf8());

    if (file.second.sampleRate > 0)
      result.setAttribute(res, "sampleFrequency", QByteArray::number(file.second.sampleRate));

    if (file.second.channels > 0)
      result.setAttribute(res, "nrAudioChannels", QByteArray::number(file.second.channels));

    if (file.second.resolution.isValid() && !file.second.resolution.isNull())
      result.setAttribute(res, "resolution", QByteArray::number(file.second.resolution.width()) + "x" + QByteArray::number(file.second.resolution.height()));

    if (file.second.size > 0)
      result.setAttribute(res, "size", QByteArray::number(file.second.size));
  }

  numberReturned++;
}

void ActionBrowse::addContainer(const ContentDirectory::BrowseContainer &browseContainer)
{
  IXML_Element * const item = result.addElement(&didl->n, "container");
  result.setAttribute(item, "id", browseContainer.id);
  result.setAttribute(item, "parentID", browseContainer.parentID);
  result.setAttribute(item, "restricted", browseContainer.restricted ? "1" : "0");

  if (browseContainer.childCount != quint32(-1))
    result.setAttribute(item, "childCount", QByteArray::number(browseContainer.childCount));

  result.addTextElement(&item->n, "dc:title", browseContainer.title);

  typedef QPair<QByteArray, QByteArray> Attribute;
  foreach (const Attribute &attribute, browseContainer.attributes)
    result.addTextElement(&item->n, attribute.first, attribute.second);

  numberReturned++;
}

void ActionBrowse::setResponse(quint32 totalMatches, quint32 updateID)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":BrowseResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeContentDirectory);

  DOMString r = ixmlDocumenttoString(result.doc);
  addTextElement(&response->n, "Result", r);
  ixmlFreeDOMString(r);

  addTextElement(&response->n, "NumberReturned", QByteArray::number(numberReturned));
  addTextElement(&response->n, "TotalMatches", QByteArray::number(totalMatches));
  addTextElement(&response->n, "UpdateID", QByteArray::number(updateID));
}


ActionSearch::ActionSearch(IXML_Node *src, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
    src(src),
    prefix(prefix),
    result(),
    didl(result.addElement(&result.doc->n, "DIDL-Lite")),
    numberReturned(0)
{
  result.setAttribute(didl, "xmlns", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
  result.setAttribute(didl, "xmlns:dc", "http://purl.org/dc/elements/1.1/");
  result.setAttribute(didl, "xmlns:dlna", "urn:schemas-dlna-org:metadata-1-0/");
  result.setAttribute(didl, "xmlns:upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
}

QByteArray ActionSearch::getContainerID() const
{
  return getTextElement(src, "ContainerID");
}

QByteArray ActionSearch::getSearchCriteria() const
{
  return getTextElement(src, "SearchCriteria");
}

QByteArray ActionSearch::getFilter() const
{
  return getTextElement(src, "Filter");
}

quint32 ActionSearch::getStartingIndex() const
{
  return getTextElement(src, "StartingIndex").toUInt();
}

quint32 ActionSearch::getRequestedCount() const
{
  return getTextElement(src, "RequestedCount").toUInt();
}

QByteArray ActionSearch::getSortCriteria() const
{
  return getTextElement(src, "SortCriteria");
}

void ActionSearch::addItem(const ContentDirectory::BrowseItem &browseItem)
{
  IXML_Element * const item = result.addElement(&didl->n, "item");
  result.setAttribute(item, "id", browseItem.id);
  result.setAttribute(item, "parentID", browseItem.parentID);
  result.setAttribute(item, "restricted", browseItem.restricted ? "1" : "0");

  result.addTextElement(&item->n, "dc:title", browseItem.title);

  typedef QPair<QByteArray, QByteArray> Attribute;
  foreach (const Attribute &attribute, browseItem.attributes)
  {
    IXML_Element * const e = result.addTextElement(&item->n, attribute.first, attribute.second);
    if (attribute.first == "upnp:albumArtURI")
    {
      if (attribute.second.endsWith(".jpeg") || attribute.second.endsWith(".jpeg"))
        result.setAttribute(e, "dlna:profileID", "JPEG_TN");
      else if (attribute.second.endsWith(".png"))
        result.setAttribute(e, "dlna:profileID", "PNG_SM");
    }
  }

  typedef QPair<QByteArray, ConnectionManager::Protocol> File;
  foreach (const File &file, browseItem.files)
  {
    IXML_Element * const res = result.addTextElement(&item->n, "res", file.first);
    result.setAttribute(res, "protocolInfo", file.second.toByteArray());

    if (browseItem.duration > 0)
      result.setAttribute(res, "duration", QTime(0, 0).addSecs(browseItem.duration).toString("h:mm:ss.zzz").toUtf8());

    if (file.second.sampleRate > 0)
      result.setAttribute(res, "sampleFrequency", QByteArray::number(file.second.sampleRate));

    if (file.second.channels > 0)
      result.setAttribute(res, "nrAudioChannels", QByteArray::number(file.second.channels));

    if (file.second.resolution.isValid() && !file.second.resolution.isNull())
      result.setAttribute(res, "resolution", QByteArray::number(file.second.resolution.width()) + "x" + QByteArray::number(file.second.resolution.height()));

    if (file.second.size > 0)
      result.setAttribute(res, "size", QByteArray::number(file.second.size));
  }

  numberReturned++;
}

void ActionSearch::setResponse(quint32 totalMatches, quint32 updateID)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":SearchResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeContentDirectory);

  DOMString r = ixmlDocumenttoString(result.doc);
  addTextElement(&response->n, "Result", r);
  ixmlFreeDOMString(r);

  addTextElement(&response->n, "NumberReturned", QByteArray::number(numberReturned));
  addTextElement(&response->n, "TotalMatches", QByteArray::number(totalMatches));
  addTextElement(&response->n, "UpdateID", QByteArray::number(updateID));
}


ActionGetSearchCapabilities::ActionGetSearchCapabilities(IXML_Node *, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
//    src(src),
    prefix(prefix)
{
}

void ActionGetSearchCapabilities::setResponse(const QByteArray &ids)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":GetSearchCapabilitiesResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeContentDirectory);
  addTextElement(&response->n, "SearchCaps", ids);
}


ActionGetSortCapabilities::ActionGetSortCapabilities(IXML_Node *, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
//    src(src),
    prefix(prefix)
{
}

void ActionGetSortCapabilities::setResponse(const QByteArray &ids)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":GetSortCapabilitiesResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeContentDirectory);
  addTextElement(&response->n, "SortCaps", ids);
}


ActionGetSystemUpdateID::ActionGetSystemUpdateID(IXML_Node *, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
//    src(src),
    prefix(prefix)
{
}

void ActionGetSystemUpdateID::setResponse(quint32 id)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":GetSystemUpdateIDResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeContentDirectory);
  addTextElement(&response->n, "Id", QByteArray::number(id));
}


// Samsung GetFeatureList
ActionGetFeatureList::ActionGetFeatureList(IXML_Node *, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
//    src(src),
    prefix(prefix)
{
}

void ActionGetFeatureList::setResponse(const QList<QByteArray> &containers)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":X_GetFeatureListResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeContentDirectory);

  XmlStructure sdoc;
  IXML_Element * const features = sdoc.addElement(&sdoc.doc->n, "Features");
  sdoc.setAttribute(features, "xmlns", "urn:schemas-upnp-org:av:avs");
  sdoc.setAttribute(features, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  sdoc.setAttribute(features, "xsi:schemaLocation", "urn:schemas-upnp-org:av:avs http://www.upnp.org/schemas/av/avs.xsd");
  IXML_Element * const feature = sdoc.addElement(&features->n, "Feature");
  sdoc.setAttribute(feature, "name", "samsung.com_BASICVIEW");
  sdoc.setAttribute(feature, "version", "1");

  for (int i = 0; i < containers.size(); i++)
  {
    IXML_Element * const e = sdoc.addElement(&feature->n, "container");
    sdoc.setAttribute(e, "id", QByteArray::number(i + 1));
    sdoc.setAttribute(e, "type", containers[i]);
  }

  DOMString f = ixmlDocumenttoString(sdoc.doc);
  addTextElement(&response->n, "FeatureList", f);
  ixmlFreeDOMString(f);
}


// Microsoft MediaReceiverRegistrar IsAuthorized
ActionIsAuthorized::ActionIsAuthorized(IXML_Node *src, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
    src(src),
    prefix(prefix)
{
}

QByteArray ActionIsAuthorized::getDeviceID() const
{
  return getTextElement(src, "DeviceID");
}

void ActionIsAuthorized::setResponse(int result)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":IsAuthorizedResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeMediaReceiverRegistrar);

  addTextElement(&response->n, "Result", QByteArray::number(result));
  setAttribute(response, "xmlns:dt", "urn:schemas-microsoft-com:datatypes");
  setAttribute(response, "dt:dt", "int");
}


// Microsoft MediaReceiverRegistrar IsValidated
ActionIsValidated::ActionIsValidated(IXML_Node *src, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
    src(src),
    prefix(prefix)
{
}

QByteArray ActionIsValidated::getDeviceID() const
{
  return getTextElement(src, "DeviceID");
}

void ActionIsValidated::setResponse(int result)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":IsValidatedResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeMediaReceiverRegistrar);

  addTextElement(&response->n, "Result", QByteArray::number(result));
  setAttribute(response, "xmlns:dt", "urn:schemas-microsoft-com:datatypes");
  setAttribute(response, "dt:dt", "int");
}


// Microsoft MediaReceiverRegistrar RegisterDevice
ActionRegisterDevice::ActionRegisterDevice(IXML_Node *src, IXML_Document *&dst, const char *prefix)
  : XmlStructure(dst),
    src(src),
    prefix(prefix)
{
}

QByteArray ActionRegisterDevice::getRegistrationReqMsg() const
{
  return QByteArray::fromBase64(getTextElement(src, "RegistrationReqMsg"));
}

void ActionRegisterDevice::setResponse(const QByteArray &result)
{
  IXML_Element * const response = addElement(&doc->n, prefix + ":RegisterDeviceResponse");
  setAttribute(response, "xmlns:" + prefix, RootDevice::serviceTypeMediaReceiverRegistrar);

  addTextElement(&response->n, "RegistrationRespMsg", result.toBase64());
  setAttribute(response, "xmlns:dt", "urn:schemas-microsoft-com:datatypes");
  setAttribute(response, "dt:dt", "bin.Base64");
}

}
