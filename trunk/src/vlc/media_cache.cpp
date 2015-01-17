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
#include <mutex>
#include <sstream>
#include <unordered_set>

namespace vlc {

static const char revision_name[] = "rev_1";

media_cache::media_cache(
        class platform::messageloop_ref &messageloop,
        class instance &instance)
    : instance(instance),
      inifile(platform::config_dir() + "/media_cache"),
      timer(messageloop, std::bind(&platform::inifile::save, &inifile)),
      save_delay(5),
      section(inifile.open_section(revision_name)),
      background_scan_finished(true)
{
    inifile.on_touched = [this] { timer.start(save_delay, true); };

    for (auto &i : inifile.sections())
        if (i != revision_name)
            inifile.erase_section(i);
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

platform::uuid media_cache::uuid(const std::string &mrl)
{
    auto i = uuids.find(mrl);
    if (i == uuids.end())
    {
        const auto uuid = uuid_from_file(platform::path_from_mrl(mrl));
        uuids[mrl] = uuid;
        return uuid;
    }
    else
        return i->second;
}

static bool should_read_media_info_from_player(const std::string &path)
{
    platform::ifstream file(path, std::ios_base::binary);
    if (file.is_open())
    {
        uint8_t buffer[16];
        if (file.read(reinterpret_cast<char *>(buffer), sizeof(buffer)))
        {
            // MPEG TS
            if (buffer[0] == 0x47)
                return true;

            // MPEG PS
            if ((buffer[0] == 0x00) && (buffer[1] == 0x00) &&
                (buffer[2] == 0x01) && (buffer[3] >= 0xB9))
            {
                return true;
            }
        }
    }

    return false;
}

static void read_media_info_from_player(
        class media &media,
        struct media_cache::media_info &media_info)
{
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
        t.pixel_buffer.resize((width * height * sizeof(uint16_t)) + align);

        libvlc_media_player_set_rate(player, 30.0f);

        libvlc_audio_set_callbacks(player, &T::play, nullptr, nullptr, nullptr, nullptr, &t);
        libvlc_audio_set_format(player, "S16N", 44100, 2);
        libvlc_video_set_callbacks(player, &T::lock, nullptr, nullptr, &t);
        libvlc_video_set_format(player, "YUYV", width, height, width * sizeof(uint16_t));

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
            while ((t.new_time < 750) && !t.stopped) t.condition.wait(l);

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

        media_info.duration = std::max(
                    std::chrono::milliseconds(t.new_length),
                    media_info.duration);
    }
}

static bool read_track_list(
        class media &media,
        struct media_cache::media_info &media_info)
{
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

    return track_list && (count > 0);
}

static struct media_cache::media_info media_info_from_media(
    class media &media)
{
    struct media_cache::media_info media_info;

    libvlc_media_parse(media);

    media_info.duration = std::chrono::milliseconds(
                libvlc_media_get_duration(media));

    const auto path = platform::path_from_mrl(media.mrl());
    if (should_read_media_info_from_player(path))
        read_media_info_from_player(media, media_info);

    read_track_list(media, media_info);

    return media_info;
}

struct media_cache::media_info media_cache::media_info(class media &media)
{
    const auto mrl = media.mrl();
    const auto uuid = this->uuid(mrl);

    if (section.has_value(uuid))
    {
        std::stringstream str(section.read(uuid));
        struct media_info media_info;
        str >> media_info;
        return media_info;
    }
    else
    {
        const auto media_info = media_info_from_media(media);

        std::ostringstream str;
        str << media_info;
        section.write(uuid, str.str());

        return media_info;
    }
}

enum media_type media_cache::media_type(class media &media)
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
    const unsigned num_queues = std::max(
                std::thread::hardware_concurrency(),
                1u);

    std::vector<std::list<media>> tasks;
    tasks.resize(num_queues);

#ifdef PROCESS_USES_THREAD
    std::mutex mutex;
    std::vector<vlc::instance> instances;
    instances.resize(num_queues);
#endif

    unsigned index = 0;
    for (auto &i : files)
    {
        const auto queue_index = index % num_queues;
#ifdef PROCESS_USES_THREAD
        auto media = media::from_file(instances[queue_index], i);
#else
        auto media = media::from_file(instance, i);
#endif

        tasks[queue_index].emplace_back(std::move(media));
        index++;
    }

    for (bool more = true; more;)
    {
        std::vector<std::unique_ptr<platform::process>> processes;

        for (unsigned i = 0; i < num_queues; i++)
        {
            if (!tasks[i].empty())
            {
#ifdef PROCESS_USES_THREAD
                processes.emplace_back(new platform::process([this, &mutex, tasks, i](platform::process &, std::ostream &out)
#else
                processes.emplace_back(new platform::process([this, &tasks, i](platform::process &, std::ostream &out)
#endif
                {
                    for (auto &task : tasks[i])
                    {
                        const std::string mrl = task.mrl();
                        platform::uuid uuid;

#ifdef PROCESS_USES_THREAD
                        std::unique_lock<std::mutex> l(mutex);
#else
                        struct { void lock() {} void unlock() {} } l;
#endif

                        auto i = uuids.find(mrl);
                        if (i == uuids.end())
                        {
                            l.unlock();
                            uuid = uuid_from_file(platform::path_from_mrl(mrl));
                            l.lock();
                        }
                        else
                            uuid = i->second;

                        if (!uuid.is_null() && !section.has_value(uuid))
                        {
                            l.unlock();
                            out << uuid << ' ' << std::flush;
                            out << media_info_from_media(const_cast<class media &>(task)) << std::endl;
                        }
                        else
                        {
                            l.unlock();
                            out << platform::uuid() << std::endl;
                        }
                    }
                }));
            }
            else
                processes.emplace_back(nullptr);
        }

        for (unsigned i = 0; i < num_queues; i++)
        {
            while (!tasks[i].empty())
            {
                platform::uuid uuid;
                *processes[i] >> uuid;
                if (!uuid.is_null())
                {
#ifdef PROCESS_USES_THREAD
                    std::unique_lock<std::mutex> l(mutex);
#else
                    struct { void lock() {} void unlock() {} } l;
#endif

                    uuids[tasks[i].front().mrl()] = uuid;

                    l.unlock();
                    struct media_info media_info;
                    *processes[i] >> media_info;
                    l.lock();

                    std::ostringstream str;
                    str << media_info;
                    section.write(uuid, str.str());
                }

                tasks[i].pop_front();

                if (!*processes[i])
                    break; // Process crashed while parsing this file.
            }

            if (processes[i] && processes[i]->joinable())
                processes[i]->join();
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
