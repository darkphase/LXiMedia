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

#include "messageloop.h"
#include <chrono>
#include <cstdint>
#include <string>
#include <map>

enum class encode_mode { slow, fast };
enum class canvas_mode { none, pad, crop };
enum class surround_mode { stereo, surround51 };
enum class video_mode { auto_, vcd, dvd_ntsc, dvd_pal, hdtv_720, hdtv_1080 };
enum class path_type { auto_, music };
struct root_path { path_type type; std::string path; };

class settings
{
public:
    explicit settings(class messageloop &);
    ~settings();

    std::string uuid();
    std::string devicename() const;
    uint16_t http_port() const;

    enum encode_mode encode_mode() const;
    enum canvas_mode canvas_mode() const;
    enum surround_mode surround_mode() const;
    enum video_mode video_mode() const;

    std::vector<root_path> root_paths() const;

private:
    void save();
    std::string read(const std::string &, const std::string &, const std::string &) const;
    void write(const std::string &, const std::string &, const std::string &);

private:
    class messageloop &messageloop;
    class timer timer;
    const std::chrono::milliseconds save_delay;

    std::map<std::string, std::map<std::string, std::string>> values;
    bool touched;
};

#endif
