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

#ifndef RESOURCE_FILE_H
#define RESOURCE_FILE_H

#include <cstdlib>
#include <string>

namespace resources {

class resource_file
{
public:
    template <size_t _size>
    explicit resource_file(const unsigned char (& data)[_size], const std::string &suffix)
        : resource_file(data, _size, suffix)
    {
    }

    ~resource_file();

    operator const std::string &() const { return filename; }

private:
    resource_file(const unsigned char *data, size_t size, const std::string &suffix);

    const std::string filename;
};

}

#endif
