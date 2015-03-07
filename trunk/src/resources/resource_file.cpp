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

#include "resource_file.h"
#include "platform/fstream.h"
#include "platform/path.h"
#include <stdexcept>

namespace resources {

resource_file::resource_file(const unsigned char *data, size_t size, const std::string &suffix)
    : filename(platform::temp_file_path(suffix))
{
    platform::ofstream str(filename, std::ios::binary);
    if (str.is_open())
        str.write(reinterpret_cast<const char *>(data), size);
}

resource_file::~resource_file()
{
    platform::remove_file(filename);
}

}
