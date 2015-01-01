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

#include "transcode_stream.h"
#include "media.h"
#include "instance.h"
#include "platform/process.h"
#include <vlc/vlc.h>
#include <cmath>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#if defined(__unix__) || defined(__APPLE__)
# include <unistd.h>
#elif defined(WIN32)
# include <io.h>
#endif

namespace vlc {

struct transcode_stream::shared_info
{
    libvlc_time_t time;
};

transcode_stream::transcode_stream(class platform::messageloop_ref &messageloop, class instance &instance)
    : std::istream(nullptr),
      messageloop(messageloop),
      instance(instance),
      chapter(-1),
      position(-1),
      info(platform::process::make_shared<shared_info>()),
      last_info(std::make_shared<shared_info>()),
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

bool transcode_stream::open(
        const std::string &mrl,
        const std::string &transcode,
        const std::string &mux,
        float rate)
{
    close();

    std::clog << "vlc::transcode_stream: " << transcode << std::endl;

    process.reset(new platform::process([this, mrl, transcode, mux, rate](platform::process &process, int fd)
    {
        auto media = media::from_mrl(instance, mrl);

        std::ostringstream sout;
        sout
                << ":sout=" << transcode
                << ":std{access=fd,mux=" << mux << ",dst=" << fd << "}";

        libvlc_media_add_option(media, sout.str().c_str());
        for (auto &option : options)
            libvlc_media_add_option(media, option.c_str());

        struct T
        {
            static void callback(const libvlc_event_t *e, void *opaque)
            {
                T * const t = reinterpret_cast<T *>(opaque);
                std::lock_guard<std::mutex> _(t->mutex);

                if (e->type == libvlc_MediaPlayerTimeChanged)
                {
                    if (t->me->info)
                        t->me->info->time = e->u.media_player_time_changed.new_time;
                }
                else if (e->type == libvlc_MediaPlayerPlaying)
                {
                    t->started = true;

//                        libvlc_video_set_track(t->player, -1);
//                        if (t->me->track_ids.video > 0)
//                            libvlc_video_set_track(t->player, t->me->track_ids.video);

                    libvlc_audio_set_track(t->player, -1);
                    if (t->me->track_ids.audio >= 0)
                        libvlc_audio_set_track(t->player, t->me->track_ids.audio);

                    libvlc_video_set_spu(t->player, -1);
                    if (t->me->track_ids.text >= 0)
                        libvlc_video_set_spu(t->player, t->me->track_ids.text);
                }
                else if (e->type == libvlc_MediaPlayerEndReached)
                    t->end_reached = true;
                else if (e->type == libvlc_MediaPlayerEncounteredError)
                    t->encountered_error = true;

                t->condition.notify_one();
            }

            transcode_stream *me;
            std::mutex mutex;
            std::condition_variable condition;
            bool started;
            bool end_reached;
            bool encountered_error;

            libvlc_media_player_t *player;
        } t;

        t.me = this;
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

                while (!t.end_reached && !t.encountered_error && !process.term_pending())
                {
                    if (t.started)
                    {
                        l.unlock();

                        if (chapter >= 0)
                            libvlc_media_player_set_chapter(t.player, chapter);

                        if (position.count() > 0)
                            libvlc_media_player_set_time(t.player, position.count());

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

        ::close(fd);
    }));

    update_info_timer.start(std::chrono::seconds(5));

    std::istream::rdbuf(process->rdbuf());
    return true;
}

void transcode_stream::close()
{
    std::istream::rdbuf(nullptr);

    update_info_timer.stop();

    if (process)
    {
        if (process->joinable())
        {
            // Flush pipe to prevent deadlock while stopping player.
            std::thread flush([this] { while (*process) process->get(); });

            process->send_term();
            process->join();

            flush.join();
        }

        process = nullptr;
    }
}

std::chrono::milliseconds transcode_stream::playback_position() const
{
    if (info)
        return std::chrono::milliseconds(info->time);

    return std::chrono::milliseconds(0);
}

void transcode_stream::update_info()
{
    if (info && last_info)
    {
        if (on_playback_position_changed && (info->time != last_info->time))
            on_playback_position_changed(playback_position());

        *last_info = *info;
    }
}

} // End of namespace
