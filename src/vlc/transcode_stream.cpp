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

static const size_t buffer_size = 1048576;

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

    size_t m2ts_filter(int, void *, size_t);
    void consume();

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

    std::unique_ptr<std::thread> consume_thread;
    std::mutex mutex;

    bool stream_start_pending;
    bool stream_end_pending;
    bool stream_end;
    bool end_reached;
    int pipe[2];

    std::vector<char> buffer;
    std::condition_variable buffer_condition;
    size_t buffer_pos;
    size_t buffer_used;
    size_t buffer_visible;

    const bool m2ts_filter_enabled;
    std::vector<char> m2ts_filter_buffer;
    size_t m2ts_filter_buffer_pos;
};

transcode_stream::transcode_stream(class platform::messageloop_ref &messageloop, class instance &instance)
    : std::istream(nullptr),
      messageloop(messageloop),
      instance(instance),
      streambuf(nullptr)
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

bool transcode_stream::open(const std::string &mrl,
        int chapter,
        const struct track_ids &track_ids,
        const std::string &transcode,
        const std::string &mux,
        float rate)
{
    streambuf.reset(new class streambuf(*this, mrl, chapter, track_ids, transcode, mux, rate));
    if (streambuf->is_open())
    {
        std::istream::rdbuf(streambuf.get());

        return true;
    }

    streambuf = nullptr;
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
    streambuf.reset(new class streambuf(*this, mrl, position, track_ids, transcode, mux, rate));
    if (streambuf->is_open())
    {
        std::istream::rdbuf(streambuf.get());

        return true;
    }

    streambuf = nullptr;
    return false;
}

void transcode_stream::close()
{
    std::istream::rdbuf(nullptr);
    streambuf = nullptr;
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
      rate(rate),
      stream_start_pending(false),
      stream_end_pending(false),
      stream_end(false),
      end_reached(false),
      buffer_pos(0),
      buffer_used(0),
      buffer_visible(0),
      m2ts_filter_enabled(mux == "m2ts"),
      m2ts_filter_buffer_pos(0)
{
    pipe_create(pipe);

    const std::string vlc_mux = m2ts_filter_enabled ? "ts" : mux;

    std::ostringstream sout;
    sout
            << ":sout=" << transcode
            << ":std{access=fd,mux=" << vlc_mux << ",dst=" << pipe[1] << "}";

    libvlc_media_add_option(media, sout.str().c_str());
    for (auto &option : parent.options)
        libvlc_media_add_option(media, option.c_str());

    player = libvlc_media_player_new_from_media(media);
    if (player)
    {
        buffer.resize(buffer_size);

        event_manager = libvlc_media_player_event_manager(player);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerPlaying, &streambuf::callback, this);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEndReached, &streambuf::callback, this);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEncounteredError, &streambuf::callback, this);

        if (parent.on_playback_progress)
            libvlc_event_attach(event_manager, libvlc_MediaPlayerTimeChanged, &streambuf::callback, this);

        consume_thread.reset(new std::thread(std::bind(&transcode_stream::streambuf::consume, this)));

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
    if (player)
    {
        std::unique_lock<std::mutex> l(mutex);

        libvlc_event_detach(event_manager, libvlc_MediaPlayerEncounteredError, &streambuf::callback, this);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEndReached, &streambuf::callback, this);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerPlaying, &streambuf::callback, this);

        if (parent.on_playback_progress)
            libvlc_event_detach(event_manager, libvlc_MediaPlayerTimeChanged, &streambuf::callback, this);

        if (pipe[1] >= 0)
        {
            stream_end = true;
            buffer_condition.notify_all();

            l.unlock();
            libvlc_media_player_stop(player);
            l.lock();

            pipe_close(pipe[1]);
            pipe[1] = -1;
        }

        l.unlock();
        consume_thread->join();

        libvlc_media_player_release(player);
    }

    if (pipe[1] >= 0) pipe_close(pipe[1]);
    if (pipe[0] >= 0) pipe_close(pipe[0]);

    std::clog << '[' << ((void *)this) << "] destroyed transcode_stream " << media.mrl() << std::endl;
}

size_t transcode_stream::streambuf::m2ts_filter(int filedes, void *buf, size_t nbyte)
{
    static const size_t ts_packet_size = 188;
    static const char ts_sync_byte = 0x47;
    static const uint32_t timestamp = 0;

    if (nbyte > 0)
    {
        if (m2ts_filter_buffer_pos == 0)
        {
            m2ts_filter_buffer.resize(sizeof(timestamp) + ts_packet_size);
            memcpy(&m2ts_filter_buffer[0], &timestamp, sizeof(timestamp));

            for (size_t i = sizeof(timestamp); i < m2ts_filter_buffer.size(); )
            {
                const size_t c = pipe_read(filedes, &m2ts_filter_buffer[i], m2ts_filter_buffer.size() - i);
                if ((c > 0) && (c != size_t(-1)))
                    i += c;
                else
                    return c;

                assert(m2ts_filter_buffer[sizeof(timestamp)] == ts_sync_byte);
                while ((i > sizeof(timestamp)) && (m2ts_filter_buffer[sizeof(timestamp)] != ts_sync_byte))
                    memmove(&m2ts_filter_buffer[sizeof(timestamp) + 1], &m2ts_filter_buffer[sizeof(timestamp)], --i);
            }
        }

        const size_t result = std::min(nbyte, m2ts_filter_buffer.size() - m2ts_filter_buffer_pos);
        memcpy(buf, &m2ts_filter_buffer[m2ts_filter_buffer_pos], result);
        m2ts_filter_buffer_pos = (m2ts_filter_buffer_pos + result) % m2ts_filter_buffer.size();
        return result;
    }

    return 0;
}

void transcode_stream::streambuf::consume()
{
    {
        std::unique_lock<std::mutex> l(mutex);

        while (!stream_end)
        {
            char * write_block = nullptr;
            size_t write_block_size = 0;
            do
            {
                if (buffer_used < buffer.size())
                {
                    const size_t write_block_pos = (buffer_pos + buffer_used) % buffer.size();
                    write_block = &buffer[write_block_pos];
                    write_block_size = buffer.size() - write_block_pos;
                }
                else
                    buffer_condition.wait(l);
            } while (!stream_end && !write_block);

            if (!stream_end)
            {
                l.unlock();
                assert(write_block_size > 0);
                const size_t read = m2ts_filter_enabled
                        ? m2ts_filter(pipe[0], write_block, write_block_size)
                        : pipe_read(pipe[0], write_block, write_block_size);
                l.lock();

                if ((read > 0) && (read != size_t(-1)))
                {
                    buffer_used += read;
                    buffer_condition.notify_one();
                }
                else
                {
                    stream_end = true;
                    buffer_condition.notify_all();
                }
            }
        }
    }

    // Flush pipe to prevent deadlock while stopping player.
    char flush_buffer[64];
    for (;;)
    {
        const size_t r = pipe_read(pipe[0], flush_buffer, sizeof(flush_buffer));
        if ((r == 0) || (r == size_t(-1)))
            break;
    }
}

int transcode_stream::streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*gptr());

    std::unique_lock<std::mutex> l(mutex);

    buffer_pos = (buffer_pos + buffer_visible) % buffer.size();
    buffer_used -= buffer_visible;
    buffer_visible = 0;
    buffer_condition.notify_one();

    while (!stream_end && (buffer_used == 0))
    {
        if (stream_start_pending)
        {
            if (track_ids.video >= 0)   libvlc_video_set_track(player, track_ids.video);
            if (track_ids.audio >= 0)   libvlc_audio_set_track(player, track_ids.audio);
            libvlc_video_set_spu(player, track_ids.text);

            if (chapter >= 0)           libvlc_media_player_set_chapter(player, chapter);
            if (position.count() > 0)   libvlc_media_player_set_time(player, position.count());

            if (std::abs(rate - 1.0f) > 0.01f) libvlc_media_player_set_rate(player, rate);

            stream_start_pending = false;
        }

        if (stream_end_pending)
        {
            libvlc_media_player_stop(player);
            pipe_close(pipe[1]);
            pipe[1] = -1;
            stream_end_pending = false;
        }

        buffer_condition.wait(l);
    }

    if (!stream_end)
    {
        buffer_visible = std::min(buffer.size() - buffer_pos, buffer_used);
        setg(&buffer[buffer_pos], &buffer[buffer_pos], &buffer[buffer_pos + buffer_visible]);
        return traits_type::to_int_type(*gptr());
    }

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
            me->parent.messageloop.post([me, time]
            {
                if (!me->end_reached && me->parent.on_playback_progress)
                {
                    const std::chrono::milliseconds pos(time);
                    me->parent.on_playback_progress(pos);
                }
            });
        }
    }
    else if (e->type == libvlc_MediaPlayerPlaying)
    {
        std::lock_guard<std::mutex> _(me->mutex);
        me->stream_start_pending = true;
        me->buffer_condition.notify_all();
    }
    else if (e->type == libvlc_MediaPlayerEndReached)
    {
        std::lock_guard<std::mutex> _(me->mutex);
        me->stream_end_pending = true;
        me->buffer_condition.notify_all();

        me->parent.messageloop.post([me]
        {
            me->end_reached = true;
            if (me->parent.on_playback_progress)
                me->parent.on_playback_progress(std::chrono::milliseconds(-1));
        });
    }
    else if (e->type == libvlc_MediaPlayerEncounteredError)
    {
        std::lock_guard<std::mutex> _(me->mutex);
        me->stream_end_pending = true;
        me->buffer_condition.notify_all();
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
