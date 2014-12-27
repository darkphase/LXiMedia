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
#include <chrono>
#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace platform { class process; }

namespace vlc {

class instance;

struct track_ids { int audio, video, text; track_ids() : audio(-1), video(-1), text(-1) {} };

class transcode_stream : public std::istream
{
public:

public:
    transcode_stream(class platform::messageloop_ref &, class instance &);
    ~transcode_stream();

    void add_option(const std::string &);
    void set_chapter(int);
    void set_position(std::chrono::milliseconds);
    void set_track_ids(const struct track_ids &);

    bool open(
            const std::string &mrl,
            const std::string &transcode,
            const std::string &mux,
            float rate = 1.0f);

    void close();

    std::function<void(std::chrono::milliseconds)> on_playback_progress;

private:
    class streambuf;

    class platform::messageloop_ref messageloop;
    class instance &instance;

    std::vector<std::string> options;
    int chapter;
    std::chrono::milliseconds position;
    struct track_ids track_ids;

    std::unique_ptr<platform::process> process;
};

} // End of namespace

#endif
