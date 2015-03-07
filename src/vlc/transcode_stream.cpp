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

#include "vlc/transcode_stream.h"
#include "vlc/media.h"
#include "vlc/instance.h"
#include "platform/path.h"
#include "platform/string.h"
#include <vlc/vlc.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <thread>

namespace vlc {

platform::process::function_handle transcode_stream::transcode_function =
        platform::process::register_function(&transcode_stream::transcode_process);

struct transcode_stream::shared_info
{
    libvlc_time_t time;
    bool end_reached;
};

transcode_stream::transcode_stream(class platform::messageloop_ref &messageloop)
    : std::istream(nullptr),
      messageloop(messageloop),
      font_size(-1),
      chapter(-1),
      position(-1),
      info_offset(-1),
      update_info_timer(messageloop, std::bind(&transcode_stream::update_info, this))
{
}

transcode_stream::~transcode_stream()
{
    close();
}

void transcode_stream::add_option(const std::string &option)
{
    options.emplace_back(option);
}

void transcode_stream::set_font_size(int s)
{
    font_size = s;
}

void transcode_stream::set_chapter(int ch)
{
    chapter = ch;
    position = std::chrono::milliseconds(-1);
}

void transcode_stream::set_position(std::chrono::milliseconds pos)
{
    chapter = -1;
    position = pos;
}

void transcode_stream::set_track_ids(const struct track_ids &ids)
{
    track_ids = ids;
}

void transcode_stream::set_subtitle_file(subtitles::file &&file)
{
    subtitle_file = std::move(file);
}

int transcode_stream::transcode_process(platform::process &process)
{
    std::vector<std::string> vlc_options;
    vlc_options.push_back("--no-sub-autodetect-file");
    vlc_options.push_back("--avcodec-fast");

    int font_size = -1;
    process >> font_size;
    if (font_size > 0)
    {
        vlc_options.push_back("--freetype-fontsize");
        vlc_options.push_back(std::to_string(font_size));
    }

    vlc::instance instance(vlc_options);

    std::string mrl, transcode, mux;
    process >> mrl >> transcode >> mux;

    std::ostringstream sout;
    sout
            << ":sout=" << from_percent(transcode)
            << ":std{access=fd,mux=" << mux << ",dst=" << process.output_fd() << "}";

    auto media = media::from_mrl(instance, mrl);
    libvlc_media_add_option(media, sout.str().c_str());

    std::string subtitle_file;
    process >> subtitle_file;
    if (subtitle_file != "(none)")
    {
        std::string file = from_percent(subtitle_file);
#ifdef WIN32
        file = from_utf16(platform::to_windows_path(file));
#endif
        libvlc_media_add_option(media, (":sub-file=" + file).c_str());
    }

    size_t num_options = 0;
    process >> num_options;
    for (size_t i = 0; i < num_options; i++)
    {
        std::string option;
        process >> option;
        libvlc_media_add_option(media, from_percent(option).c_str());
    }

    struct T
    {
        static void callback(const libvlc_event_t *e, void *opaque)
        {
            T * const t = reinterpret_cast<T *>(opaque);
            std::lock_guard<std::mutex> _(t->mutex);

            if (e->type == libvlc_MediaPlayerTimeChanged)
            {
                if (t->info)
                    t->info->time = e->u.media_player_time_changed.new_time;
            }
            else if (e->type == libvlc_MediaPlayerPlaying)
            {
                t->started = true;

                if (t->track_ids.video >= -1)
                {
                    libvlc_video_set_track(t->player, -1);
                    if (t->track_ids.video > 0)
                        libvlc_video_set_track(t->player, t->track_ids.video);
                }

                if (t->track_ids.audio >= -1)
                {
                    libvlc_audio_set_track(t->player, -1);
                    if (t->track_ids.audio >= 0)
                        libvlc_audio_set_track(t->player, t->track_ids.audio);
                }

                if (t->track_ids.text >= -1)
                {
                    libvlc_video_set_spu(t->player, -1);
                    if (t->track_ids.text >= 0)
                        libvlc_video_set_spu(t->player, t->track_ids.text);
                }
            }
            else if (e->type == libvlc_MediaPlayerEndReached)
            {
                t->end_reached = true;
                if (t->info)
                    t->info->end_reached = true;
            }
            else if (e->type == libvlc_MediaPlayerEncounteredError)
                t->encountered_error = true;

            t->condition.notify_one();
        }

        volatile shared_info *info;
        struct track_ids track_ids;
        std::mutex mutex;
        std::condition_variable condition;
        bool started;
        bool end_reached;
        bool encountered_error;

        libvlc_media_player_t *player;
    } t;

    unsigned info_offset(-1);
    process >> info_offset;
    process >> t.track_ids.audio >> t.track_ids.video >> t.track_ids.text;

    int chapter = -1;
    int64_t position = -1;
    float rate = 0.0f;
    process >> chapter >> position >> rate;

    t.info = &process.get_shared<shared_info>(info_offset);
    t.started = false;
    t.end_reached = false;
    t.encountered_error = false;

    t.player = libvlc_media_player_new_from_media(media);
    if (t.player)
    {
        auto event_manager = libvlc_media_player_event_manager(t.player);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerTimeChanged, &T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerPlaying, &T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEndReached, &T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEncounteredError, &T::callback, &t);

        if (libvlc_media_player_play(t.player) == 0)
        {
            std::unique_lock<std::mutex> l(t.mutex);

            while (!t.end_reached && !t.encountered_error && process)
            {
                if (t.started)
                {
                    l.unlock();

                    if (chapter >= 0)
                        libvlc_media_player_set_chapter(t.player, chapter);

                    if (position > 0)
                        libvlc_media_player_set_time(t.player, position);

                    if (std::abs(rate - 1.0f) > 0.01f)
                        libvlc_media_player_set_rate(t.player, rate);

                    l.lock();
                    t.started = false;
                }

                t.condition.wait(l);
            }

            l.unlock();
            libvlc_media_player_stop(t.player);
        }

        libvlc_event_detach(event_manager, libvlc_MediaPlayerEncounteredError, &T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEndReached, &T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerPlaying, &T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerTimeChanged, &T::callback, &t);

        libvlc_media_player_release(t.player);
    }

    return 0;
}

bool transcode_stream::open(
        const std::string &mrl,
        const std::string &transcode,
        const std::string &mux,
        float rate)
{
    close();

    std::clog << "vlc::transcode_stream: " << transcode << std::endl;

    process.reset(new platform::process(transcode_function));
    *process << font_size << ' '
             << mrl << ' '
             << to_percent(transcode) << ' '
             << mux << std::endl;

    if (subtitle_file)
        *process << to_percent(subtitle_file) << std::endl;
    else
        *process << "(none)" << std::endl;

    *process << options.size();
    for (auto &i : options) *process << ' ' << to_percent(i);
    *process << std::endl;

    info_offset = process->alloc_shared<shared_info>();
    *process << info_offset << std::endl;

    *process << track_ids.audio << ' '
             << track_ids.video << ' '
             << track_ids.text << std::endl;

    *process << chapter << ' '
             << position.count() << ' '
             << rate << std::endl;

    last_info.reset(new shared_info());
    update_info_timer.start(std::chrono::seconds(5));

    std::istream::rdbuf(process->rdbuf());
    return true;
}

void transcode_stream::close()
{
    std::istream::rdbuf(nullptr);

    if (process)
    {
        update_info_timer.stop();

        if (process->joinable())
        {
            // Flush pipe to prevent deadlock while stopping player.
            std::thread flush([this] { while (*process) process->get(); });

            process->send_term();
            process->join();

            flush.join();
        }

        update_info();

        process = nullptr;
        last_info = nullptr;
        info_offset = unsigned(-1);
    }
}

std::chrono::milliseconds transcode_stream::playback_position() const
{
    if (process)
        return std::chrono::milliseconds(
                    process->get_shared<shared_info>(info_offset).time);

    return std::chrono::milliseconds(0);
}

bool transcode_stream::end_reached() const
{
    if (process)
        return process->get_shared<shared_info>(info_offset).end_reached;

    return false;
}

void transcode_stream::update_info()
{
    if (process && last_info)
    {
        const auto new_info = const_cast<const shared_info &>(
                    process->get_shared<shared_info>(info_offset));

        if (on_playback_position_changed && (new_info.time != last_info->time))
            on_playback_position_changed(std::chrono::milliseconds(new_info.time));

        if (on_end_reached && (new_info.end_reached != last_info->end_reached))
            on_end_reached();

        *last_info = new_info;
    }
}

} // End of namespace
