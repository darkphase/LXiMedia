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
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace lximediacenter {

static std::string from_base64(const std::string &input)
{
  static const uint8_t table[256] =
  {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  62,0x00,0x00,0x00,  63,
      52,  53,  54,  55,  56,  57,  58,  59,  60,  61,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
      15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,0x00,0x00,0x00,0x00,0x00,
    0x00,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
      41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,0x00,0x00,0x00,0x00,0x00
    // Remainder will be initialized to zero.
  };

  std::string result;
  result.reserve(input.length() * 3 / 4);

  for (size_t i = 0; i < input.length(); i += 4)
  {
    const bool has[] =
    {
                                 input[i  ] != '=' ,
      (i+1 < input.length()) && (input[i+1] != '='),
      (i+2 < input.length()) && (input[i+2] != '='),
      (i+3 < input.length()) && (input[i+3] != '='),
    };

    uint32_t triple = 0;
    for (int j = 0; j < 4; j++)
      triple |= uint32_t(has[j] ? table[uint8_t(input[i + j])] : 0) << (18 - (j * 6));

    for (int j = 0; (j < 3) && has[j + 1]; j++)
      result.push_back(char(triple >> (16 - (j * 8)) & 0xFF));
  }

  return result;
}

const char content_directory::service_id[]   = "urn:upnp-org:serviceId:ContentDirectory";
const char content_directory::service_type[] = "urn:schemas-upnp-org:service:ContentDirectory:1";

content_directory::content_directory(class messageloop &messageloop, class upnp &upnp, class rootdevice &rootdevice, class connection_manager &connection_manager)
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
  rootdevice.service_unregister(service_id);

  connection_manager.numconnections_changed.erase(this);
}

void content_directory::item_source_register(const std::string &path, struct item_source &item_source)
{
  item_sources[path] = &item_source;
}

void content_directory::item_source_unregister(const std::string &path)
{
  item_sources.erase(path);
}

void content_directory::update_system()
{
  const uint32_t update_id = ::time(nullptr);
  if (update_id != system_update_id)
  {
    system_update_id = update_id;

    messageloop.post([this] { rootdevice.emit_event(service_id); });
  }
}

void content_directory::update_path(const std::string &path)
{
  const std::string objectid = to_objectid(path, false);
  if (!objectid.empty() && (objectid != "0") && (objectid != "-1"))
  {
    if (pending_container_updates.find(objectid) != pending_container_updates.end())
    {
      pending_container_updates.insert(objectid);
      if (allow_process_pending_updates)
        update_timer.start(update_timer_interval, true);
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
    const uint32_t update_id = ::time(nullptr);

    for (auto &objectid : pending_container_updates)
      container_update_ids[objectid] = update_id;

    if (pending_container_updates.empty())
      messageloop.post([this] { rootdevice.emit_event(service_id); });

    pending_container_updates.clear();
  }
}

static const unsigned seek_sec = 120;

static std::string to_time(int secs)
{
  std::ostringstream str;
  str << (secs / 60) << ":" << std::setw(2) << std::setfill('0') << (secs % 60);
  return str.str();
}

static std::vector<std::string> playseek_items(const content_directory::item &item)
{
  std::vector<std::string> result;

  if ((item.last_position > 0) && (item.last_position <= int(item.duration - (item.duration / 10))))
  {
    result.push_back(
          "p&position=" + std::to_string(item.last_position) +
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

static std::vector<std::string> stream_items(const content_directory::item &item)
{
  if (item.streams.size() > 1)
  {
    std::vector<std::string> result;
    for (const content_directory::item::stream &stream : item.streams)
    {
      std::string query;
      for (size_t i=0; i<stream.query_items.size(); i++)
        query += '&' + stream.query_items[i].first + '=' + stream.query_items[i].second;

      result.push_back('r' + query + '#' + stream.title);
    }

    return result;
  }

  return playseek_items(item);
}

static std::vector<std::string> seek_items(const content_directory::item &item)
{
  std::vector<std::string> result;

  for (unsigned i=0; i<item.duration; i+=seek_sec)
  {
    const std::string title = tr("Play from") + " " + to_time(i);
    result.push_back(
          "p&position=" + std::to_string(i) +
          "#" + ((item.last_position > int(i + seek_sec)) ? ('*' + title) : title));
  }

  return result;
}

static std::vector<std::string> chapter_items(const content_directory::item &item)
{
  std::vector<std::string> result;

  int chapter_num = 1;
  for (size_t i = 0; i < item.chapters.size(); i++)
  {
    const content_directory::item::chapter &chapter = item.chapters[i];

    std::string title = tr("Chapter") + " " + std::to_string(chapter_num++);
    if (!chapter.title.empty())
      title += ", " + chapter.title;

    const bool seen =
        ((i + 1) < item.chapters.size())
        ? (item.last_position >= int(item.chapters[i + 1].position))
        : (item.last_position >= int(item.chapters[i].position));

    result.push_back(
          "p&position=" + std::to_string(chapter.position) +
          "#" + (seen ? ('*' + title) : title));
  }

  return result;
}

static std::vector<std::string> all_items(const content_directory::item &item, const std::vector<std::string> &item_props)
{
  std::vector<std::string> items;
  if (item_props.empty() || item_props[1].empty()) // Root
    items = stream_items(item);
  else if (item_props[1] == "r")
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

  std::stringstream str(text);
  std::string section;
  while (std::getline(str, section, '\t'))
  {
    const size_t hash = section.find_first_of('#');
    if (hash != section.npos)
    {
      item_props[1] = section.substr(0, 1);
      item_props[2] += section.substr(1, hash - 1);
      item_props[3] = section.substr(hash + 1);
    }
    else
      item_props[0] = section.empty() ? item_props[0] : section;
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
    item.url.query.clear();

    std::stringstream str(item_props[2]);
    std::string qi;
    while (std::getline(str, qi, '&'))
    {
      const size_t eq = qi.find_first_of('=');
      if (eq != qi.npos)
        item.url.query[qi.substr(0, eq)] = qi.substr(eq + 1);
      else
        item.url.query[qi] = std::string();
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

  client += '@' + request.source_address;

  const std::string basepath = content_directory::basepath(path);
  auto item_source = item_sources.find(basepath);
  for (std::string i=basepath; !i.empty() && (item_source == item_sources.end()); i = parentpath(i))
    item_source = item_sources.find(i);

  if ((item_source == item_sources.end()) || !starts_with(path, item_source->first))
  {
    std::clog << "content_directory: could not find item source for path:" << std::endl;
    return;
  }

  size_t totalmatches = 0;

  if (!path.empty() && (path[path.length() - 1] == '/')) // Directory
  {
    switch (action.get_browse_flag())
    {
    case action_browse::direct_children:
      totalmatches = count;
      for (auto &item : item_source->second->list_contentdir_items(client, path, start, totalmatches))
      {
        if (!item.is_dir)
        {
          std::string title;
          if (item.last_position > int(item.duration - (item.duration / 10)))
            title = '*' + item.title;
          else if (item.last_position > 0)
            title = '+' + item.title;
          else
            title = item.title;

          switch (item.type)
          {
          case item::item_type::none:
          case item::item_type::music:
          case item::item_type::music_video:
          case item::item_type::image:
          case item::item_type::photo:
            add_file(action, request.url.host, item, item.path, item.title);
            break;

          case item::item_type::audio_broadcast:
          case item::item_type::video:
          case item::item_type::video_broadcast:
            add_file(action, request.url.host, item, item.path, title);
            break;

          case item::item_type::audio:
          case item::item_type::audio_book:
          case item::item_type::movie:
            add_container(action, item.type, item.path, title);
            break;
          }
        }
        else
          add_directory(action, item.type, client, item.path);
      }
      break;

    case action_browse::metadata:
      add_directory(action, item::item_type::none, client, path);
      totalmatches = 1;
      break;
    }
  }
  else
  {
    const auto itemprops = split_item_props(path);

    // Get the item
    const item item = item_source->second->get_contentdir_item(client, itemprops[0]);
    if (item.is_null())
    {
      std::clog << "content_directory: could not find item" << itemprops[0] << std::endl;
      return;
    }

    const std::vector<std::string> items = all_items(item, itemprops);
    switch (action.get_browse_flag())
    {
    case action_browse::direct_children:
      // Only select the items that were requested.
      for (size_t i=start, n=0; (i<items.size()) && ((count == 0) || (n < count)); i++, n++)
      {
        const std::vector<std::string> props = split_item_props(path + '\t' + items[i]);
        if (props[1] == "p")
          add_file(action, request.url.host, make_play_item(item, props), path + '\t' + items[i]);
        else
          add_container(action, item.type, path + '\t' + items[i], props[3], all_items(item, split_item_props(items[i])).size());
      }

      totalmatches = items.size();
      break;

    case action_browse::metadata:
      if (itemprops[1].empty() || (itemprops[1] == "p"))
        add_file(action, request.url.host, make_play_item(item, itemprops), path);
      else
        add_container(action, item.type, path, itemprops[3], items.size());

      totalmatches = 1;
      break;
    }
  }

  auto updateid = system_update_id;
  if (objectid != "0")
  {
    auto container_update_id = container_update_ids.find(objectid);
    if (container_update_id != container_update_ids.end())
      updateid = container_update_id->second;
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

  auto updateid = ::time(nullptr);
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
  container_update_ids.clear();

  objectid_list.clear();
  objectid_map.clear();
  objecturl_list.clear();
  objecturl_map.clear();

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
  for (auto &i : container_update_ids)
    update_ids += ',' + i.first + ',' + std::to_string(i.second);

  propset.add_property("ContainerUpdateIDs", update_ids.empty() ? update_ids : update_ids.substr(1));

  propset.add_property("TransferIDs", "");
}

int content_directory::http_request(const upnp::request &request, std::string &content_type, std::shared_ptr<std::istream> &response)
{
  if (starts_with(request.url.path, basedir))
  {
    upnp::request r = request;
    r.url = from_objecturl(request.url);

    const auto result = upnp.handle_http_request(r, content_type, response);
    if (result == upnp::http_ok)
      connection_manager.output_connection_add(content_type, response);

    return result;
  }

  return upnp::http_not_found;
}

void content_directory::add_directory(action_browse &action, item::item_type type, const std::string &, const std::string &path, const std::string &title)
{
  auto item_source = item_sources.find(path);
  for (std::string i=path; !i.empty() && (item_source == item_sources.end()); i=parentpath(i))
    item_source = item_sources.find(i);

  if ((item_source == item_sources.end()) || !starts_with(path, item_source->first))
  {
    std::clog << "content_directory: could not find item source for path:" << std::endl;
    return;
  }

  add_container(action, type, path, title);
}

void content_directory::add_container(action_browse &action, item::item_type type, const std::string &path, const std::string &title, size_t child_count)
{
  const std::string parentpath = content_directory::parentpath(path);

  browse_container container;
  container.id = to_objectid(path);
  container.parent_id = to_objectid(parentpath);
  container.restricted = true;
  container.child_count = child_count;

  container.title =
      !title.empty()
          ? title
          : (!parentpath.empty()
                ? path.substr(parentpath.length(), path.length() - parentpath.length() - 1)
                : std::string("root"));

  switch (type)
  {
  case item::item_type::none:             container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album"))); break;

  case item::item_type::audio:
  case item::item_type::audio_broadcast:
  case item::item_type::audio_book:       container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album"))); break;
  case item::item_type::music:            container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album.musicAlbum"))); break;

  case item::item_type::video:
  case item::item_type::movie:
  case item::item_type::video_broadcast:  container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album"))); break;
  case item::item_type::music_video:      container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album.musicAlbum"))); break;

  case item::item_type::image:            container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album"))); break;
  case item::item_type::photo:            container.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.container.album.photoAlbum"))); break;
  }

  action.add_container(container);
}
static std::string to_base64(const std::string &input)
{
  static const char table[64] =
  {
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
  };

  std::string result;
  result.reserve(4 * ((input.length() + 2) / 3));

  for (size_t i = 0; i < input.length(); i += 3)
  {
    const bool has[] = { true, true, i + 1 < input.length(), i + 2 < input.length() };

    uint32_t triple = 0;
    for (int j = 0; j < 3; j++)
      triple |= uint32_t(has[j+1] ? uint8_t(input[i + j]) : 0) << (j * 8);

    for (int j = 0; (j < 4) && has[j]; j++)
      result.push_back(table[(triple >> (18 - (j * 6))) & 0x3F]);
  }

  return result;
}

void content_directory::add_file(action_browse &action, const std::string &host, const item &item, const std::string &path, const std::string &title)
{
  const std::string parentpath = content_directory::parentpath(path);

  struct browse_item browse_item;
  browse_item.id = to_objectid(path);
  browse_item.parent_id = to_objectid(parentpath);
  browse_item.restricted = true;
  browse_item.title = !title.empty() ? title : item.title;

  if (!item.artist.empty())
    browse_item.attributes.push_back(std::make_pair(std::string("upnp:artist"), item.artist));

  if (!item.album.empty())
    browse_item.attributes.push_back(std::make_pair(std::string("upnp:album"), item.album));

  if (!item.icon_url.path.empty())
  {
    if (ends_with(item.icon_url.path, ".jpeg") || ends_with(item.icon_url.path, ".jpg"))
      browse_item.attributes.push_back(std::make_pair(std::string("upnp:albumArtURI"), to_objecturl(item.icon_url, ".jpeg")));
    else
      browse_item.attributes.push_back(std::make_pair(std::string("upnp:albumArtURI"), to_objecturl(item.icon_url, ".png")));
  }

  switch (item.type)
  {
  case item::item_type::none:             browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item"))); break;

  case item::item_type::audio:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.audioItem"))); break;
  case item::item_type::music:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.audioItem.musicTrack"))); break;
  case item::item_type::audio_broadcast:  browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.audioItem.audioBroadcast"))); break;
  case item::item_type::audio_book:       browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.audioItem.audioBook"))); break;

  case item::item_type::video:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.videoItem"))); break;
  case item::item_type::movie:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.videoItem.movie"))); break;
  case item::item_type::video_broadcast:  browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.videoItem.videoBroadcast"))); break;
  case item::item_type::music_video:      browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.videoItem.musicVideoClip"))); break;

  case item::item_type::image:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.imageItem"))); break;
  case item::item_type::photo:            browse_item.attributes.push_back(std::make_pair(std::string("upnp:class"), std::string("object.item.imageItem.photo"))); break;
  }

  for (auto &protocol : item.protocols)
  {
    upnp::url url = item.url;
    url.host = host;
    url.query["content_features"] = to_base64(protocol.content_features());
    browse_item.files.push_back(std::make_pair(to_objecturl(url, protocol.suffix), protocol));
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
  if (!dir.empty())
  {
    const size_t ls = dir.find_last_of('/' , dir.length() - 1);
    const size_t lt = dir.find_last_of('\t', dir.length() - 1);

    if ((ls != dir.npos) && (lt != dir.npos) && (lt > 0))
      return dir.substr(0, std::max(ls, lt - 1));
    else if (ls != dir.npos)
      return dir.substr(0, ls);
    else if ((lt != dir.npos) && (lt > 0))
      return dir.substr(0, lt - 1);
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

upnp::url content_directory::to_objecturl(const upnp::url &url, const std::string &suffix)
{
  const std::string encoded = url;

  upnp::url new_url;
  new_url.host = url.host;

  auto i = objecturl_map.find(encoded);
  if (i == objecturl_map.end())
  {
    objecturl_list.push_back(encoded);
    objecturl_list.back().shrink_to_fit();
    objecturl_map[objecturl_list.back()] = objecturl_list.size() - 1;

    std::ostringstream str;
    str << basedir << std::hex << std::setw(8) << std::setfill('0') << (objecturl_list.size() - 1) << suffix;
    new_url.path = str.str();
  }
  else
  {
    std::ostringstream str;
    str << basedir << std::hex << std::setw(8) << std::setfill('0') << i->second << suffix;
    new_url.path = str.str();
  }

  return new_url;
}

upnp::url content_directory::from_objecturl(const upnp::url &url)
{
  const size_t ls = url.path.find_last_of('/');
  if (ls != url.path.npos)
  {
    const size_t dot = url.path.find_first_of('.', ls);
    const std::string id_str = url.path.substr(ls + 1, (dot != url.path.npos) ? (dot - ls - 1) : url.path.npos);

    try
    {
      const size_t id = std::stoull(id_str);
      if (id < objecturl_list.size())
      {
        upnp::url new_url = objecturl_list[id];
        new_url.host = url.host;
        return new_url;
      }
    }
    catch (const std::invalid_argument &) { }
    catch (const std::out_of_range &) { }
  }

  return std::string();
}


content_directory::item::item(void)
  : is_dir(false), type(item_type::none), track(0), duration(0), last_position(-1)
{
}

content_directory::item::~item()
{
}

bool content_directory::item::is_null(void) const
{
  return url.path.empty();
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


content_directory::item::stream::stream(void)
{
}

content_directory::item::stream::~stream()
{
}


content_directory::item::chapter::chapter(const std::string &title, unsigned position)
  : title(title), position(position)
{
}

content_directory::item::chapter::~chapter()
{
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

  for (auto i = parent.item_sources.begin(); i != parent.item_sources.end(); i++)
  if (starts_with(i->first, path))
  {
    std::string sub = i->first.substr(path.length() - 1);
    const size_t sl = sub.find_first_of('/');
    sub = sub.substr(0, (sl != sub.npos) ? (sl + 1) : 0);
    if ((sub.length() > 1) && (names.find(sub) == names.end()))
    {
      size_t total = 1;
      if (!i->second->list_contentdir_items(client, i->first, 0, total).empty() && (total > 0))
      {
        names.insert(sub);

        if (return_all || (count > 0))
        {
          if (start == 0)
          {
            struct item item;
            item.is_dir = true;
            item.path = sub;
            item.title = sub;
            item.title = starts_with(item.title, "/") ? item.title.substr(1) : item.title;
            item.title = ends_with(item.title, "/") ? item.title.substr(0, item.title.length() - 1) : item.title;

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

  count = names.size();

  return result;
}

content_directory::item content_directory::root_item_source::get_contentdir_item(const std::string &, const std::string &)
{
  return item();
}

} // End of namespace
