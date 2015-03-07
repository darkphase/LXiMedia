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

#ifndef UUID_H
#define UUID_H

#include <string>
#include <istream>
#include <ostream>

namespace platform {

struct uuid
{
    static uuid generate();
    uuid();
    uuid(const std::string &);

    bool operator==(const uuid &) const;
    bool operator!=(const uuid &from) const { return !operator==(from); }
    bool operator<(const uuid &) const;
    operator std::string() const;
    bool is_null() const;

    uint8_t value[16];
};

std::ostream & operator<<(std::ostream &, const uuid &);
std::istream & operator>>(std::istream &, uuid &);

}

#endif
