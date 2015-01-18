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

#ifndef FILES_H
#define FILES_H

#include "recommended.h"
#include "platform/messageloop.h"
#include "pupnp/connection_manager.h"
#include "pupnp/connection_proxy.h"
#include "pupnp/content_directory.h"
#include "vlc/media_cache.h"
#include "settings.h"
#include "watchlist.h"
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace vlc { class instance; class transcode_stream; }

class watchlist;

class files
        : private pupnp::content_directory::item_source,
          private recommended::item_source
{
public:
    files(
            class platform::messageloop_ref &,
            class vlc::instance &,
            class pupnp::connection_manager &,
            class pupnp::content_directory &,
            class recommended &recommended,
            const class settings &,
            class platform::inifile &media_cache_file,
            class platform::inifile &watchlist_file);

    virtual ~files();

private: // From content_directory::item_source
    std::vector<pupnp::content_directory::item> list_contentdir_items(const std::string &client, const std::string &path, size_t start, size_t &count) override;
    pupnp::content_directory::item get_contentdir_item(const std::string &client, const std::string &path) override;
    bool correct_protocol(const pupnp::content_directory::item &, pupnp::connection_manager::protocol &) override;
    int play_item(const std::string &, const pupnp::content_directory::item &, const std::string &, std::string &, std::shared_ptr<std::istream> &) override;

private: // From recommended::item_source
    std::vector<pupnp::content_directory::item> list_recommended_items(const std::string &client, size_t start, size_t &count) override;

private:
    const std::vector<std::string> & list_files(const std::string &, bool flush_cache);
    std::vector<std::string> scan_files_paths(const std::vector<std::string> &) const;
    pupnp::content_directory::item make_item(const std::string &, const std::string &) const;
    root_path to_system_path(const std::string &) const;
    std::string to_virtual_path(const std::string &) const;

    int play_audio_video_item(
            const std::string &source_address,
            const pupnp::content_directory::item &,
            const pupnp::connection_manager::protocol &,
            std::string &content_type,
            std::shared_ptr<std::istream> &response);

    int get_image_item(
            const std::string &source_address,
            const pupnp::content_directory::item &,
            const pupnp::connection_manager::protocol &,
            std::string &content_type,
            std::shared_ptr<std::istream> &response);

    void playback_position_changed(
            const pupnp::content_directory::item &item,
            std::chrono::system_clock::time_point,
            std::chrono::milliseconds);

private:
    class platform::messageloop_ref messageloop;
    class vlc::instance &vlc_instance;
    mutable class vlc::media_cache media_cache;
    class pupnp::connection_manager &connection_manager;
    class pupnp::content_directory &content_directory;
    class recommended &recommended;
    const class settings &settings;
    class watchlist watchlist;
    const std::string basedir;
    const std::chrono::milliseconds min_parse_time;
    const std::chrono::milliseconds max_parse_time;
    const std::chrono::milliseconds item_parse_time;

    std::map<std::string, std::vector<std::string>> files_cache;
};

#endif
