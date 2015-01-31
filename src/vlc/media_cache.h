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

#ifndef VLC_MEDIA_CACHE_H
#define VLC_MEDIA_CACHE_H

#include "media.h"
#include "platform/inifile.h"
#include "platform/messageloop.h"
#include "platform/process.h"
#include "platform/uuid.h"
#include <chrono>
#include <map>
#include <string>
#include <vector>

struct libvlc_media_t;

namespace vlc {

class media;

enum class track_type { unknown, audio, video, text };
enum class media_type { unknown, audio, video, picture };

class media_cache
{
public:
    struct track
    {
        track();
        ~track();

        int id;
        std::string file;

        std::string language;
        std::string description;

        track_type type;
        union
        {
            struct { unsigned sample_rate, channels; } audio;
            struct { unsigned width, height; float frame_rate; } video;
        };
        struct { std::string encoding; } text;
    };

    struct media_info
    {
        media_info();
        ~media_info();

        std::vector<track> tracks;
        std::chrono::milliseconds duration;
        int chapter_count;
    };

private:
    struct data
    {
        data();
        ~data();

        bool uuid_generated;
        bool media_info_read;

        platform::uuid uuid;
        struct media_info media_info;
    };

public:
    explicit media_cache(
            class platform::messageloop_ref &,
            class instance &,
            class platform::inifile &);

    ~media_cache();

    platform::uuid uuid(const std::string &);
    struct media_info media_info(class media &);
    enum media_type media_type(class media &);

    void scan_files(const std::vector<std::string> &);

private:
    static int scan_files_process(platform::process &);
    void stop_process_pool();
    platform::process &get_process_from_pool(unsigned);
    struct media_info subtitle_info(const std::string &);

private:
    static platform::process::function_handle scan_files_function;

    class platform::messageloop_ref messageloop;
    class instance &instance;
    class platform::inifile &inifile;

    std::map<std::string, platform::uuid> uuids;
    class platform::timer save_inifile_timer;
    const std::chrono::seconds save_delay;
    class platform::inifile::section section;

    std::vector<std::unique_ptr<platform::process>> process_pool;
    platform::timer stop_process_pool_timer;
    const std::chrono::seconds process_pool_timeout;
};

std::ostream & operator<<(std::ostream &, const struct media_cache::track &);
std::ostream & operator<<(std::ostream &, const struct media_cache::media_info &);
std::istream & operator>>(std::istream &, struct media_cache::track &);
std::istream & operator>>(std::istream &, struct media_cache::media_info &);

} // End of namespace

#endif
