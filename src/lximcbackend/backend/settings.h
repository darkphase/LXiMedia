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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "platform/messageloop.h"
#include <chrono>
#include <cstdint>
#include <string>
#include <map>

enum class encode_mode { slow, fast };
enum class video_mode { auto_, vcd, dvd, hdtv_720, hdtv_1080 };
enum class canvas_mode { none, pad, crop };
enum class surround_mode { stereo, surround51 };
enum class path_type { auto_, music, pictures, videos };
struct root_path { path_type type; std::string path; };

class settings
{
public:
    explicit settings(class messageloop &);
    ~settings();

    std::string uuid();
    std::string devicename() const;
    void set_devicename(const std::string &);
    uint16_t http_port() const;
    void set_http_port(uint16_t);
    bool bind_all_networks() const;
    void set_bind_all_networks(bool);
    bool republish_rootdevice() const;
    void set_republish_rootdevice(bool);
    bool allow_shutdown() const;
    void set_allow_shutdown(bool);

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

    bool verbose_logging_enabled() const;
    void set_verbose_logging_enabled(bool);

    std::vector<root_path> root_paths() const;
    void set_root_paths(const std::vector<root_path> &);

private:
    void save();
    std::string read(const std::string &, const std::string &, const std::string &) const;
    void write(const std::string &, const std::string &, const std::string &);
    void erase(const std::string &, const std::string &);

private:
    class messageloop &messageloop;
    class timer timer;
    const std::chrono::milliseconds save_delay;

    std::map<std::string, std::map<std::string, std::string>> values;
    bool touched;
};

#endif
