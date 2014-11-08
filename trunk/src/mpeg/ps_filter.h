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

#ifndef MPEG_PS_FILTER_H
#define MPEG_PS_FILTER_H

#include "mpeg.h"
#include <istream>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace mpeg {

class ps_filter : public std::istream
{
public:
    ps_filter(std::unique_ptr<std::istream> &&input);
    ~ps_filter();

private:
    std::list<ps_packet> read_pack();
    void filter_packet();
    ps_packet read_ps_packet();

private:
    class streambuf;

    const std::unique_ptr<std::istream> input;
    bool stream_finished;

    std::map<stream_type, std::list<pes_packet>> streams;
    std::map<stream_type, uint64_t> last_timestamp;
    uint64_t clock_offset;

    uint64_t next_pack_header;
};

} // End of namespace

#endif
