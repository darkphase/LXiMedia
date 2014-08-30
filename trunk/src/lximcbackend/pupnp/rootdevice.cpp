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

#include "rootdevice.h"
#include "ixml_structures.h"
#include "../string.h"
#include <upnp/upnp.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>
#include <thread>

namespace pupnp {

const char  rootdevice::devicedescriptionfile[]   = "device";
const char  rootdevice::servicedescriptionfile[]  = "service-";
const char  rootdevice::servicecontrolfile[]      = "control-";
const char  rootdevice::serviceeventfile[]        = "event-";

rootdevice::rootdevice(class messageloop &messageloop, class upnp &upnp, const std::string &uuid, const std::string &devicetype)
    : messageloop(messageloop),
      upnp(upnp),
      uuid(uuid),
      devicetype(devicetype),
      basedir(upnp.http_basedir()),
      devicename(uuid),
      initialized(false),
      rootdevice_registred(false)
{
    upnp.child_add(*this);
}

rootdevice::~rootdevice()
{
    rootdevice::close();

    upnp.child_remove(*this);
}

const std::string &rootdevice::http_basedir() const
{
    return basedir;
}

void rootdevice::set_devicename(const std::string &devicename)
{
    this->devicename = devicename;
}

void rootdevice::add_icon(const std::string &path)
{
    icons.push_back(path);
}

void rootdevice::service_register(const std::string &service_id, struct service &service)
{
    std::string ext;
    for (size_t lc = service_id.find_last_of(":_"); (lc != service_id.npos) && (lc < service_id.length()); lc++)
        if ((service_id[lc] >= 'A') && (service_id[lc] <= 'Z'))
            ext.push_back(service_id[lc] + ('a' - 'A'));

    std::set<std::string> exts;
    for (auto &i : services)
        exts.insert(i.second.second);

    static const char letter[] = "abcdefghijklmnopqrstuvwxyz";
    int n = 0;
    while (letter[n] && (exts.find(ext + letter[n]) != exts.end()))
        n++;

    if (letter[n])
        services[service_id] = std::make_pair(&service, ext + letter[n]);
}

void rootdevice::service_unregister(const std::string &service_id)
{
    services.erase(service_id);
}

bool rootdevice::initialize(void)
{
    if (!initialized)
    {
        using namespace std::placeholders;

        upnp.http_callback_register(basedir, std::bind(&rootdevice::http_request, this, _1, _2, _3));

        for (auto i : services)
            i.second.first->initialize();

        initialized = true;

        return enable_rootdevice();
    }

    return initialized;
}

void rootdevice::close(void)
{
    if (initialized)
    {
        upnp.http_callback_unregister(basedir);

        if (rootdevice_registred)
        {
            rootdevice_registred = false;
            for (auto &handle : rootdevice_handles)
                ::UpnpUnRegisterRootDevice(handle.second);
        }

        for (auto i : services)
            i.second.first->close();

        initialized = false;
    }
}

void rootdevice::emit_event(const std::string &service_id)
{
    if (services.find(service_id) != services.end())
    {
        ixml_structures::eventable_propertyset propertyset;
        handle_event(service_id, propertyset);

        for (auto &handle : rootdevice_handles)
        {
            ::UpnpNotifyExt(
                        handle.second,
                        udn().c_str(),
                        service_id.c_str(),
                        propertyset.doc);
        }
    }
}

std::string rootdevice::udn() const
{
    return "uuid:" + uuid;
}

void rootdevice::handle_event(const std::string &service_id, eventable_propertyset &propset)
{
    auto i = services.find(service_id);
    if (i != services.end())
        i->second.first->write_eventable_statevariables(propset);
}

void rootdevice::write_device_description(device_description &desc)
{
    desc.set_devicetype(devicetype, "DMS-1.50");
    desc.set_friendlyname(devicename);
    desc.set_manufacturer("LeX-Interactive", "http://lximedia.sf.net/");
    desc.set_model("LXiMediaCenter Backend", "LXiMediaCenter", "http://lximedia.sf.net/", "0.5.0");
    desc.set_serialnumber(uuid);
    desc.set_udn(udn());

    //  for (auto &path : icons)
    //  {
    //    const QImage icon(':' + path);
    //    if (!icon.isNull())
    //      desc.addIcon(basedir + path, UPnP::toMimeType(path), icon.width(), icon.height(), icon.depth());
    //  }

    desc.set_presentation_url("/");
}

int rootdevice::http_request(const upnp::request &request, std::string &content_type, std::shared_ptr<std::istream> &response)
{
    if (starts_with(request.url.path, basedir + devicedescriptionfile))
    {
        ixml_structures::device_description desc(request.url.host, "/");
        write_device_description(desc);

        for (auto &i : services)
        {
            desc.add_service(
                        i.second.first->get_service_type(),
                        i.first,
                        basedir + servicedescriptionfile + i.second.second + ".xml",
                        basedir + servicecontrolfile + i.second.second,
                        basedir + serviceeventfile + i.second.second);
        }

        auto buffer = std::make_shared<std::stringstream>();
        DOMString s = ixmlDocumenttoString(desc.doc);
        *buffer << s;
        ixmlFreeDOMString(s);
        content_type = upnp::mime_text_xml;
        response = buffer;

        return upnp::http_ok;
    }
    else if (starts_with(request.url.path, basedir + servicedescriptionfile))
    {
        const size_t l = basedir.length() + strlen(servicedescriptionfile);
        const std::string path = request.url.path;
        const std::string ext = path.substr(l, path.length() - 4 - l);

        for (auto &i : services)
        {
            if (i.second.second == ext)
            {
                ixml_structures::service_description desc;
                i.second.first->write_service_description(desc);

                auto buffer = std::make_shared<std::stringstream>();
                DOMString s = ixmlDocumenttoString(desc.doc);
                *buffer << s;
                ixmlFreeDOMString(s);
                content_type = upnp::mime_text_xml;
                response = buffer;

                return upnp::http_ok;
            }
        }
    }
    else for (auto &icon : icons)
    {
        const std::string iconpath = basedir + icon;
        if (starts_with(request.url.path, iconpath))
        {
            upnp::request r = request;
            r.url.path = upnp::url("http://" + request.url.host + "/" + icon);

            return upnp.handle_http_request(r, content_type, response);
        }
    }

    return upnp::http_not_found;
}

static void split_name(const std::string &fullname, std::string &prefix, std::string &name)
{
    const size_t colon = fullname.find_first_of(':');
    if (colon != fullname.npos)
    {
        prefix = fullname.substr(0, colon);
        name = fullname.substr(colon + 1);
    }
    else
        name = fullname;
}

bool rootdevice::enable_rootdevice(void)
{
    struct T
    {
        static int callback(Upnp_EventType eventtype, void *event, void *cookie)
        {
            rootdevice * const me = reinterpret_cast<rootdevice *>(cookie);

            if (eventtype == UPNP_CONTROL_ACTION_REQUEST)
            {
                Upnp_Action_Request * const action_request = reinterpret_cast<Upnp_Action_Request *>(event);
                me->messageloop.send([me, action_request]
                {
                    auto i = me->services.find(action_request->ServiceID);
                    if ((i != me->services.end()) && me->rootdevice_registred)
                    {
                        upnp::request request;
                        request.user_agent = action_request->RequestInfo.userAgent;
                        request.source_address = action_request->RequestInfo.sourceAddress;
                        request.url.host = action_request->RequestInfo.host;

                        if ((strcmp(i->second.first->get_service_type(), connection_manager::service_type) == 0))
                        {
                            connection_manager * const service = static_cast<connection_manager *>(i->second.first);

                            IXML_NodeList * const children = ixmlNode_getChildNodes(&action_request->ActionRequest->n);
                            for (IXML_NodeList *i = children; i; i = i->next)
                            {
                                std::string prefix = "ns0", name;
                                split_name(i->nodeItem->nodeName, prefix, name);

                                if (name == "GetCurrentConnectionIDs")
                                {
                                    ixml_structures::action_get_current_connectionids action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                                else if (name == "GetCurrentConnectionInfo")
                                {
                                    ixml_structures::action_get_current_connection_info action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                                else if (name == "GetProtocolInfo")
                                {
                                    ixml_structures::action_get_protocol_info action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                            }

                            ixmlNodeList_free(children);
                        }
                        else if ((strcmp(i->second.first->get_service_type(), content_directory::service_type) == 0))
                        {
                            content_directory * const service = static_cast<content_directory *>(i->second.first);

                            IXML_NodeList * const children = ixmlNode_getChildNodes(&action_request->ActionRequest->n);
                            for (IXML_NodeList *i = children; i; i = i->next)
                            {
                                std::string prefix = "ns0", name;
                                split_name(i->nodeItem->nodeName, prefix, name);

                                if (name == "Browse")
                                {
                                    ixml_structures::action_browse action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                                else if (name == "Search")
                                {
                                    ixml_structures::action_search action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                                else if (name == "GetSearchCapabilities")
                                {
                                    ixml_structures::action_get_search_capabilities action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                                else if (name == "GetSortCapabilities")
                                {
                                    ixml_structures::action_get_sort_capabilities action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                                else if (name == "GetSystemUpdateID")
                                {
                                    ixml_structures::action_get_system_update_id action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                                else if (name == "X_GetFeatureList")
                                {
                                    ixml_structures::action_get_featurelist action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                            }

                            ixmlNodeList_free(children);
                        }
                        else if ((strcmp(i->second.first->get_service_type(), mediareceiver_registrar::service_type) == 0))
                        {
                            mediareceiver_registrar * const service = static_cast<mediareceiver_registrar *>(i->second.first);

                            IXML_NodeList * const children = ixmlNode_getChildNodes(&action_request->ActionRequest->n);
                            for (IXML_NodeList *i = children; i; i = i->next)
                            {
                                std::string prefix = "ns0", name;
                                split_name(i->nodeItem->nodeName, prefix, name);

                                if (name == "IsAuthorized")
                                {
                                    ixml_structures::action_is_authorized action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                                else if (name == "IsValidated")
                                {
                                    ixml_structures::action_is_validated action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                                else if (name == "RegisterDevice")
                                {
                                    ixml_structures::action_register_device action(i->nodeItem, action_request->ActionResult, prefix);
                                    service->handle_action(request, action);
                                }
                            }

                            ixmlNodeList_free(children);
                        }
                    }

                    for (auto &i : me->handled_action) if (i.second) i.second();
                });

                return 0;
            }
            else if (eventtype == UPNP_EVENT_SUBSCRIPTION_REQUEST)
            {
                Upnp_Subscription_Request * const subscription_request = reinterpret_cast<Upnp_Subscription_Request *>(event);
                me->messageloop.send([me, subscription_request]
                {
                    if (me->rootdevice_registred)
                    {
                        const std::string udn = me->udn();
                        ixml_structures::eventable_propertyset propertyset;

                        me->handle_event(subscription_request->ServiceId, propertyset);

                        auto i = me->rootdevice_handles.find(subscription_request->Host);
                        if (i != me->rootdevice_handles.end())
                            ::UpnpAcceptSubscriptionExt(i->second, udn.c_str(), subscription_request->ServiceId, propertyset.doc, subscription_request->Sid);
                    }
                });

                return 0;
            }

            std::clog << "[" << me << "] pupnp::rootdevice: Unsupported eventtype" << eventtype << std::endl;
            return -1;
        }
    };

    // Ugly, but needed as UpnpRegisterRootDevice retrieves files from the HTTP server.
    std::atomic<bool> finished(false);
    int result = UPNP_E_INTERNAL_ERROR;
    char **addresses = ::UpnpGetServerIpAddresses();
    const std::string port = std::to_string(::UpnpGetServerPort());
    std::thread start([this, &finished, &result, addresses, &port]
    {
        for (char **i = addresses; i && *i; i++)
        {
            const std::string host = std::string(*i) + ":" + port;
            if (::UpnpRegisterRootDevice(
                        ("http://" + host + basedir + devicedescriptionfile + ".xml").c_str(),
                        &T::callback, this,
                        &(rootdevice_handles[host])) == UPNP_E_SUCCESS)
            {
                result = UPNP_E_SUCCESS;
            }
        }

        finished = true;
    });

    while (!finished) messageloop.process_events(std::chrono::milliseconds(16));
    start.join();

    if (result == UPNP_E_SUCCESS)
    {
        rootdevice_registred = true;
        messageloop.post([this]
        {
            if (rootdevice_registred)
                for (auto &handle : rootdevice_handles)
                    ::UpnpSendAdvertisement(handle.second, 1800);
        });

        return true;
    }
    else
        std::clog << "[" << this << "] pupnp::rootdevice: UpnpRegisterRootDevice failed:" << result << std::endl;

    return false;
}

} // End of namespace
