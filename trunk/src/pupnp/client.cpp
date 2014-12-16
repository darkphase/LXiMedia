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

#include <upnp/upnp.h> // Include first to make sure off_t is the correct size.
#include "client.h"
#include <cstring>
#include <sstream>
#include <thread>

namespace pupnp {

client::client(class platform::messageloop_ref &messageloop, class upnp &upnp)
    : messageloop(messageloop),
      upnp(upnp),
      client_enabled(false),
      client_handle(-1)
{
    upnp.child_add(*this);
}

client::~client()
{
    client::close();

    upnp.child_remove(*this);
}

bool client::initialize()
{
    return enable_client();
}

void client::close(void)
{
    if (client_enabled)
    {
        client_enabled = false;

        // Ugly, but needed as UpnpUnRegisterClient waits for callbacks.
        bool finished = false;
        std::thread finish([this, &finished] { ::UpnpUnRegisterClient(client_handle); finished = true; });
        do messageloop.process_events(std::chrono::milliseconds(16)); while (!finished);
        finish.join();
    }
}

void client::start_search(const std::string &target, int mx)
{
    if (client_enabled)
        ::UpnpSearchAsync(client_handle, mx, target.c_str(), this);
}

std::string client::get(const std::string &location)
{
    char *buffer = NULL;
    char content_type[LINE_SIZE];
    if (::UpnpDownloadUrlItem(location.c_str(), &buffer, content_type) == UPNP_E_SUCCESS)
    {
        std::string result(buffer ? std::string(buffer) : std::string());
        ::free(buffer);
        return result;
    }

    return std::string();
}

static IXML_Node * get_element(_IXML_Node *from, const char *name)
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

static std::string get_text(_IXML_Node *from)
{
    std::ostringstream str;

    if (from)
    {
        IXML_NodeList * const children = ixmlNode_getChildNodes(from);
        for (IXML_NodeList *i = children; i; i = i->next)
            if (i->nodeItem->nodeValue)
                str << i->nodeItem->nodeValue;

        ixmlNodeList_free(children);
    }

    return str.str();
}

bool client::read_device_description(const std::string &location, struct device_description &device_description)
{
    char *buffer = NULL;
    char content_type[LINE_SIZE];
    if (::UpnpDownloadUrlItem(location.c_str(), &buffer, content_type) == UPNP_E_SUCCESS)
    {
        IXML_Document * const doc = ixmlParseBuffer(buffer);
        if (doc)
        {
            IXML_Node * const root_elm = get_element(&doc->n, "root");
            if (root_elm)
            {
                IXML_Node * const device_elm = get_element(root_elm, "device");
                if (device_elm)
                {
                    device_description.devicetype = get_text(get_element(device_elm, "deviceType"));
                    device_description.friendlyname = get_text(get_element(device_elm, "friendlyName"));
                    device_description.manufacturer = get_text(get_element(device_elm, "manufacturer"));
                    device_description.modelname = get_text(get_element(device_elm, "modelName"));
                    device_description.udn = get_text(get_element(device_elm, "UDN"));

                    device_description.presentation_url = get_text(get_element(device_elm, "presentationURL"));

                    IXML_Node * const icon_list_elm = get_element(device_elm, "iconList");
                    if (icon_list_elm)
                    {
                        IXML_Node * const icon_elm = get_element(icon_list_elm, "icon");
                        if (icon_elm)
                        {
                            device_description.icon_url = get_text(get_element(icon_elm, "url"));
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

bool client::enable_client()
{
    struct T
    {
        static int callback(Upnp_EventType event_type, void *event, void *cookie)
        {
            client * const me = reinterpret_cast<client *>(cookie);

            if ((event_type == UPNP_DISCOVERY_ADVERTISEMENT_ALIVE) ||
                (event_type == UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE) ||
                (event_type == UPNP_DISCOVERY_SEARCH_RESULT))
            {
                Upnp_Discovery * const discovery = reinterpret_cast<Upnp_Discovery *>(event);
                me->messageloop.send([me, discovery, event_type]
                {
                    if (me->client_enabled)
                    {
                        if (((event_type == UPNP_DISCOVERY_ADVERTISEMENT_ALIVE) ||
                             (event_type == UPNP_DISCOVERY_SEARCH_RESULT)) &&
                             me->device_discovered)
                        {
                            me->device_discovered(discovery->DeviceId, discovery->Location);
                        }
                        else if ((event_type == UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE) &&
                                 me->device_closed)
                        {
                            me->device_closed(discovery->DeviceId);
                        }
                    }
                });
            }

            return UPNP_E_SUCCESS;
        }
    };

    if (::UpnpRegisterClient(&T::callback, this, &client_handle) == UPNP_E_SUCCESS)
    {
        client_enabled = true;
        return true;
    }

    return false;
}

} // End of namespace
