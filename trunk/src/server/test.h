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

#ifndef TEST_H
#define TEST_H

#include "platform/messageloop.h"
#include "pupnp/connection_manager.h"
#include "pupnp/connection_proxy.h"
#include "pupnp/content_directory.h"
#include "resources/resource_file.h"
#include "vlc/media.h"
#include "settings.h"
#include <cstdint>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>

namespace vlc { class instance; class transcode_stream; }

class test : private pupnp::content_directory::item_source
{
public:
  test(
          class platform::messageloop_ref &,
          class vlc::instance &,
          class pupnp::connection_manager &,
          class pupnp::content_directory &,
          const class settings &);

  virtual ~test();

  const std::set<std::string> & detected_clients() const { return clients; }
  const std::set<std::string> & played_items() const { return items; }

private: // From content_directory::item_source
  std::vector<pupnp::content_directory::item> list_contentdir_items(const std::string &client, const std::string &path, size_t start, size_t &count) override;
  pupnp::content_directory::item get_contentdir_item(const std::string &client, const std::string &path) override;
  bool correct_protocol(const pupnp::content_directory::item &, pupnp::connection_manager::protocol &) override;
  int play_item(const std::string &, const pupnp::content_directory::item &, const std::string &, std::string &, std::shared_ptr<std::istream> &) override;

private:
  class platform::messageloop_ref messageloop;
  class vlc::instance &vlc_instance;
  class pupnp::connection_manager &connection_manager;
  class pupnp::content_directory &content_directory;
  const class settings &settings;

  std::set<std::string> clients;
  std::set<std::string> items;

  const resources::resource_file a440hz_flac;
  const vlc::media a440hz_flac_media;
  const resources::resource_file pm5544_png;
  const vlc::media pm5544_png_media;
  const resources::resource_file pm5644_png;
  const vlc::media pm5644_png_media;
};

#endif
