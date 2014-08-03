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

#ifndef VLC_MEDIA_H
#define VLC_MEDIA_H

#include <chrono>
#include <memory>
#include <string>
#include <vector>

struct libvlc_media_t;

namespace vlc {

class instance;

class media
{
public:
    enum class track_type { unknown, audio, video, text };

    struct track
    {
        std::string language;
        std::string description;

        track_type type;
        union
        {
            struct { unsigned sample_rate, channels; } audio;
            struct { unsigned width, height; float frame_rate; } video;
        };
    };

public:
    static media from_file(class instance &, const std::string &path) noexcept;
    static media from_mrl(class instance &, const std::string &mrl) noexcept;

    media(const media &) noexcept;
    media(media &&) noexcept;
    ~media() noexcept;

    media & operator=(const media &) noexcept;
    media & operator=(media &&) noexcept;
    inline operator libvlc_media_t *() noexcept { return libvlc_media; }

    std::string mrl() const noexcept;

    const std::vector<track> & tracks() const;
    std::chrono::milliseconds duration() const;
    int chapter_count() const;

private:
    media(libvlc_media_t *) noexcept;

    libvlc_media_t *libvlc_media;

    struct parsed_data;
    const parsed_data &parse() const;
    mutable std::shared_ptr<parsed_data> parsed;
};

} // End of namespace

#endif
