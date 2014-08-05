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

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include "messageloop.h"
#include "pupnp/connection_manager.h"
#include "pupnp/content_directory.h"
#include "settings.h"
#include "vlc/instance.h"
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

class mediaplayer : private pupnp::content_directory::item_source
{
public:
  mediaplayer(
          class messageloop &,
          class vlc::instance &,
          pupnp::connection_manager &,
          pupnp::content_directory &,
          const class settings &);

  virtual ~mediaplayer();

private: // From content_directory::item_source
  std::vector<pupnp::content_directory::item> list_contentdir_items(const std::string &client, const std::string &path, size_t start, size_t &count) override;
  pupnp::content_directory::item get_contentdir_item(const std::string &client, const std::string &path) override;
  void correct_protocol(const pupnp::content_directory::item &, pupnp::connection_manager::protocol &) override;
  int play_item(const pupnp::content_directory::item &, const std::string &, std::string &, std::shared_ptr<std::istream> &) override;

private:
  pupnp::content_directory::item make_item(const std::string &, const std::string &) const;
  root_path to_system_path(const std::string &) const;
  std::string to_virtual_path(const std::string &) const;

private:
  class messageloop &messageloop;
  class vlc::instance &vlc_instance;
  class pupnp::connection_manager &connection_manager;
  class pupnp::content_directory &content_directory;
  const class settings &settings;
  const std::string basedir;

  struct transcode_stream;
  std::map<std::string, std::shared_ptr<transcode_stream>> pending_transcode_streams;
};

#endif
