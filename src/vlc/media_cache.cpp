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
#include "platform/process.h"
#include "platform/path.h"
#include "platform/string.h"
#include <sha1/sha1.h>
#include <vlc/vlc.h>
#include <cstring>
#include <sstream>
#include <unordered_set>

namespace vlc {

media_cache::media_cache(class instance &instance)
    : instance(instance),
      background_scan_finished(true)
{
}

media_cache::~media_cache()
{
    if (background_scan_thread.joinable())
        background_scan_thread.join();
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

        const auto uuid = uuid_from_file(platform::path_from_mrl(mrl));

        l.lock();

        auto &data = cache[mrl];
        data.uuid = uuid;
        data.uuid_generated = true;
        return data.uuid;
    }

    return data.uuid;
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
                else if (e->type == libvlc_MediaPlayerLengthChanged)
                    t->new_length = e->u.media_player_length_changed.new_length;
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
            libvlc_time_t new_length;
            std::vector<uint8_t> pixel_buffer;
        } t;

        t.stopped = t.playing = false;
        t.new_time = -1;
        t.new_length = -1;
        t.pixel_buffer.resize((width * height * sizeof(uint32_t)) + align);

        libvlc_media_player_set_rate(player, 30.0f);

        libvlc_audio_set_callbacks(player, &T::play, nullptr, nullptr, nullptr, nullptr, &t);
        libvlc_audio_set_format(player, "S16N", 44100, 2);
        libvlc_video_set_callbacks(player, &T::lock, nullptr, nullptr, &t);
        libvlc_video_set_format(player, "RV32", width, height, width * sizeof(uint32_t));

        auto event_manager = libvlc_media_player_event_manager(player);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerPlaying, T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEndReached, T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEncounteredError, T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerLengthChanged, T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerTimeChanged, T::callback, &t);

        if (libvlc_media_player_play(player) == 0)
        {
            std::unique_lock<std::mutex> l(t.mutex);

            while (!t.playing && !t.stopped) t.condition.wait(l);
            while ((t.new_time < 1000) && !t.stopped) t.condition.wait(l);

            l.unlock();

            media_info.chapter_count = libvlc_media_player_get_chapter_count(player);

            libvlc_media_player_stop(player);
        }

        libvlc_event_detach(event_manager, libvlc_MediaPlayerTimeChanged, T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerLengthChanged, T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEncounteredError, T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEndReached, T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerPlaying, T::callback, &t);

        libvlc_media_player_release(player);

        media_info.duration = std::chrono::milliseconds(
                    std::max(libvlc_media_get_duration(media), t.new_length));
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

                track.type = track_type::unknown;
                switch (track_list[i]->i_type)
                {
                case libvlc_track_unknown:
                    track.type = track_type::unknown;

                    media_info.tracks.emplace_back(std::move(track));
                    break;

                case libvlc_track_audio:
                    track.type = track_type::audio;
                    if (track_list[i]->audio &&
                        (track_list[i]->audio->i_rate > 0) &&
                        (track_list[i]->audio->i_channels > 0))
                    {
                        track.audio.sample_rate = track_list[i]->audio->i_rate;
                        track.audio.channels = track_list[i]->audio->i_channels;

                        media_info.tracks.emplace_back(std::move(track));
                    }
                    break;

                case libvlc_track_video:
                    track.type = track_type::video;
                    if (track_list[i]->video &&
                        (track_list[i]->video->i_width > 0) &&
                        (track_list[i]->video->i_height > 0))
                    {
                        track.video.width = track_list[i]->video->i_width;
                        track.video.height = track_list[i]->video->i_height;
                        track.video.frame_rate =
                                float(track_list[i]->video->i_frame_rate_num) /
                                float(track_list[i]->video->i_frame_rate_den);

                        media_info.tracks.emplace_back(std::move(track));
                    }
                    break;

                case libvlc_track_text:
                    track.type = track_type::text;

                    media_info.tracks.emplace_back(std::move(track));
                    break;
                }
            }

        libvlc_media_tracks_release(track_list, count);
    }

    return media_info;
}

struct media_cache::media_info media_cache::media_info(class media &media) const
{
    std::unique_lock<std::mutex> l(mutex);

    const auto mrl = media.mrl();

    auto &data = cache[mrl];
    if (!data.media_info_read)
    {
        l.unlock();

        const auto media_info = media_info_from_media(media);

        l.lock();

        auto &data = cache[mrl];
        data.media_info = media_info;
        data.media_info_read = true;
        return data.media_info;
    }

    return data.media_info;
}

enum media_type media_cache::media_type(class media &media) const
{
    vlc::media_type result = vlc::media_type::unknown;

    bool has_audio = false, has_video = false;
    for (auto &i : media_info(media).tracks)
        switch (i.type)
        {
        case track_type::unknown:  break;
        case track_type::audio:    has_audio = true;   break;
        case track_type::video:    has_video = true;   break;
        case track_type::text:     break;
        }

    if (has_audio && has_video)
        result = vlc::media_type::video;
    else if (has_audio)
        result = vlc::media_type::audio;
    else if (has_video)
        result = vlc::media_type::picture;

    return result;
}

void media_cache::scan_files(const std::vector<std::string> &files)
{
    return scan_files(files, false);
}

void media_cache::scan_files_background(const std::vector<std::string> &files)
{
    std::unique_lock<std::mutex> l(mutex);

    for (auto &i : files)
        background_scan_queue.emplace_back(i);

    if (!background_scan_queue.empty() && background_scan_finished)
    {
        background_scan_finished = false;
        l.unlock();

        if (background_scan_thread.joinable())
            background_scan_thread.join();

        background_scan_thread = std::thread([this]
        {
            std::unique_lock<std::mutex> l(mutex);

            while (!background_scan_queue.empty())
            {
                std::unordered_set<std::string> mrls;
                std::vector<std::string> files;
                while (!background_scan_queue.empty())
                {
                    auto mrl = media::from_file(
                                instance,
                                background_scan_queue.back()).mrl();

                    if ((mrls.find(mrl) == mrls.end()) &&
                        (cache.find(mrl) == cache.end()))
                    {
                        mrls.emplace(std::move(mrl));
                        files.emplace_back(
                                    std::move(background_scan_queue.back()));
                    }

                    background_scan_queue.pop_back();
                }

                if (!files.empty())
                {
                    l.unlock();
                    scan_files(files, true);
                    l.lock();
                }
            }

            background_scan_finished = true;
        });
    }
}

void media_cache::scan_files(const std::vector<std::string> &files, bool background)
{
    const unsigned num_queues = std::max(
                std::min(
                    std::thread::hardware_concurrency(),
                    background ? 2u : unsigned(-1)),
                1u);

    std::vector<std::list<std::pair<std::string, media>>> tasks;
    tasks.resize(num_queues);

#ifdef PROCESS_USES_THREAD
    std::vector<vlc::instance> instances;
    instances.resize(num_queues);
#endif

    {
        std::lock_guard<std::mutex> _(mutex);

        unsigned index = 0;
        for (auto &i : files)
        {
            const auto queue_index = index % num_queues;
#ifdef PROCESS_USES_THREAD
            auto media = media::from_file(instances[queue_index], i);
#else
            auto media = media::from_file(instance, i);
#endif

            if (cache.find(media.mrl()) == cache.end())
            {
                tasks[queue_index].emplace_back(std::make_pair(i, std::move(media)));
                index++;
            }
        }
    }

    for (bool more = true; more;)
    {
        std::vector<platform::process> processes;
        processes.resize(num_queues);

        for (unsigned i = 0; i < num_queues; i++)
        {
            if (!tasks[i].empty())
            {
#ifdef PROCESS_USES_THREAD
                processes[i] = platform::process([this, tasks, i](platform::process &, std::ostream &out)
#else
                processes[i] = platform::process([this, &tasks, i](platform::process &, std::ostream &out)
#endif
                {
                    for (auto &task : tasks[i])
                    {
                        out << uuid_from_file(task.first) << ' ' << std::flush;
                        out << media_info_from_media(const_cast<class media &>(task.second)) << std::endl;
                    }
                }, background ? platform::process::priority::idle : platform::process::priority::low);
            }
        }

        for (unsigned i = 0; i < num_queues; i++)
        {
            while (!tasks[i].empty())
            {
                platform::uuid uuid;
                processes[i] >> uuid;
                struct media_info media_info;
                processes[i] >> media_info;

                {
                    std::lock_guard<std::mutex> _(mutex);

                    auto &data = cache[tasks[i].front().second.mrl()];
                    data.uuid = uuid;
                    data.uuid_generated = true;
                    data.media_info = media_info;
                    data.media_info_read = true;
                }

                tasks[i].pop_front();

                if (!processes[i])
                    break; // Process crashed while parsing this file.
            }

            if (processes[i].joinable())
                processes[i].join();
        }

        more = false;
        for (unsigned i = 0; (i < num_queues) && !more; i++)
            more |= !tasks[i].empty();
    }
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
      media_info_read(false)
{
}

media_cache::data::~data()
{
}


std::ostream & operator<<(std::ostream &str, const struct media_cache::track &track)
{
    str << track.id << ' '
        << '"' << to_percent(track.language) << '"' << ' '
        << '"' << to_percent(track.description) << '"' << ' '
        << int(track.type);

    switch (track.type)
    {
    case track_type::audio:
        str << ' ' << track.audio.sample_rate << ' ' << track.audio.channels;
        break;

    case track_type::video:
        str << ' ' << track.video.width << ' ' << track.video.height << ' ' << track.video.frame_rate;
        break;

    case track_type::unknown:
    case track_type::text:
        break;
    }

    return str;
}

std::istream & operator>>(std::istream &str, struct media_cache::track &track)
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
    track.type = track_type(type);

    switch (track.type)
    {
    case track_type::audio:
        str >> track.audio.sample_rate >> track.audio.channels;
        break;

    case track_type::video:
        str >> track.video.width >> track.video.height >> track.video.frame_rate;
        break;

    case track_type::unknown:
    case track_type::text:
        break;
    }

    return str;
}

std::ostream & operator<<(std::ostream &str, const struct media_cache::media_info &media_info)
{
    str << '{' << ' ';
    for (auto &i : media_info.tracks) str << i << ' ';
    str << '}' << ' ';

    str << media_info.duration.count() << ' ';
    str << media_info.chapter_count;

    return str;
}

std::istream & operator>>(std::istream &str, struct media_cache::media_info &media_info)
{
    std::string i;
    str >> i;
    if (i == "{")
    {
        while (str && (str.get() == ' ') && (str.peek() != '}'))
        {
            struct media_cache::track track;
            str >> track;
            if (str) media_info.tracks.emplace_back(track);
        }

        str >> i; // '}'
    }

    long long duration = 0;
    str >> duration;
    media_info.duration = std::chrono::milliseconds(duration);

    str >> media_info.chapter_count;

    return str;
}

} // End of namespace
