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

#ifndef VLC_IMAGE_STREAM_H
#define VLC_IMAGE_STREAM_H

#include "platform/messageloop.h"
#include "platform/process.h"
#include <istream>
#include <memory>
#include <string>

namespace platform { class process; }

namespace vlc {

class instance;

class image_stream : public std::istream
{
public:
    explicit image_stream(class instance &);
    ~image_stream();

    bool open(
            const std::string &mrl,
            const std::string &mime,
            unsigned width, unsigned height);

    void close();

private:
    static int transcode_process(platform::process &);

private:
    static platform::process::function_handle transcode_function;

    class instance &instance;

    std::unique_ptr<platform::process> process;
};

} // End of namespace

#endif
