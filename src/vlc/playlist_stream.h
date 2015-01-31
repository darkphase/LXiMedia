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

#ifndef VLC_PLAYLIST_STREAM_H
#define VLC_PLAYLIST_STREAM_H

#include "platform/messageloop.h"
#include <istream>
#include <memory>
#include <string>
#include <queue>

namespace vlc {

class playlist_stream : public std::istream
{
public:
    playlist_stream(
            class platform::messageloop_ref &,
            const std::string &transcode,
            const std::string &mux);

    ~playlist_stream();

    void add(const std::string &mrl);

private:
    class streambuf;

    class platform::messageloop_ref messageloop;
    const std::string transcode;
    const std::string mux;

    std::queue<std::string> items;
};

} // End of namespace

#endif
