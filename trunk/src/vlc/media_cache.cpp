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
#include "platform/fstream.h"
#include "platform/sha1.h"
#include "platform/string.h"
#include <stdexcept>
#include <vlc/vlc.h>
#include <cstring>

namespace vlc {

struct media_cache::parsed_data
{
    parsed_data() : chapter_count(-1) { }

    platform::uuid uuid;
    std::vector<track> tracks;
    std::chrono::milliseconds duration;
    int chapter_count;
};

media_cache::media_cache(
        class platform::messageloop_ref &messageloop,
        class instance &instance)
  : messageloop(messageloop),
    instance(instance),
    pending_items(0)
{
}

media_cache::~media_cache()
{
    abort();

    std::unique_lock<std::mutex> l(mutex);

    while (!thread_pool.empty() || (pending_items > 0))
        condition.wait(l);

    for (auto &i : thread_dump) i->join();
    thread_dump.clear();
}

void media_cache::async_parse_items(const std::vector<std::string> &mrls)
{
    std::lock_guard<std::mutex> _(mutex);

    while (!work_list.empty())
        work_list.pop();

    for (auto &i : mrls)
        if (cache.find(i) == cache.end())
            work_list.push(i);

    if (!work_list.empty())
    {
        if (on_finished)
            this->on_finished = on_finished;

        for (size_t i = thread_pool.size(), n = std::max(std::thread::hardware_concurrency(), 1u);
             (i < n) && (i < work_list.size());
             i++)
        {
            auto thread = new std::thread(std::bind(&media_cache::worker_thread, this));
            thread_pool.emplace(thread->get_id(), std::unique_ptr<std::thread>(thread));
        }
    }
}

void media_cache::wait()
{
    std::unique_lock<std::mutex> l(mutex);

    while (!thread_pool.empty())
        condition.wait(l);
}

void media_cache::abort()
{
    std::unique_lock<std::mutex> l(mutex);

    while (!work_list.empty())
        work_list.pop();
}

bool media_cache::has_data(const std::string &mrl)
{
    {
        std::lock_guard<std::mutex> _(mutex);

        auto i = cache.find(mrl);
        if (i != cache.end())
            return i->second != nullptr;
    }

    return false;
}

platform::uuid media_cache::uuid(const std::string &mrl)
{
    return read_parsed_data(instance, mrl).uuid;
}

const std::vector<media_cache::track> & media_cache::tracks(const std::string &mrl)
{
    return read_parsed_data(instance, mrl).tracks;
}

std::chrono::milliseconds media_cache::duration(const std::string &mrl)
{
    return read_parsed_data(instance, mrl).duration;
}

int media_cache::chapter_count(const std::string &mrl)
{
    return read_parsed_data(instance, mrl).chapter_count;
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
        const uint64_t chunk = std::max(uint64_t((length / block_count) & ~(block_size - 1)), block_size);
        file.seekg(0, std::ios_base::beg);

        std::vector<char> buffer;
        buffer.resize(block_size * block_count);
        int num_blocks = 0;
        for (; num_blocks < block_count; num_blocks++)
        {
            file.read(&buffer[num_blocks * block_size], block_size);
            if (file.gcount() == block_size)
                file.seekg(chunk - block_size, std::ios_base::cur);
            else
                break;
        }

        unsigned char hash[20];
        memset(hash, 0, sizeof(hash));
        sha1::calc(buffer.data(), block_size * num_blocks, hash);

        struct platform::uuid uuid;
        memcpy(uuid.value, hash, std::min(sizeof(uuid.value), sizeof(hash)));
        uuid.value[6] = (uuid.value[6] & 0x0F) | 0x50;
        uuid.value[8] = (uuid.value[8] & 0x3F) | 0x80;

        return uuid;
    }

    return platform::uuid();
}

const struct media_cache::parsed_data & media_cache::read_parsed_data(
        class instance &instance,
        const std::string &mrl)
{
    {
        std::unique_lock<std::mutex> l(mutex);

        for (;;)
        {
            auto i = cache.find(mrl);
            if (i == cache.end())
            {
                cache[mrl] = nullptr;
                break;
            }
            else if (i->second == nullptr) // Other thread is currently parsing this item.
                condition.wait(l);
            else // Already parsed.
                return *(i->second);
        }

        pending_items++;
    }

    std::unique_ptr<parsed_data> parsed(new parsed_data());

#if defined(__unix__) || defined(__APPLE__)
    if (starts_with(mrl, "file://"))
        parsed->uuid = uuid_from_file(from_percent(mrl.substr(7)));
#elif defined(WIN32)
    if (starts_with(mrl, "file:"))
        parsed->uuid = uuid_from_file(from_percent(mrl.substr(5)));
#endif

    auto media = media::from_mrl(instance, mrl);

    //libvlc_media_parse(media);

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

        libvlc_media_player_set_rate(player, 10.0f);

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

            parsed->chapter_count = libvlc_media_player_get_chapter_count(player);

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
                struct track track;
                track.id = track_list[i]->i_id;
                if (track_list[i]->psz_language)    track.language    = track_list[i]->psz_language;
                if (track_list[i]->psz_description) track.description = track_list[i]->psz_description;

                track.type = track_type::unknown;
                switch (track_list[i]->i_type)
                {
                case libvlc_track_unknown:
                    track.type = track_type::unknown;
                    break;

                case libvlc_track_audio:
                    track.type = track_type::audio;
                    if (track_list[i]->audio)
                    {
                        track.audio.sample_rate = track_list[i]->audio->i_rate;
                        track.audio.channels = track_list[i]->audio->i_channels;
                    }
                    break;

                case libvlc_track_video:
                    track.type = track_type::video;
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
                    track.type = track_type::text;
                    break;
                }

                parsed->tracks.emplace_back(std::move(track));
            }

        libvlc_media_tracks_release(track_list, count);
    }

    parsed->duration = std::chrono::milliseconds(libvlc_media_get_duration(media));

    {
        std::lock_guard<std::mutex> _(mutex);

        auto i = cache.find(mrl);
        if (i != cache.end())
        {
            i->second = std::move(parsed);
            pending_items--;
            condition.notify_all();
            return *(i->second);
        }
    }

    throw std::runtime_error("Cache corruption detected.");
}

void media_cache::worker_thread()
{
    class instance instance;

    for (;;)
    {
        std::string mrl;
        {
            std::lock_guard<std::mutex> _(mutex);

            if (!work_list.empty())
            {
                mrl = std::move(work_list.front());
                work_list.pop();
            }
            else
                break;
        }

        read_parsed_data(instance, mrl);
    }

    {
        std::lock_guard<std::mutex> _(mutex);

        auto i = thread_pool.find(std::this_thread::get_id());
        if (i != thread_pool.end())
        {
            thread_dump.emplace_back(std::move(i->second));
            thread_pool.erase(i);
            condition.notify_all();
        }

        messageloop.post(std::bind(&media_cache::finish, this));
    }
}

void media_cache::finish()
{
    std::lock_guard<std::mutex> _(mutex);

    if (thread_pool.empty())
    {
        for (auto &i : thread_dump) i->join();
        thread_dump.clear();

        if (on_finished)
            messageloop.post(on_finished);
    }
}

} // End of namespace
