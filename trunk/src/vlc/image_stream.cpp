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

#include "image_stream.h"
#include "media.h"
#include "instance.h"
#include "platform/fstream.h"
#include "platform/path.h"
#include "platform/uuid.h"
#include "png/image.h"
#include <vlc/vlc.h>
#include <condition_variable>
#include <mutex>

namespace vlc {

image_stream::image_stream(class instance &instance)
    : std::istream(nullptr),
      instance(instance)
{
}

image_stream::~image_stream()
{
    close();
}

bool image_stream::open(
        const std::string &mrl,
        unsigned width, unsigned height)
{
    close();

    bool result = false;

    auto media = media::from_mrl(instance, mrl);
    auto player = libvlc_media_player_new_from_media(media);
    if (player)
    {
        struct T
        {
            static void callback(const libvlc_event_t *e, void *opaque)
            {
                T * const t = reinterpret_cast<T *>(opaque);
                std::lock_guard<std::mutex> _(t->mutex);

                if (e->type == libvlc_MediaPlayerEncounteredError)
                    t->encountered_error = true;

                t->condition.notify_one();
            }

            static void play(void */*opaque*/, const void */*samples*/, unsigned /*count*/, int64_t /*pts*/)
            {
            }

            static void * lock(void *opaque, void **planes)
            {
                T * const t = reinterpret_cast<T *>(opaque);
                return *planes = t->pixel_buffer.scan_line(0);
            }

            static void display(void *opaque, void *)
            {
                T * const t = reinterpret_cast<T *>(opaque);
                t->displayed = true;
                t->condition.notify_one();
            }

            std::mutex mutex;
            std::condition_variable condition;
            bool displayed;
            bool encountered_error;
            png::image pixel_buffer;
        } t;

        std::unique_lock<std::mutex> l(t.mutex);

        t.displayed = false;
        t.encountered_error = false;
        t.pixel_buffer = png::image(width, height);

        libvlc_media_player_set_rate(player, 10.0f);

        libvlc_audio_set_callbacks(player, &T::play, nullptr, nullptr, nullptr, nullptr, &t);
        libvlc_audio_set_format(player, "S16N", 44100, 2);
        libvlc_video_set_callbacks(player, &T::lock, nullptr, &T::display, &t);
        libvlc_video_set_format(player, "RV32", width, height, width * sizeof(uint32_t));

        auto event_manager = libvlc_media_player_event_manager(player);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerEncounteredError, &T::callback, &t);
        libvlc_event_attach(event_manager, libvlc_MediaPlayerPlaying, &T::callback, &t);

        libvlc_media_player_play(player);

        while (!t.displayed && !t.encountered_error)
            t.condition.wait(l);

        l.unlock();
        libvlc_media_player_stop(player);

        libvlc_event_detach(event_manager, libvlc_MediaPlayerPlaying, &T::callback, &t);
        libvlc_event_detach(event_manager, libvlc_MediaPlayerEncounteredError, &T::callback, &t);

        libvlc_media_player_release(player);

        if (t.displayed && !t.encountered_error)
        {
            stream.reset(new std::stringstream());
            if (t.pixel_buffer.save(*stream))
            {
                std::istream::rdbuf(stream->rdbuf());
                result = true;
            }
            else
                stream = nullptr;
        }
    }

    return result;
}

void image_stream::close()
{
    std::istream::rdbuf(nullptr);
    stream = nullptr;
}

} // End of namespace
