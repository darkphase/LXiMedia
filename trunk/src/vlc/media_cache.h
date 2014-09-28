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

#ifndef VLC_MEDIA_CACHE_H
#define VLC_MEDIA_CACHE_H

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

struct libvlc_media_t;

namespace platform { class messageloop; }

namespace vlc {

class instance;
class media;

class media_cache
{
public:
    enum class track_type { unknown, audio, video, text };

    struct track
    {
        int id;
        std::string language;
        std::string description;

        track_type type;
        union
        {
            struct { unsigned sample_rate, channels; } audio;
            struct { unsigned width, height; float frame_rate; } video;
        };
    };

public:
    explicit media_cache(class platform::messageloop &);
    ~media_cache();

    void async_parse_items(const std::vector<class media> &items);
    void wait();
    template< class Rep, class Period >
    std::cv_status wait_for(const std::chrono::duration<Rep, Period>& rel_time);
    void abort();

    bool has_data(const class media &);

    const std::vector<track> & tracks(class media &);
    std::chrono::milliseconds duration(class media &);
    int chapter_count(class media &);

    std::function<void()> on_finished;

private:
    struct parsed_data;
    const struct parsed_data &read_parsed_data(class media &media);
    void worker_thread();
    void finish();

private:
    class platform::messageloop &messageloop;

    std::mutex mutex;
    std::condition_variable condition;
    std::map<std::string, std::unique_ptr<parsed_data>> cache;
    int pending_items;

    std::map<std::thread::id, std::unique_ptr<std::thread>> thread_pool;
    std::vector<std::unique_ptr<std::thread>> thread_dump;
    std::queue<class media> work_list;
};

template< class Rep, class Period >
std::cv_status media_cache::wait_for(const std::chrono::duration<Rep, Period> &rel_time)
{
    const auto deadline = std::chrono::system_clock::now() + rel_time;
    std::unique_lock<std::mutex> l(mutex);

    std::cv_status result = std::cv_status::no_timeout;
    while (!thread_pool.empty() && (result == std::cv_status::no_timeout))
        result = condition.wait_until(l, deadline);

    if (result == std::cv_status::no_timeout)
        this->on_finished = nullptr;

    return result;
}

} // End of namespace

#endif
