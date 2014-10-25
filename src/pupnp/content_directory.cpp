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

#include "content_directory.h"
#include "platform/string.h"
#include "platform/translator.h"
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace pupnp {

const char content_directory::service_id[]   = "urn:upnp-org:serviceId:ContentDirectory";
const char content_directory::service_type[] = "urn:schemas-upnp-org:service:ContentDirectory:1";

content_directory::content_directory(class platform::messageloop_ref &messageloop, class upnp &upnp, class rootdevice &rootdevice, class connection_manager &connection_manager)
    : messageloop(messageloop),
      upnp(upnp),
      rootdevice(rootdevice),
      connection_manager(connection_manager),
      basedir(rootdevice.http_basedir() + "condir/"),
      update_timer(messageloop, std::bind(&content_directory::process_pending_updates, this)),
      update_timer_interval(2),
      system_update_id(0),
      allow_process_pending_updates(true),
      root_item_source(*this)
{
    using namespace std::placeholders;

    // Add the root path.
    item_sources["/"] = &root_item_source;

    objectid_list.push_back(std::string());
    objectid_map[objectid_list.back()] = objectid_list.size() - 1;

    connection_manager.numconnections_changed[this] = std::bind(&content_directory::num_connections_changed, this, _1);

    rootdevice.service_register(service_id, *this);
}

content_directory::~content_directory()
{
    content_directory::close();

    rootdevice.service_unregister(service_id);

    connection_manager.numconnections_changed.erase(this);
}

void content_directory::item_source_register(const std::string &path, struct item_source &item_source)
{
    item_sources[path] = &item_source;
    item_source_order.push_back(path);
}

void content_directory::item_source_unregister(const std::string &path)
{
    item_sources.erase(path);

    auto i = std::find(item_source_order.begin(), item_source_order.end(), path);
    if (i != item_source_order.end())
        item_source_order.erase(i);
}

void content_directory::update_system()
{
    system_update_id++;
    if (allow_process_pending_updates)
        update_timer.start(update_timer_interval, true);
}

void content_directory::update_path(const std::string &path)
{
    const std::string objectid = to_objectid(path, false);
    if (!objectid.empty() && (objectid != "0") && (objectid != "-1"))
    {
        auto container_update_id = container_update_ids.find(objectid);
        if (container_update_id != container_update_ids.end())
        {
            (container_update_id->second.id)++;
            pending_container_updates.insert(objectid);
            update_system();
        }
    }
}

void content_directory::num_connections_changed(int num_connections)
{
    allow_process_pending_updates = num_connections == 0;
    if (allow_process_pending_updates)
        update_timer.start(update_timer_interval, true);
}

void content_directory::process_pending_updates(void)
{
    if (allow_process_pending_updates)
    {
        updated_container_update_ids = std::move(pending_container_updates);
        pending_container_updates.clear();

        messageloop.post([this] { rootdevice.emit_event(service_id); });
    }
}

static const std::chrono::seconds seek_sec(120);

template<typename rep, typename period>
static std::string to_time(std::chrono::duration<rep, period> duration)
{
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    std::ostringstream str;
    str << (seconds / 3600)
        << ":" << std::setw(2) << std::setfill('0') << ((seconds % 3600) / 60)
        << ":" << std::setw(2) << std::setfill('0') << (seconds % 60);

    return str.str();
}

static std::chrono::milliseconds complete_time(std::chrono::milliseconds duration)
{
    return duration - std::max(duration / 10, std::chrono::milliseconds(60000));
}

static std::vector<std::string> playseek_items(const content_directory::item &item)
{
    std::vector<std::string> result;

    if ((item.last_position.count() > 0) &&
            (item.last_position <= complete_time(item.duration)))
    {
        result.push_back(
                    "p&position=" + std::to_string(item.last_position.count()) +
                    "#" + tr("Resume") +
                    " (" + to_time(item.last_position) +
                    "/" + to_time(item.duration) + ")");
    }
    else
    {
        result.push_back(
                    "p#" + tr("Play") +
                    " (" + to_time(item.duration) + ")");
    }

    if (item.chapters.size() > 1)
        result.push_back("c#" + tr("Chapters"));

    if (item.duration > seek_sec)
        result.push_back("s#" + tr("Seek"));

    return result;
}

static std::vector<std::string> seek_items(const content_directory::item &item)
{
    std::vector<std::string> result;

    for (std::chrono::milliseconds i(0); i<item.duration; i+=seek_sec)
    {
        const std::string title = tr("Play from") + " " + to_time(i);
        result.push_back(
                    "p&position=" + std::to_string(i.count()) +
                    "#" + ((item.last_position > (i + seek_sec)) ? ('*' + title) : title));
    }

    return result;
}

static std::vector<std::string> chapter_items(const content_directory::item &item)
{
    std::vector<std::string> result;

    for (size_t i = 0; i < item.chapters.size(); i++)
    {
        result.push_back(
                    "p&chapter=" + std::to_string(i + 1) +
                    "#" + item.chapters[i].title);
    }

    return result;
}

static std::vector<std::string> all_items(const content_directory::item &item, const std::vector<std::string> &item_props)
{
    std::vector<std::string> items;
    if (item_props.empty() || item_props[1].empty()) // Root
        items = playseek_items(item);
    else if (item_props[1] == "s")
        items = seek_items(item);
    else if (item_props[1] == "c")
        items = chapter_items(item);

    return items;
}

static std::vector<std::string> split_item_props(const std::string &text)
{
    std::vector<std::string> item_props;
    item_props.resize(4);

    size_t sep = text.find("///");
    while ((sep != text.npos) && (sep > 0) && (text[sep - 1] == ':'))
        sep = text.find("///", sep + 3);

    if (sep == text.npos)
        item_props[0] = text;
    else if (sep > 0)
        item_props[0] = text.substr(0, sep);

    for (; sep != text.npos; sep = text.find("///", sep + 3))
    {
        const std::string section = text.substr(sep + 3);
        const size_t hash = section.find_first_of('#');
        if (hash != section.npos)
        {
            item_props[1] = section.substr(0, 1);
            item_props[2] += section.substr(1, hash - 1);
            item_props[3] = section.substr(hash + 1);
        }
    }

    return item_props;
}

static content_directory::item make_play_item(const content_directory::item &base_item, const std::vector<std::string> &item_props)
{
    content_directory::item item = base_item;

    if (!item_props[3].empty())
        item.title = item_props[3];

    if (!item_props[2].empty())
    {
        std::stringstream str(item_props[2]);
        std::string qi;
        while (std::getline(str, qi, '&'))
        {
            const size_t eq = qi.find_first_of('=');
            if (eq != qi.npos)
            {
                const std::string key = qi.substr(0, eq);
                if (key == "position")
                    item.position = std::chrono::milliseconds(std::stoull(qi.substr(eq + 1)));
                else if (key == "chapter")
                    item.chapter = std::stoi(qi.substr(eq + 1));
            }
        }
    }

    return item;
}

void content_directory::handle_action(const upnp::request &request, action_browse &action)
{
    const auto objectid = action.get_object_id();
    const auto path = from_objectid(objectid);
    const auto start = action.get_starting_index();
    const auto count = action.get_requested_count();

    std::string client = request.user_agent;
    const size_t space = request.user_agent.find_first_of(' ');
    if (space != request.user_agent.npos)
        client = client.substr(0, space);

    const std::string basepath = content_directory::basepath(path);
    auto item_source = item_sources.find(basepath);
    for (std::string i=basepath; !i.empty() && (item_source == item_sources.end()); i = parentpath(i))
        item_source = item_sources.find(i);

    if ((item_source == item_sources.end()) || !starts_with(path, item_source->first))
    {
        std::clog << "[" << this << "] pupnp::content_directory: Could not find item source for path: " << std::endl;
        return;
    }

    size_t totalmatches = 0;

    if (!path.empty() && (path[path.length() - 1] == '/')) // Directory
    {
        switch (action.get_browse_flag())
        {
        case action_browse::browse_flag::direct_children:
            totalmatches = count;
            for (auto &item : item_source->second->list_contentdir_items(client, path, start, totalmatches))
            {
                std::string title = item.title;
                if (item.duration.count() > 0)
                {
                    if (item.last_position > complete_time(item.duration))
                        title = '*' + item.title;
                    else if (item.last_position.count() > 0)
                        title = '+' + item.title;
                }

                if (!item.is_dir)
                {
                    switch (item.type)
                    {
                    case item_type::none:
                    case item_type::music:
                    case item_type::music_video:
                    case item_type::image:
                    case item_type::photo:
                        add_file(action, request.url.host, *item_source->second, item, item.path, item.title);
                        break;

                    case item_type::audio_broadcast:
                    case item_type::video_broadcast:
                        add_file(action, request.url.host, *item_source->second, item, item.path, title);
                        break;

                    case item_type::audio:
                    case item_type::video:
                        if (item.duration < std::chrono::minutes(5))
                            add_file(action, request.url.host, *item_source->second, item, item.path, title);
                        else
                            add_container(action, item.type, item.path, title);

                        break;

                    case item_type::audio_book:
                    case item_type::movie:
                        add_container(action, item.type, item.path, title);
                        break;
                    }
                }
                else
                    add_directory(action, item.type, client, item.path, title);
            }
            break;

        case action_browse::browse_flag::metadata:
            add_directory(action, item_type::none, client, path);
            totalmatches = 1;
            break;
        }
    }
    else
    {
        const auto itemprops = split_item_props(path);

        // Get the item
        const item item = item_source->second->get_contentdir_item(client, itemprops[0]);
        if (item.mrl.empty())
        {
            std::clog << "[" << this << "] pupnp::content_directory: Could not find item " << itemprops[0] << std::endl;
            return;
        }

        const std::vector<std::string> items = all_items(item, itemprops);
        switch (action.get_browse_flag())
        {
        case action_browse::browse_flag::direct_children:
            // Only select the items that were requested.
            for (size_t i=start, n=0; (i<items.size()) && ((count == 0) || (n < count)); i++, n++)
            {
                const auto props = split_item_props(path + "///" + items[i]);
                if (props[1] == "p")
                    add_file(action, request.url.host, *item_source->second, make_play_item(item, props), path + "///" + items[i]);
                else
                    add_container(action, item.type, path + "///" + items[i], props[3]);
            }

            totalmatches = items.size();
            break;

        case action_browse::browse_flag::metadata:
            if (itemprops[1].empty() || (itemprops[1] == "p"))
                add_file(action, request.url.host, *item_source->second, make_play_item(item, itemprops), path);
            else
                add_container(action, item.type, path, itemprops[3]);

            totalmatches = 1;
            break;
        }
    }

    auto updateid = system_update_id;
    if (objectid != "0")
    {
        auto container_update_id = container_update_ids.find(objectid);
        if (container_update_id != container_update_ids.end())
        {
            if ((start == 0) && (totalmatches != container_update_id->second.totalmatches))
            {
                updateid = ++(container_update_id->second.id);
                container_update_id->second.totalmatches = totalmatches;
                pending_container_updates.insert(objectid);
                update_system();
            }
            else
                updateid = container_update_id->second.id;
        }
        else
            container_update_ids[objectid] = update_id { updateid, totalmatches };
    }

    action.set_response(totalmatches, updateid);
}

void content_directory::handle_action(const upnp::request &, action_search &action)
{
    action.set_response(0, system_update_id);
}

void content_directory::handle_action(const upnp::request &, action_get_search_capabilities &action)
{
    action.set_response(std::string());
}

void content_directory::handle_action(const upnp::request &, action_get_sort_capabilities &action)
{
    action.set_response(std::string());
}

void content_directory::handle_action(const upnp::request &, action_get_system_update_id &action)
{
    action.set_response(system_update_id);
}

void content_directory::handle_action(const upnp::request &, action_get_featurelist &action)
{
    std::vector<std::string> containers;
    containers.push_back("object.item.audioItem");
    containers.push_back("object.item.videoItem");
    containers.push_back("object.item.imageItem");

    action.set_response(containers);
}

const char * content_directory::get_service_type(void)
{
    return service_type;
}

void content_directory::initialize(void)
{
    using namespace std::placeholders;

    uint32_t updateid = uint32_t(::time(nullptr));
    if (updateid == system_update_id)
        updateid++;

    system_update_id = updateid;

    upnp.http_callback_register(basedir, std::bind(&content_directory::http_request, this, _1, _2, _3));
}

void content_directory::close(void)
{
    upnp.http_callback_unregister(basedir);

    update_timer.stop();
    pending_container_updates.clear();
    updated_container_update_ids.clear();
    container_update_ids.clear();

    objectid_list.clear();
    objectid_map.clear();
    objecturl_list.clear();
    objecturl_map.clear();
    objectprofile_list.clear();
    objectprofile_map.clear();

    objectid_list.push_back(std::string());
    objectid_map[objectid_list.back()] = objectid_list.size() - 1;
}

void content_directory::write_service_description(rootdevice::service_description &desc) const
{
    {
        static const char * const argname[] = { "ObjectID"            , "BrowseFlag"            , "Filter"            , "StartingIndex"   , "RequestedCount"  , "SortCriteria"            , "Result"            , "NumberReturned"  , "TotalMatches"    , "UpdateID"            };
        static const char * const argdir[]  = { "in"                  , "in"                    , "in"                , "in"              , "in"              , "in"                      , "out"               , "out"             , "out"             , "out"                 };
        static const char * const argvar[]  = { "A_ARG_TYPE_ObjectID" , "A_ARG_TYPE_BrowseFlag" , "A_ARG_TYPE_Filter" , "A_ARG_TYPE_Index", "A_ARG_TYPE_Count", "A_ARG_TYPE_SortCriteria" , "A_ARG_TYPE_Result" , "A_ARG_TYPE_Count", "A_ARG_TYPE_Count", "A_ARG_TYPE_UpdateID" };
        desc.add_action("Browse", argname, argdir, argvar);
    }
    {
        static const char * const argname[] = { "ContainerID"         , "SearchCriteria"            , "Filter"            , "StartingIndex"   , "RequestedCount"  , "SortCriteria"            , "Result"            , "NumberReturned"  , "TotalMatches"    , "UpdateID"            };
        static const char * const argdir[]  = { "in"                  , "in"                        , "in"                , "in"              , "in"              , "in"                      , "out"               , "out"             , "out"             , "out"                 };
        static const char * const argvar[]  = { "A_ARG_TYPE_ObjectID" , "A_ARG_TYPE_SearchCriteria" , "A_ARG_TYPE_Filter" , "A_ARG_TYPE_Index", "A_ARG_TYPE_Count", "A_ARG_TYPE_SortCriteria" , "A_ARG_TYPE_Result" , "A_ARG_TYPE_Count", "A_ARG_TYPE_Count", "A_ARG_TYPE_UpdateID" };
        desc.add_action("Search", argname, argdir, argvar);
    }
    {
        static const char * const argname[] = { "SearchCaps"          };
        static const char * const argdir[]  = { "out"                 };
        static const char * const argvar[]  = { "SearchCapabilities"  };
        desc.add_action("GetSearchCapabilities", argname, argdir, argvar);
    }
    {
        static const char * const argname[] = { "SortCaps"          };
        static const char * const argdir[]  = { "out"               };
        static const char * const argvar[]  = { "SortCapabilities"  };
        desc.add_action("GetSortCapabilities", argname, argdir, argvar);
    }
    {
        static const char * const argname[] = { "Id"              };
        static const char * const argdir[]  = { "out"             };
        static const char * const argvar[]  = { "SystemUpdateID"  };
        desc.add_action("GetSystemUpdateID", argname, argdir, argvar);
    }
    { // Samsung GetFeatureList
        static const char * const argname[] = { "FeatureList"             };
        static const char * const argdir[]  = { "out"                     };
        static const char * const argvar[]  = { "A_ARG_TYPE_Featurelist"  };
        desc.add_action("X_GetFeatureList", argname, argdir, argvar);
    }

    desc.add_statevariable("A_ARG_TYPE_ObjectID"      , "string", false );
    desc.add_statevariable("A_ARG_TYPE_Result"        , "string", false );
    desc.add_statevariable("A_ARG_TYPE_SearchCriteria", "string", false );
    static const char * const browseflag_values[] = { "BrowseMetadata", "BrowseDirectChildren" };
    desc.add_statevariable("A_ARG_TYPE_BrowseFlag"    , "string", false, browseflag_values);
    desc.add_statevariable("A_ARG_TYPE_Filter"        , "string", false );
    desc.add_statevariable("A_ARG_TYPE_SortCriteria"  , "string", false );
    desc.add_statevariable("A_ARG_TYPE_Index"         , "ui4"   , false );
    desc.add_statevariable("A_ARG_TYPE_Count"         , "ui4"   , false );
    desc.add_statevariable("A_ARG_TYPE_UpdateID"      , "ui4"   , false );
    desc.add_statevariable("A_ARG_TYPE_Featurelist"   , "string", false ); // Samsung GetFeatureList
    desc.add_statevariable("SearchCapabilities"       , "string", false );
    desc.add_statevariable("SortCapabilities"         , "string", false );
    desc.add_statevariable("SystemUpdateID"           , "ui4"   , true  );
    desc.add_statevariable("ContainerUpdateIDs"       , "ui4"   , true  );
    desc.add_statevariable("TransferIDs"              , "string", true  );
}

void content_directory::write_eventable_statevariables(rootdevice::eventable_propertyset &propset) const
{
    propset.add_property("SystemUpdateID", std::to_string(system_update_id));

    std::string update_ids;
    for (auto &i : updated_container_update_ids)
    {
        auto j = container_update_ids.find(i);
        if (j != container_update_ids.end())
            update_ids += ',' + j->first + ',' + std::to_string(j->second.id);
    }

    if (!update_ids.empty())
        propset.add_property("ContainerUpdateIDs", update_ids.substr(1));

    propset.add_property("TransferIDs", "");
}

int content_directory::http_request(const upnp::request &request, std::string &content_type, std::shared_ptr<std::istream> &response)
{
    if (starts_with(request.url.path, basedir))
    {
        std::string profile;
        const std::string path = from_objectpath(request.url.path, profile);
        if (!path.empty())
        {
            auto item_source = item_sources.find(path);
            for (std::string i=path; !i.empty() && (item_source == item_sources.end()); i=parentpath(i))
                item_source = item_sources.find(i);

            if ((item_source == item_sources.end()) || !starts_with(path, item_source->first))
            {
                std::clog << "[" << this << "] pupnp::content_directory: Could not find item source for path: " << std::endl;
                return upnp::http_not_found;
            }

            const auto props = split_item_props(path);
            auto item = item_source->second->get_contentdir_item(request.user_agent, props[0]);
            if (props[1] == "p")
                item = make_play_item(item, props);

            return item_source->second->play_item(request.source_address, item, profile, content_type, response);
        }
    }

    return upnp::http_not_found;
}

void content_directory::add_directory(action_browse &action, item_type type, const std::string &, const std::string &path, const std::string &title)
{
    auto item_source = item_sources.find(path);
    for (std::string i=path; !i.empty() && (item_source == item_sources.end()); i=parentpath(i))
        item_source = item_sources.find(i);

    if ((item_source == item_sources.end()) || !starts_with(path, item_source->first))
    {
        std::clog << "[" << this << "] pupnp::content_directory: Could not find item source for path: " << std::endl;
        return;
    }

    add_container(action, type, path, title);
}

void content_directory::add_container(action_browse &action, item_type type, const std::string &path, const std::string &title)
{
    const std::string parentpath = content_directory::parentpath(path);

    browse_container container;
    container.id = to_objectid(path);
    container.parent_id = to_objectid(parentpath);
    container.restricted = true;

    container.title =
            !title.empty()
            ? title
            : (!parentpath.empty()
               ? path.substr(parentpath.length(), path.length() - parentpath.length() - 1)
               : std::string("root"));

    switch (type)
    {
    case item_type::none:             container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album"))); break;

    case item_type::audio:
    case item_type::audio_broadcast:
    case item_type::audio_book:       container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album"))); break;
    case item_type::music:            container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album.musicAlbum"))); break;

    case item_type::video:
    case item_type::movie:
    case item_type::video_broadcast:  container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album"))); break;
    case item_type::music_video:      container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album.musicAlbum"))); break;

    case item_type::image:            container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album"))); break;
    case item_type::photo:            container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album.photoAlbum"))); break;
    }

    action.add_container(container);
}

void content_directory::add_file(action_browse &action, const std::string &host, struct item_source &item_source, const item &item, const std::string &path, const std::string &title)
{
    const std::string parentpath = content_directory::parentpath(path);

    struct browse_item browse_item;
    browse_item.id = to_objectid(path);
    browse_item.parent_id = to_objectid(parentpath);
    browse_item.restricted = true;
    browse_item.title = !title.empty() ? title : item.title;
    browse_item.duration = item.duration - item.position;

    if (!item.artist.empty())
        browse_item.attributes.push_back(std::make_pair(std::string("upnp:artist"), item.artist));

    if (!item.album.empty())
        browse_item.attributes.push_back(std::make_pair(std::string("upnp:album"), item.album));

    //  if (!item.icon_url.path.empty())
    //  {
    //    if (ends_with(item.icon_url.path, ".jpeg") || ends_with(item.icon_url.path, ".jpg"))
    //      browse_item.attributes.push_back(std::make_pair(std::string("upnp:albumArtURI"), to_objecturl(item.icon_url, ".jpeg")));
    //    else
    //      browse_item.attributes.push_back(std::make_pair(std::string("upnp:albumArtURI"), to_objecturl(item.icon_url, ".png")));
    //  }

    switch (item.type)
    {
    case item_type::none:             browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item"))); break;

    case item_type::audio:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.audioItem"))); break;
    case item_type::music:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.audioItem.musicTrack"))); break;
    case item_type::audio_broadcast:  browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.audioItem.audioBroadcast"))); break;
    case item_type::audio_book:       browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.audioItem.audioBook"))); break;

    case item_type::video:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.videoItem"))); break;
    case item_type::movie:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.videoItem.movie"))); break;
    case item_type::video_broadcast:  browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.videoItem.videoBroadcast"))); break;
    case item_type::music_video:      browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.videoItem.musicVideoClip"))); break;

    case item_type::image:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.imageItem"))); break;
    case item_type::photo:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.imageItem.photo"))); break;
    }

    if (!item.mrl.empty())
    {
        std::vector<connection_manager::protocol> protocols;
        if (item.is_audio())  protocols = connection_manager.get_protocols(item.channels);
        if (item.is_video())  protocols = connection_manager.get_protocols(item.channels, item.width, item.frame_rate);

        for (auto protocol : protocols)
            if (item_source.correct_protocol(item, protocol))
            {
                upnp::url url;
                url.host = host;
                url.path = to_objectpath(path, protocol.profile, protocol.suffix);
                browse_item.files.push_back(std::make_pair(url, protocol));
            }
    }

    action.add_item(browse_item);
}

std::string content_directory::basepath(const std::string &dir)
{
    const size_t ls = dir.find_last_of('/');
    if (ls != dir.npos)
        return dir.substr(0, ls + 1);

    return std::string();
}

std::string content_directory::parentpath(const std::string &dir)
{
    if (dir.length() > 1)
    {
        const size_t ls = dir.find_last_of('/' , dir.length() - 2);
        if (ls != dir.npos)
        {
            if ((ls > 0) && (dir[ls - 1] == '/'))
                return dir.substr(0, ls - 1);
            else
                return dir.substr(0, ls + 1);
        }
    }

    return std::string();
}

std::string content_directory::to_objectid(const std::string &path, bool create)
{
    if (path == "/")
    {
        return "0";
    }
    else if (path.empty())
    {
        return "-1";
    }
    else
    {
        auto i = objectid_map.find(path);
        if (i != objectid_map.end())
            return std::to_string(i->second);

        if (create)
        {
            objectid_list.push_back(path);
            objectid_list.back().shrink_to_fit();
            objectid_map[objectid_list.back()] = objectid_list.size() - 1;

            return std::to_string(objectid_list.size() - 1);
        }
    }

    return std::string();
}

std::string content_directory::from_objectid(const std::string &id_str)
{
    if (id_str == "0")
    {
        return "/";
    }
    else if (id_str == "-1")
    {
        return std::string();
    }
    else
    {
        try
        {
            const size_t id = std::stoull(id_str);
            if (id < objectid_list.size())
                return objectid_list[id];
        }
        catch (const std::invalid_argument &) { }
        catch (const std::out_of_range &) { }

        return std::string();
    }
}

std::string content_directory::to_objectpath(const std::string &path, const std::string &profile, const std::string &suffix)
{
    size_t profile_id = 0;
    {
        auto i = objectprofile_map.find(profile);
        if (i == objectprofile_map.end())
        {
            objectprofile_list.push_back(profile);
            objectprofile_list.back().shrink_to_fit();
            objectprofile_map[objectprofile_list.back()] = profile_id = objectprofile_list.size() - 1;
        }
        else
            profile_id = i->second;
    }

    size_t path_id = 0;
    {
        auto i = objecturl_map.find(path);
        if (i == objecturl_map.end())
        {
            objecturl_list.push_back(path);
            objecturl_list.back().shrink_to_fit();
            objecturl_map[objecturl_list.back()] = path_id = objecturl_list.size() - 1;
        }
        else
            path_id = i->second;
    }

    std::ostringstream str;
    str << basedir << std::hex << std::setfill('0')
        << std::setw(5) << path_id << '-'
        << std::setw(2) << profile_id << '.' << suffix;
    return str.str();
}

std::string content_directory::from_objectpath(const std::string &path, std::string &profile)
{
    const size_t ls = path.find_last_of('/');
    if (ls != path.npos)
    {
        const size_t dash = path.find_first_of('-', ls);
        const size_t dot = path.find_first_of('.', dash);
        if ((dash != path.npos) && (dot != path.npos))
        {
            const std::string path_id_str = path.substr(ls + 1, dash - ls - 1);
            const std::string profile_id_str = path.substr(dash + 1, dot - dash - 1);

            try
            {
                const size_t path_id = std::stoull(path_id_str, nullptr, 16);
                const size_t profile_id = std::stoull(profile_id_str, nullptr, 16);

                if (profile_id < objectprofile_list.size())
                {
                    profile = objectprofile_list[profile_id];
                    if (path_id < objecturl_list.size())
                        return objecturl_list[path_id];
                }
            }
            catch (const std::invalid_argument &) { }
            catch (const std::out_of_range &) { }
        }
    }

    return std::string();
}


content_directory::item::item(void)
    : is_dir(false), type(item_type::none), track(0),
      sample_rate(0), channels(0),
      width(0), height(0), frame_rate(0.0f),
      chapter(0), duration(0), position(0), last_position(0)
{
}

content_directory::item::~item()
{
}

bool content_directory::item::is_audio(void) const
{
    return (type >= item_type::audio) && (type < item_type::video);
}

bool content_directory::item::is_video(void) const
{
    return (type >= item_type::video) && (type < item_type::image);
}

bool content_directory::item::is_image(void) const
{
    return (type >= item_type::image) && (type <= item_type::photo);
}

bool content_directory::item::is_music(void) const
{
    return (type == item_type::music) || (type == item_type::music_video);
}


content_directory::browse_item::browse_item()
    : restricted(true),
      duration(0)
{
}

content_directory::browse_item::~browse_item()
{
}


content_directory::browse_container::browse_container()
    : restricted(true),
      child_count(size_t(-1))
{
}

content_directory::browse_container::~browse_container()
{
}


std::vector<content_directory::item> content_directory::root_item_source::list_contentdir_items(const std::string &client, const std::string &path, size_t start, size_t &count)
{
    const bool return_all = count == 0;
    std::vector<content_directory::item> result;
    std::set<std::string> names;

    for (auto &i : parent.item_source_order)
    {
        const auto item_source = parent.item_sources.find(i);
        if ((item_source != parent.item_sources.end()) && starts_with(item_source->first, path))
        {
            auto item = get_contentdir_item(client, item_source->first);
            if (!item.title.empty() && (names.find(item.title) == names.end()))
            {
                size_t total = 1;
                if (!item_source->second->list_contentdir_items(client, item_source->first, 0, total).empty() && (total > 0))
                {
                    names.insert(item.title);
                    if (return_all || (count > 0))
                    {
                        if (start == 0)
                        {
                            result.push_back(item);
                            if (count > 0)
                                count--;
                        }
                        else
                            start--;
                    }
                }
            }
        }
    }

    count = names.size();
    return result;
}

content_directory::item content_directory::root_item_source::get_contentdir_item(const std::string &, const std::string &path)
{
    struct item item;
    item.is_dir = true;
    item.path = path;

    const size_t lsl = std::max(path.find_last_of('/'), path.length() - 1);
    const size_t psl = path.find_last_of('/', lsl - 1);
    item.title = path.substr(psl + 1, lsl - psl - 1);

    return item;
}

bool content_directory::root_item_source::correct_protocol(const item &, connection_manager::protocol &)
{
    return true;
}

int content_directory::root_item_source::play_item(const std::string &, const item &, const std::string &, std::string &, std::shared_ptr<std::istream> &)
{
    return upnp::http_not_found;
}

} // End of namespace
