/******************************************************************************
 *   Copyright (C) 2015  A.J. Admiraal                                        *
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

#include "vlc/playlist_stream.h"
#include "vlc/transcode_stream.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

static const size_t buffer_size = 1048576;

namespace vlc {

class playlist_stream::streambuf : public std::streambuf
{
public:
    streambuf(class playlist_stream &);

    int underflow() override;

private:
    std::unique_ptr<std::istream> open_next();

private:
    class playlist_stream &parent;
    std::unique_ptr<std::istream> input;
    std::vector<char> buffer;
};

playlist_stream::playlist_stream(
        class platform::messageloop_ref &messageloop,
        const std::string &transcode,
        const std::string &mux)
    : std::istream(new class streambuf(*this)),
      messageloop(messageloop),
      transcode(transcode),
      mux(mux)
{
}

playlist_stream::~playlist_stream()
{
    delete std::istream::rdbuf(nullptr);
}

void playlist_stream::add(const std::string &mrl)
{
    items.emplace(mrl);
}


playlist_stream::streambuf::streambuf(class playlist_stream &parent)
    : parent(parent),
      input(nullptr)
{
    buffer.resize(buffer_size);
}

std::unique_ptr<std::istream> playlist_stream::streambuf::open_next()
{
    while (!parent.items.empty())
    {
        std::unique_ptr<transcode_stream> stream(
                    new transcode_stream(parent.messageloop));

        const std::string mrl = std::move(parent.items.front());
        parent.items.pop();

        if (stream->open(mrl, parent.transcode, parent.mux))
            return std::move(stream);
    }

    return nullptr;
}

int playlist_stream::streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*gptr());

    for (;;)
    {
        while (!input || !*input)
        {
            input = nullptr;
            input = open_next();
            if (!input)
                break;
        }

        input->read(buffer.data(), buffer.size());
        const size_t read = input->gcount();

        if (read > 0)
        {
            setg(&buffer[0], &buffer[0], &buffer[read]);
            return traits_type::to_int_type(*gptr());
        }
    }

    return traits_type::eof();
}

} // End of namespace
