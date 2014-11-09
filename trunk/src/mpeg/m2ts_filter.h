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

#ifndef MPEG_M2TS_FILTER_H
#define MPEG_M2TS_FILTER_H

#include <istream>
#include <memory>

namespace mpeg {

class m2ts_filter : public std::istream
{
public:
    m2ts_filter(std::unique_ptr<std::istream> &&input);
    ~m2ts_filter();

private:
    bool read_ts_packet(char *);

private:
    class streambuf;

    const std::unique_ptr<std::istream> input;
};

} // End of namespace

#endif
