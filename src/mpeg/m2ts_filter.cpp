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

#include "m2ts_filter.h"
#include <cassert>
#include <cstring>

namespace mpeg {

static const char ts_sync_byte = 0x47;
static const size_t ts_packet_size = 188;

class m2ts_filter::streambuf : public std::streambuf
{
public:
    streambuf(class m2ts_filter &);

    int underflow() override;

private:
    class m2ts_filter &parent;

    static const size_t putback = 8;
    char buffer[putback + sizeof(uint32_t) + ts_packet_size];
};

m2ts_filter::m2ts_filter(std::unique_ptr<std::istream> &&input)
    : std::istream(new class streambuf(*this)),
      input(std::move(input))
{
}

m2ts_filter::~m2ts_filter()
{
    delete std::istream::rdbuf(nullptr);
}

bool m2ts_filter::read_ts_packet(char *dest)
{
    do dest[0] = input->get(); while((dest[0] != ts_sync_byte) && *input);

    return bool(input->read(dest + 1, ts_packet_size - 1));
}


m2ts_filter::streambuf::streambuf(class m2ts_filter &parent)
    : parent(parent)
{
    memset(buffer, 0, sizeof(buffer));
}

int m2ts_filter::streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*gptr());

    memset(buffer + putback, 0, sizeof(uint32_t));

    if (parent.read_ts_packet(buffer + putback + sizeof(uint32_t)))
    {
        setg(buffer, buffer + putback, buffer + putback + sizeof(uint32_t) + ts_packet_size);
        return traits_type::to_int_type(*gptr());
    }
    else
        return traits_type::eof();
}

} // End of namespace
