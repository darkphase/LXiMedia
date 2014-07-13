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

#include "media.h"
#include "instance.h"
#include <vlc/vlc.h>
#include <condition_variable>
#include <mutex>

namespace vlc {

media::media(class instance &instance, const std::string &path)
  : libvlc_media(libvlc_media_new_path(instance, path.c_str()))
{
}

media::media(const media &from)
  : libvlc_media(from.libvlc_media)
{
  libvlc_media_retain(libvlc_media);
}

media::~media()
{
  libvlc_media_release(libvlc_media);
}

media & media::operator=(const media &from)
{
  libvlc_media_retain(from.libvlc_media);
  libvlc_media_release(libvlc_media);
  libvlc_media = from.libvlc_media;

  return *this;
}

void media::parse() const
{
  if (!libvlc_media_is_parsed(libvlc_media))
  {
    libvlc_media_parse(libvlc_media);

    libvlc_media_track_t **track_list = nullptr;
    const unsigned count = libvlc_media_tracks_get(libvlc_media, &track_list);
    if (track_list)
      libvlc_media_tracks_release(track_list, count);

    // Try to play a bit if libvlc_media_parse() failed.
    if (count == 0)
    {
      auto player = libvlc_media_player_new_from_media(libvlc_media);
      if (player)
      {
        static const int width = 256, height = 256, align = 32;

        struct T
        {
          static void callback(const libvlc_event_t *, void *opaque)
          {
            T * const t = reinterpret_cast<T *>(opaque);
            std::lock_guard<std::mutex> _(t->mutex);
            t->position_changed = true;
            t->condition.notify_one();
          }

          static void play(void */*opaque*/, const void */*samples*/, unsigned /*count*/, int64_t /*pts*/)
          {
          }

          static void * lock(void *opaque, void **planes)
          {
            T * const t = reinterpret_cast<T *>(opaque);
            *planes = (void *)((uintptr_t(&t->pixel_buffer[0]) + (align - 1)) & ~uintptr_t(align - 1));
          }

          std::condition_variable condition;
          std::mutex mutex;
          bool position_changed;
          std::vector<uint8_t> pixel_buffer;
        } t;

        t.position_changed = false;
        t.pixel_buffer.resize((width * height * sizeof(uint32_t)) + align);

        libvlc_audio_set_callbacks(player, &T::play, nullptr, nullptr, nullptr, nullptr, &t);
        libvlc_audio_set_format(player, "S16N", 44100, 2);
        libvlc_video_set_callbacks(player, &T::lock, nullptr, nullptr, &t);
        libvlc_video_set_format(player, "RV32", width, height, width * sizeof(uint32_t));

        if (libvlc_media_player_play(player) == 0)
        {
          auto event_manager = libvlc_media_player_event_manager(player);
          libvlc_event_attach(event_manager, libvlc_MediaPlayerPositionChanged, T::callback, &t);

          {
            std::unique_lock<std::mutex> l(t.mutex);
            libvlc_media_player_set_position(player, 0.05f);
            while (!t.position_changed) t.condition.wait_for(l, std::chrono::seconds(1));
          }

          libvlc_event_detach(event_manager, libvlc_MediaPlayerPositionChanged, T::callback, &t);
          libvlc_media_player_stop(player);
        }

        libvlc_media_player_release(player);
      }
    }
  }
}

std::string media::mrl() const
{
  return libvlc_media_get_mrl(libvlc_media);
}

std::vector<media::track> media::tracks() const
{
  std::vector<track> result;

  parse();

  libvlc_media_track_t **track_list = nullptr;
  const unsigned count = libvlc_media_tracks_get(libvlc_media, &track_list);
  if (track_list)
  {
    for (unsigned i = 0; (i < count) && track_list[i]; i++)
    {
      struct track track;
      track.type = track_type::unknown;

      switch (track_list[i]->i_type)
      {
      case libvlc_track_unknown:  track.type = track_type::unknown; break;
      case libvlc_track_audio  :  track.type = track_type::audio  ; break;
      case libvlc_track_video  :  track.type = track_type::video  ; break;
      case libvlc_track_text   :  track.type = track_type::text   ; break;
      }

      result.emplace_back(track);
    }

    libvlc_media_tracks_release(track_list, count);
  }

  return result;
}

} // End of namespace
