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
#include "platform/messageloop.h"
#include "platform/uuid.h"
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
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
        std::string language;
        std::string description;

        track_type type;
        union
        {
            struct { unsigned sample_rate, channels; } audio;
            struct { unsigned width, height; float frame_rate; } video;
        };
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
    media_cache();
    ~media_cache();

    void scan_all(std::vector<class media> &);

    platform::uuid uuid(const std::string &) const;
    struct media_info media_info(class media &) const;
    enum media_type media_type(class media &) const;

private:
    mutable std::mutex mutex;
    mutable std::map<std::string, data> cache;
};

std::ostream & operator<<(std::ostream &, const struct media_cache::track &);
std::ostream & operator<<(std::ostream &, const struct media_cache::media_info &);
std::istream & operator>>(std::istream &, struct media_cache::track &);
std::istream & operator>>(std::istream &, struct media_cache::media_info &);

} // End of namespace

#endif
