/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include "ixml_structures.h"
#include <ixml.h>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace lximediacenter {
namespace ixml_structures {

xml_structure::xml_structure()
  : doc(ixmlDocument_createDocument()),
    dest(NULL)
{
}

xml_structure::xml_structure(_IXML_Document *&dest)
  : doc(dest ? dest : ixmlDocument_createDocument()),
    dest(&dest)
{
}

xml_structure::~xml_structure()
{
  if (dest)
    *dest = doc;
  else
    ixmlDocument_free(doc);
}

IXML_Element * xml_structure::add_element(_IXML_Node *to, const std::string &name)
{
  IXML_Element *e = ixmlDocument_createElement(doc, name.c_str());
  ixmlNode_appendChild(to, &e->n);
  return e;
}

IXML_Element * xml_structure::add_element(_IXML_Node *to, const std::string &ns, const std::string &name)
{
  IXML_Element *e = ixmlDocument_createElement(doc, name.c_str());

  char prefix[32];
  prefix[sizeof(prefix) - 1] = '\0';
  strncpy(prefix, name.c_str(), sizeof(prefix) - 1);
  char * const colon = strchr(prefix, ':');
  if (colon)
  {
    *colon = '\0';
    char xmlns[sizeof(prefix) + 8];
    strcpy(xmlns, "xmlns:");
    strcat(xmlns, prefix);
    ixmlElement_setAttribute(e, xmlns, ns.c_str());
  }

  ixmlNode_appendChild(to, &e->n);
  return e;
}

IXML_Element * xml_structure::add_textelement(IXML_Node *to, const std::string &name, const std::string &value)
{
  IXML_Element *e = add_element(to, name);
  IXML_Node *n = ixmlDocument_createTextNode(doc, value.c_str());
  ixmlNode_appendChild(&e->n, n);
  return e;
}

IXML_Element * xml_structure::add_textelement(IXML_Node *to, const std::string &ns, const std::string &name, const std::string &value)
{
  IXML_Element *e = add_element(to, ns, name);
  IXML_Node *n = ixmlDocument_createTextNode(doc, value.c_str());
  ixmlNode_appendChild(&e->n, n);
  return e;
}

void xml_structure::set_attribute(_IXML_Element *to, const std::string &name, const std::string &value)
{
  ixmlElement_setAttribute(to, name.c_str(), value.c_str());
}

std::string xml_structure::get_textelement(_IXML_Node *from, const std::string &name) const
{
  std::string result;

  IXML_NodeList * const children = ixmlNode_getChildNodes(from);
  for (IXML_NodeList *i = children; i; i = i->next)
  {
    const char *n = strchr(i->nodeItem->nodeName, ':');
    if (n == NULL)
      n = i->nodeItem->nodeName;
    else
      n++;

    if (strcmp(n, name.c_str()) == 0)
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


device_description::device_description(const std::string &host, const std::string &baseDir)
  : xml_structure(),
    root(add_element(&doc->n, "root")),
    device(NULL),
    iconlist(NULL),
    servicelist(NULL),
    host(host)
{
  set_attribute(root, "xmlns", "urn:schemas-upnp-org:device-1-0");

  IXML_Element * const specVersion = add_element(&root->n, "specVersion");
  add_textelement(&specVersion->n, "major", "1");
  add_textelement(&specVersion->n, "minor", "0");

  add_textelement(&root->n, "URLBase", "http://" + host + baseDir);

  device = add_element(&root->n, "device");
}

void device_description::set_devicetype(const std::string &deviceType, const std::string &dlnaDoc)
{
  add_textelement(&device->n, "deviceType", deviceType);
  if (!dlnaDoc.empty())
    add_textelement(&device->n, "urn:schemas-dlna-org:device-1-0", "dlna:X_DLNADOC", dlnaDoc);
}

void device_description::set_friendlyname(const std::string &friendlyName)
{
  add_textelement(&device->n, "friendlyName", friendlyName);
}

void device_description::set_manufacturer(const std::string &manufacturer, const std::string &url)
{
  add_textelement(&device->n, "manufacturer", manufacturer);
  add_textelement(&device->n, "manufacturerURL", url);
}

void device_description::set_model(const std::string &description, const std::string &name, const std::string &url, const std::string &number)
{
  add_textelement(&device->n, "modelDescription", description);
  add_textelement(&device->n, "modelName", name);
  add_textelement(&device->n, "modelNumber", number);
  add_textelement(&device->n, "modelURL", url);
}

void device_description::set_serialnumber(const std::string &serialNumber)
{
  add_textelement(&device->n, "serialNumber", serialNumber);
}

void device_description::set_udn(const std::string &udn)
{
  add_textelement(&device->n, "UDN", udn);
}

void device_description::set_presentation_url(const std::string &presentationURL)
{
  if (servicelist == NULL)
    servicelist = add_element(&device->n, "servicelist");

  add_textelement(&device->n, "presentationURL", "http://" + host + presentationURL);
}

void device_description::add_icon(const std::string &url, const char *mimetype, int width, int height, int depth)
{
  if (iconlist == NULL)
    iconlist = add_element(&device->n, "iconlist");

  IXML_Element * const icon = add_element(&iconlist->n, "icon");
  add_textelement(&icon->n, "mimetype", mimetype);
  add_textelement(&icon->n, "width", std::to_string(width));
  add_textelement(&icon->n, "height", std::to_string(height));
  add_textelement(&icon->n, "depth", std::to_string(depth));
  add_textelement(&icon->n, "url", url);
}

void device_description::add_service(const std::string &service_type, const std::string &service_id, const std::string &description_file, const std::string &control_file, const std::string &event_file)
{
  if (servicelist == NULL)
    servicelist = add_element(&device->n, "servicelist");

  IXML_Element * const service = add_element(&servicelist->n, "service");
  add_textelement(&service->n, "serviceType", service_type);
  add_textelement(&service->n, "serviceId", service_id);
  add_textelement(&service->n, "SCPDURL", description_file);
  add_textelement(&service->n, "controlURL", control_file);
  add_textelement(&service->n, "eventSubURL", event_file);
}


service_description::service_description()
  : xml_structure(),
    scpd(add_element(&doc->n, "scpd")),
    actionlist(NULL),
    servicestatetable(NULL)
{
  set_attribute(scpd, "xmlns", "urn:schemas-upnp-org:service-1-0");

  IXML_Element * const specVersion = add_element(&scpd->n, "specVersion");
  add_textelement(&specVersion->n, "major", "1");
  add_textelement(&specVersion->n, "minor", "0");
}

void service_description::add_action(const char *name, const char * const *argname, const char * const *argdir, const char * const *argvar, int argcount)
{
  if (actionlist == NULL)
    actionlist = add_element(&scpd->n, "actionlist");

  IXML_Element * const action = add_element(&actionlist->n, "action");
  add_textelement(&action->n, "name", name);

  if (argcount > 0)
  {
    IXML_Element * const argumentList = add_element(&action->n, "argumentList");
    for (int i=0; i<argcount; i++)
    {
      IXML_Element * const argument = add_element(&argumentList->n, "argument");
      add_textelement(&argument->n, "name", argname[i]);
      add_textelement(&argument->n, "direction", argdir[i]);
      add_textelement(&argument->n, "relatedStateVariable", argvar[i]);
    }
  }
}

void service_description::add_statevariable(const char *name, const char *type, bool sendEvents, const char * const *values, int valcount)
{
  if (servicestatetable == NULL)
    servicestatetable = add_element(&scpd->n, "servicestatetable");

  IXML_Element * const stateVariable = add_element(&servicestatetable->n, "stateVariable");
  set_attribute(stateVariable, "sendEvents", sendEvents ? "yes" : "no");
  add_textelement(&stateVariable->n, "name", name);
  add_textelement(&stateVariable->n, "dataType", type);

  if (valcount > 0)
  {
    IXML_Element * const allowedValueList = add_element(&stateVariable->n, "allowedValueList");
    for (int i=0; i<valcount; i++)
      add_textelement(&allowedValueList->n, "allowedValue", values[i]);
  }
}


eventable_propertyset::eventable_propertyset()
  : xml_structure(),
    propertySet(add_element(&doc->n, "propertyset"))
{
  set_attribute(propertySet, "xmlns", "urn:schemas-upnp-org:event-1-0");
}

void eventable_propertyset::add_property(const std::string &name, const std::string &value)
{
  IXML_Element * const property = add_element(&propertySet->n, "property");
  add_textelement(&property->n, name, value);
}

action_get_current_connectionids::action_get_current_connectionids(IXML_Node *, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    prefix(prefix)
{
}

void action_get_current_connectionids::set_response(const std::vector<int32_t> &ids)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":GetCurrentConnectionIDsResponse");
  set_attribute(response, "xmlns:" + prefix, connection_manager::service_type);

  std::string result;
  for (auto id : ids)
    result += "," + std::to_string(id);

  add_textelement(&response->n, "ConnectionIDs", result.empty() ? result : result.substr(1));
}


action_get_current_connection_info::action_get_current_connection_info(IXML_Node *src, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    src(src),
    prefix(prefix)
{
}

int32_t action_get_current_connection_info::get_connectionid() const
{
  try { return std::stoi(get_textelement(src, "ConnectionID")); }
  catch (const std::invalid_argument &) { return -1; }
  catch (const std::out_of_range &) { return -1; }
}

void action_get_current_connection_info::set_response(const connection_manager::connection_info &info)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":GetCurrentConnectionInfoResponse");
  set_attribute(response, "xmlns:" + prefix, connection_manager::service_type);
  add_textelement(&response->n, "RcsID", std::to_string(info.rcs_id));
  add_textelement(&response->n, "AVTransportID", std::to_string(info.avtransport_id));
  add_textelement(&response->n, "ProtocolInfo", info.protocol_info);
  add_textelement(&response->n, "PeerConnectionManager", info.peerconnection_manager);
  add_textelement(&response->n, "PeerConnectionID", std::to_string(info.peerconnection_id));

  const char *direction = NULL;
  switch (info.direction)
  {
  case connection_manager::connection_info::input:   direction = "Input";  break;
  case connection_manager::connection_info::output:  direction = "Output"; break;
  }

  if (direction)
    add_textelement(&response->n, "Direction", direction);

  const char *status = NULL;
  switch (info.status)
  {
  case connection_manager::connection_info::ok:                     status = "OK";                    break;
  case connection_manager::connection_info::contentformat_mismatch: status = "ContentFormatMismatch"; break;
  case connection_manager::connection_info::insufficient_bandwidth: status = "InsufficientBandwidth"; break;
  case connection_manager::connection_info::unreliable_channel:     status = "UnreliableChannel";     break;
  case connection_manager::connection_info::unknown:                status = "Unknown";               break;
  }

  if (status)
    add_textelement(&response->n, "Status", status);
}


action_get_protocol_info::action_get_protocol_info(IXML_Node *, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    prefix(prefix)
{
}

void action_get_protocol_info::set_response(const std::string &source, const std::string &sink)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":GetProtocolInfoResponse");
  set_attribute(response, "xmlns:" + prefix, connection_manager::service_type);
  add_textelement(&response->n, "Source", source);
  add_textelement(&response->n, "Sink", sink);
}


action_browse::action_browse(IXML_Node *src, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    src(src),
    prefix(prefix),
    result(),
    didl(result.add_element(&result.doc->n, "DIDL-Lite")),
    number_returned(0)
{
  result.set_attribute(didl, "xmlns", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
  result.set_attribute(didl, "xmlns:dc", "http://purl.org/dc/elements/1.1/");
  result.set_attribute(didl, "xmlns:dlna", "urn:schemas-dlna-org:metadata-1-0/");
  result.set_attribute(didl, "xmlns:upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
}

std::string action_browse::get_object_id() const
{
  return get_textelement(src, "ObjectID");
}

action_browse::browse_flag action_browse::get_browse_flag() const
{
  const std::string f = get_textelement(src, "BrowseFlag");
  if (f == "BrowseMetadata")
    return action_browse::browse_flag::metadata;
  else if (f == "BrowseDirectChildren")
    return action_browse::browse_flag::direct_children;

  return action_browse::browse_flag(-1);
}

std::string action_browse::get_filter() const
{
  return get_textelement(src, "Filter");
}

size_t action_browse::get_starting_index() const
{
  try { return std::stoul(get_textelement(src, "StartingIndex")); }
  catch (const std::invalid_argument &) { return size_t(-1); }
  catch (const std::out_of_range &) { return size_t(-1); }
}

size_t action_browse::get_requested_count() const
{
  try { return std::stoul(get_textelement(src, "RequestedCount")); }
  catch (const std::invalid_argument &) { return size_t(-1); }
  catch (const std::out_of_range &) { return size_t(-1); }
}

std::string action_browse::get_sort_criteria() const
{
  return get_textelement(src, "SortCriteria");
}

static std::string to_time(int secs)
{
  std::ostringstream str;
  str << (secs / 3600)
      << ":" << std::setw(2) << std::setfill('0') << ((secs % 3600) / 60)
      << ":" << std::setw(2) << std::setfill('0') << (secs % 60)
      << ".000";
  return str.str();
}

void action_browse::add_item(const content_directory::browse_item &browse_item)
{
  IXML_Element * const item = result.add_element(&didl->n, "item");
  result.set_attribute(item, "id", browse_item.id);
  result.set_attribute(item, "parentID", browse_item.parent_id);
  result.set_attribute(item, "restricted", browse_item.restricted ? "1" : "0");

  result.add_textelement(&item->n, "dc:title", browse_item.title);

  for (auto &attribute : browse_item.attributes)
  {
    IXML_Element * const e = result.add_textelement(&item->n, attribute.first, attribute.second);
    if (attribute.first == "upnp:albumArtURI")
    {
      if (ends_with(attribute.second, ".jpeg") || ends_with(attribute.second, ".jpg"))
        result.set_attribute(e, "dlna:profileID", "JPEG_TN");
      else if (ends_with(attribute.second, ".png"))
        result.set_attribute(e, "dlna:profileID", "PNG_SM");
    }
  }

  for (auto &file : browse_item.files)
  {
    IXML_Element * const res = result.add_textelement(&item->n, "res", file.first);
    result.set_attribute(res, "protocolInfo", file.second.to_string());

    if (browse_item.duration > 0)
      result.set_attribute(res, "duration", to_time(browse_item.duration));

    if (file.second.sample_rate > 0)
      result.set_attribute(res, "sampleFrequency", std::to_string(file.second.sample_rate));

    if (file.second.channels > 0)
      result.set_attribute(res, "nrAudioChannels", std::to_string(file.second.channels));

    if ((file.second.resolution_x > 0) && (file.second.resolution_y > 0))
      result.set_attribute(res, "resolution", std::to_string(file.second.resolution_x) + "x" + std::to_string(file.second.resolution_y));

    if (file.second.size > 0)
      result.set_attribute(res, "size", std::to_string(file.second.size));
  }

  number_returned++;
}

void action_browse::add_container(const content_directory::browse_container &browse_container)
{
  IXML_Element * const item = result.add_element(&didl->n, "container");
  result.set_attribute(item, "id", browse_container.id);
  result.set_attribute(item, "parentID", browse_container.parent_id);
  result.set_attribute(item, "restricted", browse_container.restricted ? "1" : "0");

  if (browse_container.child_count != size_t(-1))
    result.set_attribute(item, "childCount", std::to_string(browse_container.child_count));

  result.add_textelement(&item->n, "dc:title", browse_container.title);

  for (auto &attribute : browse_container.attributes)
    result.add_textelement(&item->n, attribute.first, attribute.second);

  number_returned++;
}

void action_browse::set_response(size_t total_matches, uint32_t update_id)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":BrowseResponse");
  set_attribute(response, "xmlns:" + prefix, content_directory::service_type);

  DOMString r = ixmlDocumenttoString(result.doc);
  add_textelement(&response->n, "Result", r);
  ixmlFreeDOMString(r);

  add_textelement(&response->n, "NumberReturned", std::to_string(number_returned));
  add_textelement(&response->n, "TotalMatches", std::to_string(total_matches));
  add_textelement(&response->n, "UpdateID", std::to_string(update_id));
}


action_search::action_search(IXML_Node *src, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    src(src),
    prefix(prefix),
    result(),
    didl(result.add_element(&result.doc->n, "DIDL-Lite")),
    number_returned(0)
{
  result.set_attribute(didl, "xmlns", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
  result.set_attribute(didl, "xmlns:dc", "http://purl.org/dc/elements/1.1/");
  result.set_attribute(didl, "xmlns:dlna", "urn:schemas-dlna-org:metadata-1-0/");
  result.set_attribute(didl, "xmlns:upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
}

std::string action_search::get_container_id() const
{
  return get_textelement(src, "ContainerID");
}

std::string action_search::get_search_criteria() const
{
  return get_textelement(src, "SearchCriteria");
}

std::string action_search::get_filter() const
{
  return get_textelement(src, "Filter");
}

size_t action_search::get_starting_index() const
{
  try { return std::stoul(get_textelement(src, "StartingIndex")); }
  catch (const std::invalid_argument &) { return size_t(-1); }
  catch (const std::out_of_range &) { return size_t(-1); }
}

size_t action_search::get_requested_count() const
{
  try { return std::stoul(get_textelement(src, "RequestedCount")); }
  catch (const std::invalid_argument &) { return size_t(-1); }
  catch (const std::out_of_range &) { return size_t(-1); }
}

std::string action_search::get_sort_criteria() const
{
  return get_textelement(src, "SortCriteria");
}

void action_search::add_item(const content_directory::browse_item &browse_item)
{
  IXML_Element * const item = result.add_element(&didl->n, "item");
  result.set_attribute(item, "id", browse_item.id);
  result.set_attribute(item, "parentID", browse_item.parent_id);
  result.set_attribute(item, "restricted", browse_item.restricted ? "1" : "0");

  result.add_textelement(&item->n, "dc:title", browse_item.title);

  for (auto &attribute : browse_item.attributes)
  {
    IXML_Element * const e = result.add_textelement(&item->n, attribute.first, attribute.second);
    if (attribute.first == "upnp:albumArtURI")
    {
      if (ends_with(attribute.second, ".jpeg") || ends_with(attribute.second, ".jpg"))
        result.set_attribute(e, "dlna:profileID", "JPEG_TN");
      else if (ends_with(attribute.second, ".png"))
        result.set_attribute(e, "dlna:profileID", "PNG_SM");
    }
  }

  for (auto &file : browse_item.files)
  {
    IXML_Element * const res = result.add_textelement(&item->n, "res", file.first);
    result.set_attribute(res, "protocolInfo", file.second.to_string());

    if (browse_item.duration > 0)
      result.set_attribute(res, "duration", to_time(browse_item.duration));

    if (file.second.sample_rate > 0)
      result.set_attribute(res, "sampleFrequency", std::to_string(file.second.sample_rate));

    if (file.second.channels > 0)
      result.set_attribute(res, "nrAudioChannels", std::to_string(file.second.channels));

    if ((file.second.resolution_x > 0) && (file.second.resolution_y > 0))
      result.set_attribute(res, "resolution", std::to_string(file.second.resolution_x) + "x" + std::to_string(file.second.resolution_y));

    if (file.second.size > 0)
      result.set_attribute(res, "size", std::to_string(file.second.size));
  }

  number_returned++;
}

void action_search::set_response(size_t total_matches, uint32_t update_id)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":SearchResponse");
  set_attribute(response, "xmlns:" + prefix, content_directory::service_type);

  DOMString r = ixmlDocumenttoString(result.doc);
  add_textelement(&response->n, "Result", r);
  ixmlFreeDOMString(r);

  add_textelement(&response->n, "NumberReturned", std::to_string(number_returned));
  add_textelement(&response->n, "TotalMatches", std::to_string(total_matches));
  add_textelement(&response->n, "UpdateID", std::to_string(update_id));
}


action_get_search_capabilities::action_get_search_capabilities(IXML_Node *, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    prefix(prefix)
{
}

void action_get_search_capabilities::set_response(const std::string &ids)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":GetSearchCapabilitiesResponse");
  set_attribute(response, "xmlns:" + prefix, content_directory::service_type);
  add_textelement(&response->n, "SearchCaps", ids);
}


action_get_sort_capabilities::action_get_sort_capabilities(IXML_Node *, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    prefix(prefix)
{
}

void action_get_sort_capabilities::set_response(const std::string &ids)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":GetSortCapabilitiesResponse");
  set_attribute(response, "xmlns:" + prefix, content_directory::service_type);
  add_textelement(&response->n, "SortCaps", ids);
}


action_get_system_update_id::action_get_system_update_id(IXML_Node *, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    prefix(prefix)
{
}

void action_get_system_update_id::set_response(uint32_t id)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":GetSystemUpdateIDResponse");
  set_attribute(response, "xmlns:" + prefix, content_directory::service_type);
  add_textelement(&response->n, "Id", std::to_string(id));
}


// Samsung GetFeatureList
action_get_featurelist::action_get_featurelist(IXML_Node *, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    prefix(prefix)
{
}

void action_get_featurelist::set_response(const std::vector<std::string> &containers)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":X_GetFeatureListResponse");
  set_attribute(response, "xmlns:" + prefix, content_directory::service_type);

  xml_structure sdoc;
  IXML_Element * const features = sdoc.add_element(&sdoc.doc->n, "Features");
  sdoc.set_attribute(features, "xmlns", "urn:schemas-upnp-org:av:avs");
  sdoc.set_attribute(features, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  sdoc.set_attribute(features, "xsi:schemaLocation", "urn:schemas-upnp-org:av:avs http://www.upnp.org/schemas/av/avs.xsd");
  IXML_Element * const feature = sdoc.add_element(&features->n, "Feature");
  sdoc.set_attribute(feature, "name", "samsung.com_BASICVIEW");
  sdoc.set_attribute(feature, "version", "1");

  for (int i = 0; i < containers.size(); i++)
  {
    IXML_Element * const e = sdoc.add_element(&feature->n, "container");
    sdoc.set_attribute(e, "id", std::to_string(i + 1));
    sdoc.set_attribute(e, "type", containers[i]);
  }

  DOMString f = ixmlDocumenttoString(sdoc.doc);
  add_textelement(&response->n, "FeatureList", f);
  ixmlFreeDOMString(f);
}


// Microsoft MediaReceiverRegistrar IsAuthorized
action_is_authorized::action_is_authorized(IXML_Node *src, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    src(src),
    prefix(prefix)
{
}

std::string action_is_authorized::get_deviceid() const
{
  return get_textelement(src, "DeviceID");
}

void action_is_authorized::set_response(int result)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":IsAuthorizedResponse");
  set_attribute(response, "xmlns:" + prefix, mediareceiver_registrar::service_type);

  add_textelement(&response->n, "Result", std::to_string(result));
  set_attribute(response, "xmlns:dt", "urn:schemas-microsoft-com:datatypes");
  set_attribute(response, "dt:dt", "int");
}


// Microsoft MediaReceiverRegistrar IsValidated
action_is_validated::action_is_validated(IXML_Node *src, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    src(src),
    prefix(prefix)
{
}

std::string action_is_validated::get_deviceid() const
{
  return get_textelement(src, "DeviceID");
}

void action_is_validated::set_response(int result)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":IsValidatedResponse");
  set_attribute(response, "xmlns:" + prefix, mediareceiver_registrar::service_type);

  add_textelement(&response->n, "Result", std::to_string(result));
  set_attribute(response, "xmlns:dt", "urn:schemas-microsoft-com:datatypes");
  set_attribute(response, "dt:dt", "int");
}


// Microsoft MediaReceiverRegistrar RegisterDevice
action_register_device::action_register_device(IXML_Node *src, IXML_Document *&dst, const std::string &prefix)
  : xml_structure(dst),
    src(src),
    prefix(prefix)
{
}

std::string action_register_device::get_registration_req_msg() const
{
  return from_base64(get_textelement(src, "RegistrationReqMsg"));
}

void action_register_device::set_response(const std::string &result)
{
  IXML_Element * const response = add_element(&doc->n, prefix + ":RegisterDeviceResponse");
  set_attribute(response, "xmlns:" + prefix, mediareceiver_registrar::service_type);

  add_textelement(&response->n, "RegistrationRespMsg", to_base64(result));
  set_attribute(response, "xmlns:dt", "urn:schemas-microsoft-com:datatypes");
  set_attribute(response, "dt:dt", "bin.Base64");
}

} // End of namespace
} // End of namespace
