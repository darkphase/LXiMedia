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
#include "instance.h"
#include <vlc/vlc.h>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <sstream>
#include <streambuf>
#include <thread>
#include <vector>

namespace vlc {

struct transcode_stream::data : std::streambuf
{
  static void write(data *, const uint8_t *, size_t);
  int underflow() override;

  std::string broadcast_name;
  bool stream_end;

  static const size_t block_size;
  std::vector<char> buffer;
  std::mutex buffer_mutex;
  std::condition_variable buffer_condition;
  size_t buffer_pos;
  size_t buffer_used;
};

const size_t transcode_stream::data::block_size = 65536;

transcode_stream::transcode_stream(class instance &instance)
  : instance(instance),
    d(new data())
{
  d->stream_end = false;
  d->buffer_pos = 0;
  d->buffer_used = 0;
}

transcode_stream::~transcode_stream()
{
  close();
}

bool transcode_stream::open(const std::string &mrl)
{
  close();

  static const char transcode[] = "#transcode{vcodec=mp2v,vb=4096,acodec=mpga,ab=256}";
  std::ostringstream sout;
  sout
      << transcode
      << ":standard{access=lximedia_memout{callback=" << intptr_t(&transcode_stream::data::write)
      << ",opaque=" << intptr_t(d.get())
      << "},mux=ps}";

  {
    std::lock_guard<std::mutex> _(d->buffer_mutex);

    d->broadcast_name = std::to_string(intptr_t(this));
    if (libvlc_vlm_add_broadcast(
          instance,
          d->broadcast_name.c_str(),
          mrl.c_str(),
          sout.str().c_str(),
          0, NULL, 1, 0) == 0)
    {
      if (libvlc_vlm_play_media(instance, d->broadcast_name.c_str()) == 0)
      {
        d->stream_end = false;
        d->buffer.resize(d->block_size * 64);
        d->buffer_pos = 0;
        d->buffer_used = 0;

        std::istream::rdbuf(d.get());
        return true;
      }
    }

    d->broadcast_name.clear();
  }

  return false;
}

void transcode_stream::close()
{
  std::unique_lock<std::mutex> l(d->buffer_mutex);

  if (!d->broadcast_name.empty())
  {
    d->stream_end = true;
    d->buffer_condition.notify_one();
    l.unlock();
      libvlc_vlm_stop_media(instance, d->broadcast_name.c_str());
      libvlc_vlm_del_media(instance, d->broadcast_name.c_str());
    l.lock();
    d->broadcast_name.clear();
  }
}

void transcode_stream::data::write(data *d, const uint8_t *block, size_t size)
{
  std::unique_lock<std::mutex> l(d->buffer_mutex);

  if ((block == nullptr) || (size == 0))
  {
    d->stream_end = true;
    d->buffer_condition.notify_one();
  }
  else while (!d->stream_end && (size > 0))
  {
    if (d->buffer.size() > d->buffer_used)
    {
      const size_t pos = (d->buffer_pos + d->buffer_used) % d->buffer.size();
      const size_t wrt = std::min(d->buffer.size() - pos, size);

      memcpy(&(d->buffer[pos]), block, wrt);
      d->buffer_used += wrt;
      block += wrt;
      size -= wrt;

      d->buffer_condition.notify_one();
    }
    else
      d->buffer_condition.wait(l);
  }
}

int transcode_stream::data::underflow()
{
  if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
    return traits_type::to_int_type(*gptr());

  std::unique_lock<std::mutex> l(buffer_mutex);

  const size_t read = (gptr() != nullptr) ? std::min(buffer_used, block_size) : 0;
  buffer_pos = (buffer_pos + read) % buffer.size();
  buffer_used -= read;

  buffer_condition.notify_one();
  while (!stream_end && (buffer_used < block_size))
    buffer_condition.wait(l);

  if (buffer_used > 0)
  {
    const size_t size = std::min(buffer_used, block_size);
    setg(&buffer[buffer_pos], &buffer[buffer_pos], &buffer[buffer_pos + size]);
    return traits_type::to_int_type(*gptr());
  }

  return traits_type::eof();
}


} // End of namespace
