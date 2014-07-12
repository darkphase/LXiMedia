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

#ifndef PUPNP_CONTENT_DIRECTORY_H
#define PUPNP_CONTENT_DIRECTORY_H

#include "connection_manager.h"
#include "rootdevice.h"
#include "upnp.h"
#include <chrono>
#include <cstdint>
#include <map>
#include <set>
#include <vector>

namespace pupnp {

class content_directory : public rootdevice::service
{
public:
  struct item
  {
    enum class item_type : uint8_t
    {
      none  = 0,
      audio = 10, music, audio_broadcast, audio_book,
      video = 20, movie, video_broadcast, music_video,
      image = 30, photo
    };

    struct stream
    {
      stream(void);
      ~stream();

      std::string title;
      std::vector<std::pair<std::string, std::string>> query_items;
    };

    struct chapter
    {
      chapter(const std::string &title = std::string(), unsigned position = 0);
      ~chapter();

      std::string title;
      unsigned position;
    };

    item(void);
    ~item();

    bool is_null(void) const;
    bool is_audio(void) const;
    bool is_video(void) const;
    bool is_image(void) const;
    bool is_music(void) const;

    bool is_dir;
    item_type type;
    upnp::url url;
    upnp::url icon_url;
    std::string path;

    std::string title;
    std::string artist;
    std::string album;
    int track;

    unsigned duration; //!< In seconds.
    int last_position; //!< In seconds, -1 = unknown.

    std::vector<stream> streams;
    std::vector<chapter> chapters;
    std::vector<connection_manager::protocol> protocols;
  };

  struct item_source
  {
    virtual std::vector<item> list_contentdir_items(const std::string &client, const std::string &path, size_t start, size_t &count) = 0;
    virtual item get_contentdir_item(const std::string &client, const std::string &path) = 0;
  };

public:
  struct browse_item
  {
    browse_item();
    ~browse_item();

    std::string id;
    std::string parent_id;
    bool restricted;
    std::string title;
    uint32_t duration;
    std::vector<std::pair<std::string, std::string>> attributes;
    std::vector<std::pair<std::string, connection_manager::protocol>> files;
  };

  struct browse_container
  {
    browse_container();
    ~browse_container();

    std::string id;
    std::string parent_id;
    bool restricted;
    size_t child_count;
    std::string title;
    std::vector<std::pair<std::string, std::string>> attributes;
  };

  struct action_browse
  {
    enum class browse_flag { metadata, direct_children };

    virtual std::string get_object_id() const = 0;
    virtual browse_flag get_browse_flag() const = 0;
    virtual std::string get_filter() const = 0;
    virtual size_t      get_starting_index() const = 0;
    virtual size_t      get_requested_count() const = 0;
    virtual std::string get_sort_criteria() const = 0;

    virtual void add_item(const browse_item &) = 0;
    virtual void add_container(const browse_container &) = 0;
    virtual void set_response(size_t totalMatches, uint32_t updateID) = 0;
  };

  struct action_search
  {
    virtual std::string get_container_id() const = 0;
    virtual std::string get_search_criteria() const = 0;
    virtual std::string get_filter() const = 0;
    virtual size_t      get_starting_index() const = 0;
    virtual size_t      get_requested_count() const = 0;
    virtual std::string get_sort_criteria() const = 0;

    virtual void add_item(const browse_item &) = 0;
    virtual void set_response(size_t totalMatches, uint32_t updateID) = 0;
  };

  struct action_get_search_capabilities
  {
    virtual void set_response(const std::string &) = 0;
  };

  struct action_get_sort_capabilities
  {
    virtual void set_response(const std::string &) = 0;
  };

  struct action_get_system_update_id
  {
    virtual void set_response(uint32_t) = 0;
  };

  // Samsung GetFeatureList
  struct action_get_featurelist
  {
    virtual void set_response(const std::vector<std::string> &) = 0;
  };

public:
  static const char service_id[];
  static const char service_type[];

  content_directory(class messageloop &, class upnp &, class rootdevice &, class connection_manager &);
  virtual ~content_directory();

  void item_source_register(const std::string &path, struct item_source &);
  void item_source_unregister(const std::string &path);

  void update_system();
  void update_path(const std::string &path);

  void handle_action(const upnp::request &, action_browse &);
  void handle_action(const upnp::request &, action_search &);
  void handle_action(const upnp::request &, action_get_search_capabilities &);
  void handle_action(const upnp::request &, action_get_sort_capabilities &);
  void handle_action(const upnp::request &, action_get_system_update_id &);
  void handle_action(const upnp::request &, action_get_featurelist &);

protected: // From rootdevice::service
  virtual const char * get_service_type(void) override;

  virtual void initialize(void) override;
  virtual void close(void) override;

  virtual void write_service_description(rootdevice::service_description &) const override;
  virtual void write_eventable_statevariables(rootdevice::eventable_propertyset &) const override;

private:
  int http_request(const upnp::request &, std::string &, std::shared_ptr<std::istream> &);

private:
  void add_directory(action_browse &, enum item::item_type, const std::string &client, const std::string &path, const std::string &title = std::string());
  void add_container(action_browse &, enum item::item_type, const std::string &path, const std::string &title = std::string(), size_t child_count = size_t(-1));
  void add_file(action_browse &, const std::string &host, const item &, const std::string &path, const std::string &title = std::string());

  static std::string basepath(const std::string &);
  static std::string parentpath(const std::string &);
  std::string to_objectid(const std::string &path, bool create = true);
  std::string from_objectid(const std::string &id);
  pupnp::upnp::url to_objecturl(const upnp::url &url, const std::string &suffix);
  pupnp::upnp::url from_objecturl(const upnp::url &url);

  void num_connections_changed(int num_connections);
  void process_pending_updates(void);

private:
  class messageloop &messageloop;
  class upnp &upnp;
  class rootdevice &rootdevice;
  class connection_manager &connection_manager;
  const std::string basedir;

  timer update_timer;
  const std::chrono::seconds update_timer_interval;
  uint32_t system_update_id;
  std::set<std::string> pending_container_updates;
  bool allow_process_pending_updates;
  std::map<std::string, uint32_t> container_update_ids;
  std::map<std::string, item_source *> item_sources;

  std::vector<std::string> objectid_list;
  std::map<std::string, size_t> objectid_map;
  std::vector<std::string> objecturl_list;
  std::map<std::string, size_t> objecturl_map;

  struct root_item_source final : item_source
  {
    root_item_source(content_directory &parent) : parent(parent) { }
    content_directory &parent;

    virtual std::vector<item> list_contentdir_items(const std::string &client, const std::string &path, size_t start, size_t &count) override;
    virtual item get_contentdir_item(const std::string &client, const std::string &path) override;
  } root_item_source;
};

} // End of namespace

#endif
