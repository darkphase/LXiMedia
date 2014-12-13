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

#include "vlc/media_cache.h"
#include "vlc/instance.h"
#include "vlc/media.h"
#include "platform/fork.h"
#include "platform/fstream.h"
#include "platform/sha1.h"
#include "platform/string.h"
#include <stdexcept>
#include <vlc/vlc.h>
#include <cstring>
#include <sstream>
#include <iostream>

namespace vlc {

media_cache::media_cache()
{
}

media_cache::~media_cache()
{
}

static platform::uuid uuid_from_file(const std::string &path)
{
    platform::ifstream file(path, std::ios_base::binary);
    if (file.is_open())
    {
        static const int block_count = 8;
        static const uint64_t block_size = 65536;

        file.seekg(0, std::ios_base::end);
        const uint64_t length = file.tellg();
        file.seekg(0, std::ios_base::beg);

        std::vector<char> buffer;
        if (length >= (block_size * block_count))
        {
            const uint64_t chunk = (length / block_count) & ~(block_size - 1);

            buffer.resize(block_size * block_count);
            int num_blocks = 0;
            for (; num_blocks < block_count; num_blocks++)
            {
                file.read(&buffer[num_blocks * block_size], block_size);
                if (uint64_t(file.gcount()) == block_size)
                    file.seekg(chunk - block_size, std::ios_base::cur);
                else
                    break;
            }

            buffer.resize(block_size * num_blocks);
        }
        else
        {
            buffer.resize(length);
            file.read(&buffer[0], buffer.size());
        }

        unsigned char hash[20];
        memset(hash, 0, sizeof(hash));
        sha1::calc(buffer.data(), buffer.size(), hash);

        struct platform::uuid uuid;
        memcpy(uuid.value, hash, std::min(sizeof(uuid.value), sizeof(hash)));
        uuid.value[6] = (uuid.value[6] & 0x0F) | 0x50;
        uuid.value[8] = (uuid.value[8] & 0x3F) | 0x80;

        return uuid;
    }

    return platform::uuid();
}

platform::uuid media_cache::uuid(const std::string &mrl) const
{
    std::unique_lock<std::mutex> l(mutex);

    auto &data = cache[mrl];
    if (!data.uuid_generated)
    {
        l.unlock();

        platform::uuid uuid;
#if defined(__unix__) || defined(__APPLE__)
        if (starts_with(mrl, "file://"))
            uuid = uuid_from_file(from_percent(mrl.substr(7)));
#elif defined(WIN32)
        if (starts_with(mrl, "file:"))
            uuid = uuid_from_file(from_percent(mrl.substr(5)));
#endif

        l.lock();

        auto &data = cache[mrl];
        data.uuid = uuid;
        data.uuid_generated = true;
        return data.uuid;
    }

    return data.uuid;
}

static media_cache::track_type track_type_from_media(
    class media &media)
{
    media_cache::track_type track_type = media_cache::track_type::unknown;

    libvlc_media_parse(media);

    libvlc_media_track_t **track_list = nullptr;
    const unsigned count = libvlc_media_tracks_get(media, &track_list);
    if (track_list)
    {
        for (unsigned i = 0; (i < count) && track_list[i]; i++)
            if (track_list[i]->i_codec != 0x66646E75 /*'undf'*/)
            {
                switch (track_list[i]->i_type)
                {
                case libvlc_track_unknown:
                    break;

                case libvlc_track_audio:
                    if (track_type != media_cache::track_type::video)
                        track_type = media_cache::track_type::audio;

                    break;

                case libvlc_track_video:
                    track_type = media_cache::track_type::video;
                    break;

                case libvlc_track_text:
                    if (track_type == media_cache::track_type::unknown)
                        track_type = media_cache::track_type::text;

                    break;
                }
            }

        libvlc_media_tracks_release(track_list, count);
    }

    return track_type;
}

media_cache::track_type media_cache::media_type(class media &media) const
{
    std::unique_lock<std::mutex> l(mutex);

    auto &data = cache[media.mrl()];
    if (!data.media_type_read)
    {
        l.unlock();

        const auto media_type = track_type_from_media(media);

        l.lock();

        auto &data = cache[media.mrl()];
        data.media_type = media_type;
        data.media_type_read = true;
        return data.media_type;
    }

    return data.media_type;
}

static struct media_cache::media_info media_info_from_media(
    class media &media)
{
    struct media_cache::media_info media_info;

    auto player = libvlc_media_player_new_from_media(media);
    if (player)
    {
        static const int width = 256, height = 256, align = 32;

        struct T
        {
            static void callback(const libvlc_event_t *e, void *opaque)
            {
                T * const t = reinterpret_cast<T *>(opaque);
                std::lock_guard<std::mutex> _(t->mutex);

                if (e->type == libvlc_MediaPlayerTimeChanged)
                    t->new_time = e->u.media_player_time_changed.new_time;
                else if (e->type == libvlc_MediaPlayerPlaying)
                    t->playing = true;
                else if (e->type == libvlc_MediaPlayerEndReached)
                    t->stopped = true;
                else if (e->type == libvlc_MediaPlayerEncounteredError)
                    t->stopped = true;

                t->condition.notify_one();
            }

            static void play(void */*opaque*/, const void */*samples*/, unsigned /*count*/, int64_t /*pts*/)
            {
            }

            static void * lock(void *opaque, void **planes)
            {
                T * const t = reinterpret_cast<T *>(opaque);
                return *planes = (void *)((uintptr_t(&t->pixel_buffer[0]) + (align - 1)) & ~uintptr_t(align - 1));
            }

            std::condition_variable condition;
            std::mutex mutex;
            bool playing, stopped;
            libvlc_time_t new_time;
            std::vector<uint8_t> pixel_buffer;
        } t;

        t.stopped = t.playing = false;
        t.new_time = -1;
        t.pixel_buffer.resize((width * height * sizeof(uint32_t)) + align);

        libvlc_media_player_set_rate(player, 1000.0f);

        libvlc_audio_set_callbacks(player, &T::play, nullptr, nullptr, nullptr, nullptr, &t);
        libvlc_audio_set_format(player, "S16N", 44100, 2);
        libvlc_video_set_callbacks(player, &T::lock, nullptr, nullptr, &t);
        libvlc_video_set_format(player, "RV32", width, height, width * sizeof(uint32_t));

        auto event_manager = libvlc_media_player_event_manager(player);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerPlaying, T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEndReached, T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEncounteredError, T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerTimeChanged, T::callback, &t);

        if (libvlc_media_player_play(player) == 0)
        {
            {
                std::unique_lock<std::mutex> l(t.mutex);
                while (!t.playing && !t.stopped) t.condition.wait(l);
            }

            {
                std::unique_lock<std::mutex> l(t.mutex);
                while ((t.new_time < 1000) && !t.stopped) t.condition.wait(l);
            }

            media_info.chapter_count = libvlc_media_player_get_chapter_count(player);

            libvlc_media_player_stop(player);
        }

        libvlc_event_detach(event_manager, libvlc_MediaPlayerTimeChanged, T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEncounteredError, T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEndReached, T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerPlaying, T::callback, &t);

        libvlc_media_player_release(player);
    }

    libvlc_media_track_t **track_list = nullptr;
    const unsigned count = libvlc_media_tracks_get(media, &track_list);
    if (track_list)
    {
        for (unsigned i = 0; (i < count) && track_list[i]; i++)
            if (track_list[i]->i_codec != 0x66646E75 /*'undf'*/)
            {
                struct media_cache::track track;
                track.id = track_list[i]->i_id;
                if (track_list[i]->psz_language)    track.language    = track_list[i]->psz_language;
                if (track_list[i]->psz_description) track.description = track_list[i]->psz_description;

                track.type = media_cache::track_type::unknown;
                switch (track_list[i]->i_type)
                {
                case libvlc_track_unknown:
                    track.type = media_cache::track_type::unknown;
                    break;

                case libvlc_track_audio:
                    track.type = media_cache::track_type::audio;
                    if (track_list[i]->audio)
                    {
                        track.audio.sample_rate = track_list[i]->audio->i_rate;
                        track.audio.channels = track_list[i]->audio->i_channels;
                    }
                    break;

                case libvlc_track_video:
                    track.type = media_cache::track_type::video;
                    if (track_list[i]->video)
                    {
                        track.video.width = track_list[i]->video->i_width;
                        track.video.height = track_list[i]->video->i_height;
                        track.video.frame_rate =
                                float(track_list[i]->video->i_frame_rate_num) /
                                float(track_list[i]->video->i_frame_rate_den);
                    }
                    break;

                case libvlc_track_text:
                    track.type = media_cache::track_type::text;
                    break;
                }

                media_info.tracks.emplace_back(std::move(track));
            }

        libvlc_media_tracks_release(track_list, count);
    }

    media_info.duration = std::chrono::milliseconds(libvlc_media_get_duration(media));

    return media_info;
}

struct media_cache::media_info media_cache::media_info(class media &media) const
{
    std::unique_lock<std::mutex> l(mutex);

    auto &data = cache[media.mrl()];
    if (!data.media_info_read)
    {
        l.unlock();

        const auto media_info = media_info_from_media(media);

        l.lock();

        auto &data = cache[media.mrl()];
        data.media_info = media_info;
        data.media_info_read = true;
        return data.media_info;
    }

    return data.media_info;
}


media_cache::track::track()
    : id(0),
      type(track_type::unknown)
{
}

media_cache::track::~track()
{
}


media_cache::media_info::media_info()
    : duration(0),
      chapter_count(0)
{
}

media_cache::media_info::~media_info()
{
}


media_cache::data::data()
    : uuid_generated(false),
      media_type_read(false),
      media_info_read(false),
      media_type(track_type::unknown)
{
}

media_cache::data::~data()
{
}

/*

static std::ostream & operator<<(std::ostream &str, const media_cache::track &track)
{
    str << track.id << ' '
        << '"' << to_percent(track.language) << '"' << ' '
        << '"' << to_percent(track.description) << '"' << ' '
        << int(track.type);

    switch (track.type)
    {
    case media_cache::track_type::audio:
        str << ' ' << track.audio.sample_rate << ' ' << track.audio.channels;
        break;

    case media_cache::track_type::video:
        str << ' ' << track.video.width << ' ' << track.video.height << ' ' << track.video.frame_rate;
        break;

    case media_cache::track_type::unknown:
    case media_cache::track_type::text:
        break;
    }

    return str;
}

static std::istream & operator>>(std::istream &str, media_cache::track &track)
{
    str >> track.id;

    std::string language;
    str >> language;
    if (language.length() >= 2)
        track.language = from_percent(language.substr(1, language.length() - 2));

    std::string description;
    str >> description;
    if (description.length() >= 2)
        track.description = from_percent(description.substr(1, description.length() - 2));

    int type;
    str >> type;
    track.type = media_cache::track_type(type);

    switch (track.type)
    {
    case media_cache::track_type::audio:
        str >> track.audio.sample_rate >> track.audio.channels;
        break;

    case media_cache::track_type::video:
        str >> track.video.width >> track.video.height >> track.video.frame_rate;
        break;

    case media_cache::track_type::unknown:
    case media_cache::track_type::text:
        break;
    }

    return str;
}

std::ostream & operator<<(std::ostream &str, const media_cache::parsed_data &parsed_data)
{
    str << parsed_data.uuid << ' ';

    for (auto &i : parsed_data.tracks)
        str << '{' << i << '}' << ' ';

    str << parsed_data.duration.count() << ' ';
    str << parsed_data.chapter_count;

    return str;
}

std::istream & operator>>(std::istream &str, media_cache::parsed_data &parsed_data)
{
    str >> parsed_data.uuid;
    str.get(); // ' '
    while (str.peek() == '{')
    {
        str.get(); // '{'
        struct media_cache::track track;
        str >> track;
        parsed_data.tracks.push_back(track);
        str.get(); // '}'
        str.get(); // ' '
    }

    long d;
    str >> d;
    parsed_data.duration = std::chrono::milliseconds(d);

    str >> parsed_data.chapter_count;

    return str;
}

*/

} // End of namespace
