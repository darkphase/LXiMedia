/******************************************************************************
 *   Copyright (C) 2015  A.J. Admiraal                                        *
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "platform/inifile.h"
#include "platform/messageloop.h"
#include "platform/uuid.h"
#include <chrono>
#include <cstdint>
#include <vector>

enum class encode_mode { slow, fast };
enum class video_mode { auto_, vcd, dvd, hdtv_720, hdtv_1080 };
enum class canvas_mode { none, pad };
enum class font_size { small, normal, large };
enum class surround_mode { stereo, surround51 };
enum class path_type { auto_, music, pictures, videos };
struct root_path { path_type type; std::string path; };

class settings
{
public:
    settings(class platform::messageloop_ref &, bool read_only);
    ~settings();

    void save();

    bool was_clean_exit() const;
    bool is_configure_required() const;
    void set_configure_required(bool);
    std::string latest_version();

    platform::uuid uuid();
    std::string upnp_devicename() const;
    void set_upnp_devicename(const std::string &);
    uint16_t http_port() const;
    void set_http_port(uint16_t);
    bool bind_all_networks() const;
    void set_bind_all_networks(bool);
    bool republish_rootdevice() const;
    void set_republish_rootdevice(bool);

    enum encode_mode encode_mode() const;
    void set_encode_mode(enum encode_mode);
    enum video_mode video_mode() const;
    void set_video_mode(enum video_mode);
    bool canvas_mode_enabled() const;
    enum canvas_mode canvas_mode() const;
    void set_canvas_mode(enum canvas_mode);

    bool surround_mode_enabled() const;
    enum surround_mode surround_mode() const;
    void set_surround_mode(enum surround_mode);

    enum font_size font_size() const;
    void set_font_size(enum font_size);

    bool share_removable_media() const;
    void set_share_removable_media(bool);

    bool mpeg2_enabled() const;
    void set_mpeg2_enabled(bool);
    bool mpeg4_enabled() const;
    void set_mpeg4_enabled(bool);
    bool video_mpegm2ts_enabled() const;
    void set_video_mpegm2ts_enabled(bool);
    bool video_mpegts_enabled() const;
    void set_video_mpegts_enabled(bool);
    bool video_mpeg_enabled() const;
    void set_video_mpeg_enabled(bool);

    std::vector<root_path> root_paths() const;
    void set_root_paths(const std::vector<root_path> &);

private:
    class platform::messageloop_ref messageloop;
    const bool read_only;
    class platform::inifile inifile;
    class platform::inifile::section general;
    class platform::inifile::section codecs;
    class platform::inifile::section formats;
    class platform::inifile::section paths;
    class platform::timer timer;
    const std::chrono::milliseconds save_delay;

    bool last_clean_exit;
};

#endif
