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

#include "connection_proxy.h"
#include <cassert>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <streambuf>
#include <thread>
#include <vector>

static const size_t block_size = 1048576;
static const size_t block_count = 16;

namespace pupnp {

class connection_proxy::streambuf : public std::streambuf
{
friend class source;
public:
    streambuf(class connection_proxy &);

    int underflow() override;
    pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode) override;
    pos_type seekpos(pos_type, std::ios_base::openmode) override;

private:
    class connection_proxy &parent;

    size_t buffer_offset;
    size_t buffer_available;
};

class connection_proxy::source
{
public:
    source(std::unique_ptr<std::istream> &&input, bool buffer_all);
    ~source();

    bool attach(class streambuf &);
    void detach(class streambuf &);

    bool read(class streambuf &);
    bool seek(class streambuf &, size_t);
    size_t size();

    typedef std::vector<std::pair<platform::messageloop_ref *, std::function<void()>>> multicast_event;
    multicast_event on_close;
    multicast_event on_detach;

private:
    void consume();
    void recompute_buffer_offset(std::unique_lock<std::mutex> &);

private:
    const std::unique_ptr<std::istream> input;
    const bool buffer_all;

    std::unique_ptr<std::thread> consume_thread;
    std::mutex mutex;

    bool stream_end;
    std::set<class streambuf *> streambufs;

    std::vector<char> buffer;
    std::condition_variable buffer_condition;
    size_t buffer_offset;
    size_t buffer_used;

};

connection_proxy::connection_proxy()
    : std::istream(new class streambuf(*this)),
      source(nullptr)
{
}

connection_proxy::connection_proxy(std::unique_ptr<std::istream> &&input, bool buffer_all)
    : std::istream(new class streambuf(*this)),
      source(new class source(std::move(input), buffer_all))
{
    source->attach(static_cast<class streambuf &>(*std::istream::rdbuf()));
}

connection_proxy::~connection_proxy()
{
    if (source)
    {
        source->detach(static_cast<class streambuf &>(*std::istream::rdbuf()));
        source = nullptr;
    }

    delete std::istream::rdbuf(nullptr);
}

bool connection_proxy::attach(connection_proxy &parent)
{
    if (parent.source->attach(static_cast<class streambuf &>(*std::istream::rdbuf())))
    {
        source = parent.source;
        return true;
    }

    return false;
}

void connection_proxy::subscribe_close(platform::messageloop_ref &messageloop_ref, const std::function<void()> &func)
{
    source->on_close.emplace_back(std::make_pair(&messageloop_ref, func));
}

void connection_proxy::subscribe_detach(platform::messageloop_ref &messageloop_ref, const std::function<void()> &func)
{
    source->on_detach.emplace_back(std::make_pair(&messageloop_ref, func));
}


connection_proxy::source::source(std::unique_ptr<std::istream> &&input, bool buffer_all)
    : input(std::move(input)),
      buffer_all(buffer_all),
      stream_end(false),
      buffer_offset(0),
      buffer_used(0)
{
    buffer.resize(block_size * block_count);

    consume_thread.reset(new std::thread(std::bind(&connection_proxy::source::consume, this)));
}

connection_proxy::source::~source()
{
    {
        std::lock_guard<std::mutex> _(mutex);

        stream_end = true;
        buffer_condition.notify_all();
    }

    consume_thread->join();

    for (auto &i : on_close) i.first->post(i.second);
}

bool connection_proxy::source::attach(class streambuf &streambuf)
{
    std::lock_guard<std::mutex> _(mutex);

    if (buffer_offset == 0)
    {
        streambufs.insert(&streambuf);
        return true;
    }

    return false;
}

void connection_proxy::source::detach(class streambuf &streambuf)
{
    std::unique_lock<std::mutex> l(mutex);

    streambufs.erase(&streambuf);
    recompute_buffer_offset(l);
}

void connection_proxy::source::consume()
{
    std::unique_lock<std::mutex> l(mutex);

    while (!stream_end && *input)
    {
        // Wait for enough space to write a block.
        char * write_block = nullptr;
        size_t write_block_size = 0;
        do
        {
            if ((buffer_used + block_size) <= buffer.size())
            {
                const size_t write_block_pos = (buffer_offset + buffer_used) % buffer.size();
                write_block = &buffer[write_block_pos];
                write_block_size = std::min(block_size, buffer.size() - write_block_pos);
            }
            else if (buffer_all)
                buffer.resize(buffer.size() + block_size);
            else
                buffer_condition.wait(l);
        } while (!stream_end && !write_block);

        if (!stream_end)
        {
            l.unlock();
            assert(write_block_size > 0);
            input->read(write_block, write_block_size);
            const size_t read = input->gcount();
            l.lock();

            buffer_used += read;
            buffer_condition.notify_all();
        }
    }

    stream_end = true;
    buffer_condition.notify_all();
}

bool connection_proxy::source::read(class streambuf &streambuf)
{
    std::unique_lock<std::mutex> l(mutex);

    streambuf.buffer_offset += streambuf.buffer_available;
    streambuf.buffer_available = 0;
    recompute_buffer_offset(l);

    while (!stream_end && ((buffer_offset + buffer_used) <= streambuf.buffer_offset))
        buffer_condition.wait(l);

    if ((buffer_offset + buffer_used) > streambuf.buffer_offset)
    {
        const size_t bpos = streambuf.buffer_offset % buffer.size();
        const size_t size = std::min(
                    std::min(
                        buffer.size() - bpos,
                        (buffer_offset + buffer_used) - streambuf.buffer_offset),
                    block_size);

        streambuf.setg(&buffer[bpos], &buffer[bpos], &buffer[bpos] + size);
        streambuf.buffer_available = size;

        return true;
    }

    for (auto &i : on_detach) i.first->post(i.second);
    on_detach.clear();

    return false;
}

bool connection_proxy::source::seek(class streambuf &streambuf, size_t pos)
{
    std::unique_lock<std::mutex> l(mutex);

    const size_t apos = pos & ~(block_size - 1);
    if ((apos >= buffer_offset) && (pos <= buffer_offset + buffer_used))
    {
        streambuf.buffer_offset = apos;
        streambuf.buffer_available = 0;

        const size_t bpos = apos % buffer.size();
        const size_t size = std::min(
                    std::min(
                        buffer.size() - bpos,
                        (buffer_offset + buffer_used) - streambuf.buffer_offset),
                    block_size);

        streambuf.setg(&buffer[bpos], &buffer[bpos] + pos - apos, &buffer[bpos] + size);
        streambuf.buffer_available = size;

        return true;
    }

    return false;
}

size_t connection_proxy::source::size()
{
    if (buffer_all)
    {
        std::unique_lock<std::mutex> l(mutex);
        while (!stream_end)
            buffer_condition.wait(l);

        return buffer_offset + buffer_used;
    }

    return 0;
}

void connection_proxy::source::recompute_buffer_offset(std::unique_lock<std::mutex> &)
{
    if (!buffer_all)
    {
        size_t new_offset = size_t(-1);
        for (auto &i : streambufs)
            new_offset = std::min(new_offset, i->buffer_offset);

        if ((new_offset != size_t(-1)) && (new_offset >= (buffer_offset + block_size)) &&
            ((buffer_offset > 0) || (new_offset >= (buffer.size() * 3 / 4))))
        {
            if (buffer_offset == 0)
            {
                for (auto &i : on_detach) i.first->post(i.second);
                on_detach.clear();
            }

            const size_t proceed = (new_offset - buffer_offset) & ~(block_size - 1);
            buffer_offset += proceed;
            buffer_used -= proceed;
            buffer_condition.notify_all();
        }
    }
}


connection_proxy::streambuf::streambuf(class connection_proxy &parent)
    : parent(parent),
      buffer_offset(0),
      buffer_available(0)
{
}

int connection_proxy::streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*gptr());
    else if (parent.source->read(*this))
        return traits_type::to_int_type(*gptr());
    else
        return traits_type::eof();
}

std::streambuf::pos_type connection_proxy::streambuf::seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which)
{
    switch (dir)
    {
    case std::ios_base::beg:
        return seekpos(pos_type(off), which);

    case std::ios_base::cur:
        return seekpos(buffer_offset + (gptr() - eback()) + off, which);

    case std::ios_base::end:
        if (parent.source->size())
            return seekpos(parent.source->size() + off, which);

        break;

    default:
        break;
    }

    return pos_type(off_type(-1));
}

std::streambuf::pos_type connection_proxy::streambuf::seekpos(pos_type pos, std::ios_base::openmode)
{
    if (parent.source->seek(*this, pos))
        return pos;

    return pos_type(off_type(-1));
}

} // End of namespace
