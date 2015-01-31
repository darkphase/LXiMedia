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

#ifndef VLC_TRANSCODE_STREAM_H
#define VLC_TRANSCODE_STREAM_H

#include "platform/messageloop.h"
#include "platform/process.h"
#include "vlc/subtitles.h"
#include <chrono>
#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace vlc {

class instance;

struct track_ids { int audio, video, text; track_ids() : audio(-2), video(-2), text(-2) {} };

class transcode_stream : public std::istream
{
public:
    transcode_stream(class platform::messageloop_ref &, class instance &);
    ~transcode_stream();

    void add_option(const std::string &);
    void set_chapter(int);
    void set_position(std::chrono::milliseconds);
    void set_track_ids(const struct track_ids &);
    void set_subtitle_file(subtitles::file &&);

    bool open(
            const std::string &mrl,
            const std::string &transcode,
            const std::string &mux,
            float rate = 1.0f);

    void close();

    std::chrono::milliseconds playback_position() const;
    std::function<void(std::chrono::milliseconds)> on_playback_position_changed;

    bool end_reached() const;
    std::function<void()> on_end_reached;

private:
    static int transcode_process(platform::process &);
    void update_info();

private:
    static platform::process::function_handle transcode_function;

    class platform::messageloop_ref messageloop;
    class instance &instance;

    std::vector<std::string> options;
    int chapter;
    std::chrono::milliseconds position;
    struct track_ids track_ids;
    subtitles::file subtitle_file;

    std::unique_ptr<platform::process> process;
    struct shared_info;
    std::unique_ptr<shared_info> last_info;
    unsigned info_offset;
    platform::timer update_info_timer;
};

} // End of namespace

#endif
