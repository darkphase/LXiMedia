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
#include "../messageloop.h"
#include "instance.h"
#include <vlc/vlc.h>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <streambuf>
#include <thread>
#include <vector>

namespace vlc {

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
  source(class instance &, const std::string &, const std::string &, const std::string &);
  ~source();

  bool is_open() const { return !broadcast_name.empty(); }

  bool attach(class streambuf &);
  void detach(class streambuf &);

  static void write(source *, const uint8_t *, size_t);
  bool read(class streambuf &);

private:
  class instance &instance;

  std::mutex mutex;

  std::string broadcast_name;
  bool stream_end;

  std::set<class streambuf *> streambufs;

  std::vector<char> buffer;
  std::condition_variable buffer_condition;
  size_t buffer_offset;
  size_t buffer_used;
  static const size_t preload_size;
  size_t last_buffer_pos;
};

const size_t transcode_stream::block_size = 65536;
const size_t transcode_stream::block_count = 256;
const size_t transcode_stream::source::preload_size = block_size * block_count / 4;

transcode_stream::transcode_stream(class messageloop &messageloop, class instance &instance)
  : messageloop(messageloop),
    instance(instance),
    streambuf(new class streambuf(*this))
{
}

transcode_stream::~transcode_stream()
{
  close();
}

bool transcode_stream::open(const std::string &mrl, const std::string &transcode, const std::string &mux)
{
  source = std::make_shared<class source>(instance, mrl, transcode, mux);
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
    class instance &instance,
    const std::string &mrl,
    const std::string &transcode,
    const std::string &mux)
  : instance(instance),
    stream_end(false),
    buffer_offset(0),
    buffer_used(0),
    last_buffer_pos(preload_size - block_size)
{
  std::ostringstream sout;
  sout
      << transcode
      << ":std{access=lximedia_memout{callback=" << intptr_t(&source::write)
      << ",opaque=" << intptr_t(this)
      << "},mux=" << mux << "}";

  {
    std::lock_guard<std::mutex> _(mutex);

    const auto name = std::to_string(intptr_t(this));
    if (libvlc_vlm_add_broadcast(
          instance,
          name.c_str(),
          mrl.c_str(),
          sout.str().c_str(),
          0, NULL, 1, 0) == 0)
    {
      if (libvlc_vlm_play_media(instance, name.c_str()) == 0)
      {
        broadcast_name = name;
        buffer.resize(block_size * block_count);
      }
    }
  }
}

transcode_stream::source::~source()
{
  std::unique_lock<std::mutex> l(mutex);

  if (!broadcast_name.empty())
  {
    stream_end = true;
    buffer_condition.notify_one();
    l.unlock();
      libvlc_vlm_stop_media(instance, broadcast_name.c_str());
      libvlc_vlm_del_media(instance, broadcast_name.c_str());
    l.lock();
    broadcast_name.clear();
  }
}

bool transcode_stream::source::attach(class streambuf &streambuf)
{
  std::lock_guard<std::mutex> _(mutex);

  if (buffer_offset == 0)
  {
    streambufs.insert(&streambuf);
    return true;
  }

  return false;
}

void transcode_stream::source::detach(class streambuf &streambuf)
{
  std::lock_guard<std::mutex> _(mutex);

  streambufs.erase(&streambuf);
}

void transcode_stream::source::write(source *me, const uint8_t *block, size_t size)
{
  std::unique_lock<std::mutex> l(me->mutex);

  if ((block == nullptr) || (size == 0))
  {
    me->stream_end = true;
    me->buffer_condition.notify_one();
  }
  else while (!me->stream_end && (size > 0))
  {
    if (me->buffer.size() > me->buffer_used)
    {
      const size_t pos = (me->buffer_offset + me->buffer_used) % me->buffer.size();
      const size_t wrt = std::min(me->buffer.size() - pos, size);

      memcpy(&(me->buffer[pos]), block, wrt);
      me->buffer_used += wrt;
      block += wrt;
      size -= wrt;

      if ((me->buffer_offset + me->buffer_used) >= (me->last_buffer_pos + block_size))
      {
        me->last_buffer_pos = ((me->buffer_offset + me->buffer_used) + (block_size - 1)) & ~(block_size - 1);
        me->buffer_condition.notify_all();
      }
    }
    else
      me->buffer_condition.wait(l);
  }
}

bool transcode_stream::source::read(class streambuf &streambuf)
{
  std::unique_lock<std::mutex> l(mutex);

  streambuf.buffer_offset += streambuf.buffer_available;
  streambuf.buffer_available = 0;

  size_t new_offset = 0;
  for (auto &i : streambufs)
    new_offset = std::max(new_offset, i->buffer_offset);

  if ((new_offset > buffer_offset) &&
      ((buffer_offset > 0) || (new_offset > (buffer.size() * 3 / 4))))
  {
    const size_t proceed = new_offset - buffer_offset;
    buffer_offset += proceed;
    buffer_used -= proceed;
    buffer_condition.notify_one();
  }

  while (!stream_end &&
         (((buffer_offset + buffer_used) < preload_size) ||
          ((buffer_offset + buffer_used) < (streambuf.buffer_offset + block_size))))
  {
    buffer_condition.wait(l);
  }

  if ((buffer_offset + buffer_used) > streambuf.buffer_offset)
  {
    const size_t offset = streambuf.buffer_offset % buffer.size();
    const size_t size = std::min((buffer_offset + buffer_used) - streambuf.buffer_offset, block_size);
    streambuf.setg(&buffer[offset], &buffer[offset], &buffer[offset + size]);
    streambuf.buffer_available = size;
    return true;
  }

  return false;
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
