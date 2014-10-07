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
#include "platform/messageloop.h"
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

const size_t transcode_stream::block_size = 65536;
const size_t transcode_stream::block_count = 256;

class transcode_stream::streambuf : public std::streambuf
{
    friend class transcode_stream::source;
public:
    streambuf(transcode_stream &);

    int underflow() override;

private:
    transcode_stream &parent;

    size_t buffer_offset;
    size_t buffer_available;
};

class transcode_stream::source
{
public:
    source(
            class platform::messageloop &,
            class instance &,
            const std::function<int32_t(int32_t)> &,
            const std::string &,
            int,
            const struct track_ids &,
            const std::string &,
            const std::string &,
            float,
            const std::vector<std::string> &);

    source(
            class platform::messageloop &,
            class instance &,
            const std::function<int32_t(int32_t)> &,
            const std::string &,
            std::chrono::milliseconds,
            const struct track_ids &,
            const std::string &,
            const std::string &,
            float,
            const std::vector<std::string> &);

    ~source();

    bool is_open() const { return player != nullptr; }

    bool attach(class streambuf &);
    void detach(class streambuf &);

    bool read(class streambuf &);

private:
    source(
            class platform::messageloop &,
            class instance &,
            const std::function<int32_t(int32_t)> &,
            const std::string &,
            const struct track_ids &,
            const std::string &,
            const std::string &,
            float,
            const std::vector<std::string> &);

    size_t m2ts_filter(int, void *, size_t);
    void consume();

    void recompute_buffer_offset(std::unique_lock<std::mutex> &);
    static void callback(const libvlc_event_t *, void *);

private:
    class platform::messageloop &messageloop;
    class instance &instance;
    const std::function<int32_t(int32_t)> changed;
    const struct track_ids track_ids;
    class media media;
    libvlc_media_player_t *player;
    libvlc_event_manager_t *event_manager;
    int chapter;
    std::chrono::milliseconds position;
    const float rate;

    int32_t id;

    std::mutex mutex;
    bool stream_start_pending;
    bool stream_end, stream_end_pending;
    std::set<class streambuf *> streambufs;

    std::vector<char> buffer;
    std::condition_variable buffer_condition;
    size_t buffer_offset;
    size_t buffer_used;

    int pipe[2];
    std::unique_ptr<std::thread> consume_thread;
    char *write_block;
    size_t write_block_pos;

    const bool m2ts_filter_enabled;
    std::vector<char> m2ts_filter_buffer;
    size_t m2ts_filter_buffer_pos;
};

transcode_stream::transcode_stream(
        class platform::messageloop &messageloop,
        class instance &instance,
        const std::function<int32_t(int32_t)> &changed)
    : messageloop(messageloop),
      instance(instance),
      changed(changed),
      streambuf(new class streambuf(*this))
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
    source = std::make_shared<class source>(messageloop, instance, changed, mrl, chapter, track_ids, transcode, mux, rate, options);
    if (source->is_open())
    {
        source->attach(*streambuf);
        std::istream::rdbuf(streambuf.get());

        return true;
    }

    source = nullptr;
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
    source = std::make_shared<class source>(messageloop, instance, changed, mrl, position, track_ids, transcode, mux, rate, options);
    if (source->is_open())
    {
        source->attach(*streambuf);
        std::istream::rdbuf(streambuf.get());

        return true;
    }

    source = nullptr;
    return false;
}

bool transcode_stream::attach(transcode_stream &parent)
{
    if (parent.source->attach(*streambuf))
    {
        source = parent.source;
        std::istream::rdbuf(streambuf.get());

        return true;
    }

    return false;
}


void transcode_stream::close()
{
    if (source)
    {
        source->detach(*streambuf);
        source = nullptr;
    }
}


transcode_stream::source::source(
        class platform::messageloop &messageloop,
        class instance &instance,
        const std::function<int32_t(int32_t)> &changed,
        const std::string &mrl,
        const struct track_ids &track_ids,
        const std::string &transcode,
        const std::string &mux,
        float rate,
        const std::vector<std::string> &options)
    : messageloop(messageloop),
      instance(instance),
      changed(changed),
      track_ids(track_ids),
      media(media::from_mrl(instance, mrl)),
      player(nullptr),
      event_manager(nullptr),
      chapter(-1),
      position(0),
      rate(rate),
      id(0),
      stream_start_pending(true),
      stream_end(false),
      stream_end_pending(false),
      buffer_offset(0),
      buffer_used(0),
      write_block(nullptr),
      write_block_pos(0),
      m2ts_filter_enabled(mux == "m2ts"),
      m2ts_filter_buffer_pos(0)
{
    pipe_create(pipe);

    const std::string vlc_mux = m2ts_filter_enabled ? "ts" : mux;

    std::ostringstream sout;
    sout
            << ":sout=" << transcode
            << ":std{access=fd,mux=" << vlc_mux << ",dst=" << pipe[1] << "}";

    std::lock_guard<std::mutex> _(mutex);

    libvlc_media_add_option(media, sout.str().c_str());
    for (auto &option : options)
        libvlc_media_add_option(media, option.c_str());

    player = libvlc_media_player_new_from_media(media);
    if (player)
    {
        buffer.resize(block_size * block_count);
        write_block = &buffer[0];

        consume_thread.reset(new std::thread(std::bind(&transcode_stream::source::consume, this)));

        event_manager = libvlc_media_player_event_manager(player);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerPlaying, &source::callback, this);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEndReached, &source::callback, this);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEncounteredError, &source::callback, this);

        std::clog << '[' << this << "] " << sout << std::endl;
        if (changed) id = changed(id);
    }
}

transcode_stream::source::source(
        class platform::messageloop &messageloop,
        class instance &instance,
        const std::function<int32_t(int32_t)> &changed,
        const std::string &mrl,
        int chapter,
        const struct track_ids &track_ids,
        const std::string &transcode,
        const std::string &mux,
        float rate,
        const std::vector<std::string> &options)
      : source(messageloop, instance, changed, mrl, track_ids, transcode, mux, rate, options)
{
    this->chapter = chapter;

    if (player)
    {
        libvlc_media_player_play(player);

        std::clog << '[' << this << "] opened transcode_stream " << mrl << "@Chapter " << chapter << std::endl;
    }
}

transcode_stream::source::source(
        class platform::messageloop &messageloop,
        class instance &instance,
        const std::function<int32_t(int32_t)> &changed,
        const std::string &mrl,
        std::chrono::milliseconds position,
        const struct track_ids &track_ids,
        const std::string &transcode,
        const std::string &mux,
        float rate,
        const std::vector<std::string> &options)
      : source(messageloop, instance, changed, mrl, track_ids, transcode, mux, rate, options)
{
    this->position = position;

    if (player)
    {
        libvlc_media_player_play(player);

        std::clog << '[' << this << "] opened transcode_stream " << mrl << '@' << position.count() << std::endl;
    }
}

transcode_stream::source::~source()
{
    std::unique_lock<std::mutex> l(mutex);

    if (player)
    {
        stream_end = true;
        buffer_condition.notify_all();
        l.unlock();

        libvlc_event_detach(event_manager, libvlc_MediaPlayerEncounteredError, &source::callback, this);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEndReached, &source::callback, this);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerPlaying, &source::callback, this);
        libvlc_media_player_stop(player);
        libvlc_media_player_release(player);

        l.lock();
        pipe_close(pipe[1]);
        l.unlock();

        consume_thread->join();

        l.lock();
        pipe_close(pipe[0]);
    }

    std::clog << '[' << this << "] destroyed transcode_stream " << media.mrl() << std::endl;
    if (changed) id = changed(id);
}

bool transcode_stream::source::attach(class streambuf &streambuf)
{
    std::lock_guard<std::mutex> _(mutex);

    if (buffer_offset == 0)
    {
        std::clog << '[' << this << "] attached transcode_stream " << media.mrl() << std::endl;

        streambufs.insert(&streambuf);
        return true;
    }

    return false;
}

void transcode_stream::source::detach(class streambuf &streambuf)
{
    std::unique_lock<std::mutex> l(mutex);

    streambufs.erase(&streambuf);
    recompute_buffer_offset(l);

    std::clog << '[' << this << "] detached transcode_stream " << media.mrl() << std::endl;
}

size_t transcode_stream::source::m2ts_filter(int filedes, void *buf, size_t nbyte)
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

void transcode_stream::source::consume()
{
    for (;;)
    {
        const size_t chunk = m2ts_filter_enabled
                ? m2ts_filter(pipe[0], &write_block[write_block_pos], block_size - write_block_pos)
                : pipe_read(pipe[0], &write_block[write_block_pos], block_size - write_block_pos);

        if ((chunk > 0) && (chunk != size_t(-1)))
        {
            write_block_pos += chunk;
            if (write_block_pos >= block_size)
            {
                std::unique_lock<std::mutex> l(mutex);

                if (!stream_end)
                {
                    assert(write_block_pos == block_size);
                    buffer_used += block_size;
                    buffer_condition.notify_all();

                    do
                    {
                        assert((buffer_used & (block_size - 1)) == 0);
                        if ((buffer_used + block_size) <= buffer.size())
                        {
                            assert((buffer_offset & (block_size - 1)) == 0);
                            write_block = &buffer[(buffer_offset + buffer_used) % buffer.size()];
                            write_block_pos = 0;
                        }
                        else
                            buffer_condition.wait(l);
                    } while (!stream_end && (write_block_pos > 0));
                }
                else
                    write_block_pos = 0;
            }
        }
        else
            break;
    }

    // Flush pipe to prevent deadlock while stopping player.
    {
        std::unique_lock<std::mutex> l(mutex);

        char buffer[32];
        while (pipe[1])
        {
            l.unlock();
            pipe_read(pipe[0], buffer, sizeof(buffer));
            l.lock();
        }
    }
}

bool transcode_stream::source::read(class streambuf &streambuf)
{
    std::unique_lock<std::mutex> l(mutex);

    streambuf.buffer_offset += streambuf.buffer_available;
    streambuf.buffer_available = 0;
    recompute_buffer_offset(l);

    while (!stream_end &&
           (((streambuf.buffer_offset == 0) && (buffer_used < (block_size * 8))) ||
            ((buffer_offset + buffer_used) <= streambuf.buffer_offset)))
    {
        if (stream_end_pending)
        {
            buffer_used += write_block_pos;
            write_block_pos = 0;
            stream_end = true;
        }
        else
            buffer_condition.wait(l);
    }

    if ((buffer_offset + buffer_used) > streambuf.buffer_offset)
    {
        const size_t pos = streambuf.buffer_offset % buffer.size();
        const size_t size = std::min(
                    std::min(
                        buffer.size() - pos,
                        (buffer_offset + buffer_used) - streambuf.buffer_offset),
                    block_size);

        streambuf.setg(&buffer[pos], &buffer[pos], &buffer[pos + size]);
        streambuf.buffer_available = size;

        return true;
    }

    return false;
}

void transcode_stream::source::recompute_buffer_offset(std::unique_lock<std::mutex> &)
{
    size_t new_offset = size_t(-1);
    for (auto &i : streambufs)
        new_offset = std::min(new_offset, i->buffer_offset);

    if ((new_offset != size_t(-1)) && (new_offset >= (buffer_offset + block_size)) &&
            ((buffer_offset > 0) || (new_offset >= (buffer.size() * 3 / 4))))
    {
        const size_t proceed = (new_offset - buffer_offset) & ~(block_size - 1);
        buffer_offset += proceed;
        buffer_used -= proceed;
        buffer_condition.notify_all();
    }
}

void transcode_stream::source::callback(const libvlc_event_t *e, void *opaque)
{
    source * const me = reinterpret_cast<source *>(opaque);

    if ((e->type == libvlc_MediaPlayerEndReached) ||
        (e->type == libvlc_MediaPlayerEncounteredError))
    {
        std::lock_guard<std::mutex> _(me->mutex);

        me->stream_end_pending = true;
        me->buffer_condition.notify_all();
    }
    else if (e->type == libvlc_MediaPlayerPlaying)
    {
        if (me->stream_start_pending)
        {
            me->stream_start_pending = false;
            me->messageloop.post([me]
            {
                if (me->track_ids.video >= 0)   libvlc_video_set_track(me->player, me->track_ids.video);
                if (me->track_ids.audio >= 0)   libvlc_audio_set_track(me->player, me->track_ids.audio);
                libvlc_video_set_spu(me->player, me->track_ids.text);

                if (me->chapter >= 0)           libvlc_media_player_set_chapter(me->player, me->chapter);
                if (me->position.count() > 0)   libvlc_media_player_set_time(me->player, me->position.count());

                if (std::abs(me->rate - 1.0f) > 0.01f) libvlc_media_player_set_rate(me->player, me->rate);
            });
        }
    }
}


transcode_stream::streambuf::streambuf(transcode_stream &parent)
    : parent(parent),
      buffer_offset(0),
      buffer_available(0)
{
}

int transcode_stream::streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*gptr());
    else if (parent.source->read(*this))
        return traits_type::to_int_type(*gptr());
    else
        return traits_type::eof();
}

} // End of namespace

#if defined(__unix__)
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
