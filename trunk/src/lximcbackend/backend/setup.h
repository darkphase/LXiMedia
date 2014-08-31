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

#ifndef SETUP_H
#define SETUP_H

#include "platform/messageloop.h"
#include "pupnp/connection_manager.h"
#include "pupnp/content_directory.h"
#include "settings.h"
#include "vlc/instance.h"
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

class setup : private pupnp::content_directory::item_source
{
public:
  setup(
          class messageloop &,
          pupnp::content_directory &,
          const class settings &);

  virtual ~setup();

private: // From content_directory::item_source
  std::vector<pupnp::content_directory::item> list_contentdir_items(const std::string &client, const std::string &path, size_t start, size_t &count) override;
  pupnp::content_directory::item get_contentdir_item(const std::string &client, const std::string &path) override;
  void correct_protocol(const pupnp::content_directory::item &, pupnp::connection_manager::protocol &) override;
  int play_item(const std::string &, const pupnp::content_directory::item &, const std::string &, std::string &, std::shared_ptr<std::istream> &) override;

private:
  class messageloop &messageloop;
  class pupnp::content_directory &content_directory;
  const class settings &settings;
  const std::string basedir;

  bool shutdown_pending;
};

#endif
