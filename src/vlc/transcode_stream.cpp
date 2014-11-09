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
#include <vlc/vlc.h>
#include <cassert>
#include <cmath>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <streambuf>
#include <thread>
#include <vector>

static void pipe_create(int(&)[2]);
static size_t pipe_read(int, void *, size_t);
static void pipe_close(int &);

namespace vlc {

class transcode_stream::streambuf : public std::streambuf
{
public:
    streambuf(
            class transcode_stream &,
            const std::string &,
            int,
            const struct track_ids &,
            const std::string &,
            const std::string &,
            float);

    streambuf(
            class transcode_stream &,
            const std::string &,
            std::chrono::milliseconds,
            const struct track_ids &,
            const std::string &,
            const std::string &,
            float);

    ~streambuf();

    bool is_open() const { return player != nullptr; }

    int underflow() override;

private:
    streambuf(
            class transcode_stream &,
            const std::string &,
            const struct track_ids &,
            const std::string &,
            const std::string &,
            float);

    void started();
    void progress(std::chrono::milliseconds);
    void stop();

    static void callback(const libvlc_event_t *, void *);

private:
    class transcode_stream &parent;
    const struct track_ids track_ids;
    class media media;
    libvlc_media_player_t *player;
    libvlc_event_manager_t *event_manager;
    int chapter;
    std::chrono::milliseconds position;
    const float rate;

    std::mutex mutex;
    int pipe[2];
    static const size_t putback = 8;
    char buffer[putback + 65536];
};

transcode_stream::transcode_stream(class platform::messageloop_ref &messageloop, class instance &instance)
    : std::istream(nullptr),
      messageloop(messageloop),
      instance(instance)
{
}

transcode_stream::~transcode_stream()
{
    delete std::istream::rdbuf(nullptr);
}

void transcode_stream::add_option(const std::string &option)
{
    options.emplace_back(option);
}

bool transcode_stream::open(const std::string &mrl,
        int chapter,
        const struct track_ids &track_ids,
        const std::string &transcode,
        const std::string &mux,
        float rate)
{
    close();

    auto * const streambuf = new class streambuf(*this, mrl, chapter, track_ids, transcode, mux, rate);
    if (streambuf->is_open())
    {
        std::istream::rdbuf(streambuf);
        return true;
    }

    delete streambuf;
    return false;
}

bool transcode_stream::open(
        const std::string &mrl,
        std::chrono::milliseconds position,
        const struct track_ids &track_ids,
        const std::string &transcode,
        const std::string &mux,
        float rate)
{
    close();

    auto * const streambuf = new class streambuf(*this, mrl, position, track_ids, transcode, mux, rate);
    if (streambuf->is_open())
    {
        std::istream::rdbuf(streambuf);
        return true;
    }

    delete streambuf;
    return false;
}

void transcode_stream::close()
{
    delete std::istream::rdbuf(nullptr);
}


transcode_stream::streambuf::streambuf(
        class transcode_stream &parent,
        const std::string &mrl,
        const struct track_ids &track_ids,
        const std::string &transcode,
        const std::string &mux,
        float rate)
    : parent(parent),
      track_ids(track_ids),
      media(media::from_mrl(parent.instance, mrl)),
      player(nullptr),
      event_manager(nullptr),
      chapter(-1),
      position(0),
      rate(rate)
{
    std::lock_guard<std::mutex> _(mutex);

    pipe_create(pipe);
    memset(buffer, 0, sizeof(buffer));

    std::ostringstream sout;
    sout
            << ":sout=" << transcode
            << ":std{access=fd,mux=" << mux << ",dst=" << pipe[1] << "}";

    libvlc_media_add_option(media, sout.str().c_str());
    for (auto &option : parent.options)
        libvlc_media_add_option(media, option.c_str());

    player = libvlc_media_player_new_from_media(media);
    if (player)
    {
        event_manager = libvlc_media_player_event_manager(player);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerPlaying, &streambuf::callback, this);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEndReached, &streambuf::callback, this);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEncounteredError, &streambuf::callback, this);

        if (parent.on_playback_progress)
            libvlc_event_attach(event_manager, libvlc_MediaPlayerTimeChanged, &streambuf::callback, this);

        std::clog << '[' << ((void *)this) << "] " << sout.str() << std::endl;
    }
}

transcode_stream::streambuf::streambuf(
        class transcode_stream &parent,
        const std::string &mrl,
        int chapter,
        const struct track_ids &track_ids,
        const std::string &transcode,
        const std::string &mux,
        float rate)
      : streambuf(parent, mrl, track_ids, transcode, mux, rate)
{
    this->chapter = chapter;

    if (player)
    {
        libvlc_media_player_play(player);

        std::clog << '[' << ((void *)this) << "] opened transcode_stream " << mrl << "@Chapter " << chapter << std::endl;
    }
}

transcode_stream::streambuf::streambuf(
        class transcode_stream &parent,
        const std::string &mrl,
        std::chrono::milliseconds position,
        const struct track_ids &track_ids,
        const std::string &transcode,
        const std::string &mux,
        float rate)
      : streambuf(parent, mrl, track_ids, transcode, mux, rate)
{
    this->position = position;

    if (player)
    {
        libvlc_media_player_play(player);

        std::clog << '[' << ((void *)this) << "] opened transcode_stream " << mrl << '@' << position.count() << std::endl;
    }
}

transcode_stream::streambuf::~streambuf()
{
    stop();

    std::clog << '[' << ((void *)this) << "] destroyed transcode_stream " << media.mrl() << std::endl;
}

void transcode_stream::streambuf::started()
{
    std::lock_guard<std::mutex> _(mutex);

    if (track_ids.video >= 0)   libvlc_video_set_track(player, track_ids.video);
    if (track_ids.audio >= 0)   libvlc_audio_set_track(player, track_ids.audio);
    libvlc_video_set_spu(player, track_ids.text);

    if (chapter >= 0)           libvlc_media_player_set_chapter(player, chapter);
    if (position.count() > 0)   libvlc_media_player_set_time(player, position.count());

    if (std::abs(rate - 1.0f) > 0.01f) libvlc_media_player_set_rate(player, rate);
}

void transcode_stream::streambuf::progress(std::chrono::milliseconds pos)
{
    std::lock_guard<std::mutex> _(mutex);

    if (player && parent.on_playback_progress)
        parent.on_playback_progress(pos);
}

void transcode_stream::streambuf::stop()
{
    std::unique_lock<std::mutex> l(mutex);

    if (player)
    {
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEncounteredError, &streambuf::callback, this);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEndReached, &streambuf::callback, this);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerPlaying, &streambuf::callback, this);

        if (parent.on_playback_progress)
            libvlc_event_detach(event_manager, libvlc_MediaPlayerTimeChanged, &streambuf::callback, this);

        if (pipe[1] >= 0)
        {
            l.unlock();

            // Flush pipe to prevent deadlock while stopping player.
            std::thread flush_thread([this]
            {
                char flush_buffer[64];
                for (;;)
                {
                    const size_t r = pipe_read(pipe[0], flush_buffer, sizeof(flush_buffer));
                    if ((r == 0) || (r == size_t(-1)))
                        break;
                }
            });

            libvlc_media_player_stop(player);

            pipe_close(pipe[1]);
            pipe[1] = -1;

            flush_thread.join();
            l.lock();
        }

        libvlc_media_player_release(player);
        player = nullptr;
    }

    if (pipe[1] >= 0) pipe_close(pipe[1]);
    if (pipe[0] >= 0) pipe_close(pipe[0]);

    pipe[1] = pipe[0] = -1;
}

int transcode_stream::streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*gptr());

    const size_t read = pipe_read(pipe[0], buffer + putback, sizeof(buffer) - putback);
    if ((read > 0) && (read != size_t(-1)))
    {
        setg(buffer, buffer + putback, buffer + putback + read);
        return traits_type::to_int_type(*gptr());
    }
    else
        return traits_type::eof();
}

void transcode_stream::streambuf::callback(const libvlc_event_t *e, void *opaque)
{
    streambuf * const me = reinterpret_cast<streambuf *>(opaque);

    if (e->type == libvlc_MediaPlayerTimeChanged)
    {
        const auto time = libvlc_media_player_get_time(me->player);
        if (time >= 0)
        {
            const std::chrono::milliseconds pos(time);
            me->parent.messageloop.post(std::bind(&transcode_stream::streambuf::progress, me, pos));
        }
    }
    else if (e->type == libvlc_MediaPlayerPlaying)
    {
        me->parent.messageloop.post(std::bind(&transcode_stream::streambuf::started, me));
    }
    else if (e->type == libvlc_MediaPlayerEndReached)
    {
        me->parent.messageloop.post(std::bind(&transcode_stream::streambuf::stop, me));

        const std::chrono::milliseconds pos(-1);
        me->parent.messageloop.post(std::bind(&transcode_stream::streambuf::progress, me, pos));
    }
    else if (e->type == libvlc_MediaPlayerEndReached)
    {
        me->parent.messageloop.post(std::bind(&transcode_stream::streambuf::stop, me));
    }
}

} // End of namespace

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>

static void pipe_create(int(& filedes)[2])
{
    if (pipe(filedes) != 0)
        throw std::runtime_error("Creating pipe failed");
}
#elif defined(WIN32)
#include <fcntl.h>
#include <io.h>

static void pipe_create(int(& filedes)[2])
{
    if (_pipe(filedes, 65536, _O_BINARY) != 0)
        throw std::runtime_error("Creating pipe failed");
}
#endif

static size_t pipe_read(int filedes, void *buf, size_t nbyte)
{
    return read(filedes, buf, nbyte);
}

static void pipe_close(int &filedes)
{
    close(filedes);
    filedes = 0;
}
