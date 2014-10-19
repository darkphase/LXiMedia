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

#ifndef PUPNP_CONNECTION_PROXY_H
#define PUPNP_CONNECTION_PROXY_H

#include "platform/messageloop.h"
#include "pupnp/connection_manager.h"
#include <chrono>
#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace pupnp {

class connection_proxy : public std::istream
{
public:
    connection_proxy();

    connection_proxy(
            class connection_manager &,
            class connection_manager::protocol &,
            const std::string &mrl,
            const std::string &source_address,
            std::unique_ptr<std::istream> &&input);

    ~connection_proxy();

    bool attach(connection_proxy &);

private:
    class streambuf;
    class source;

    const std::unique_ptr<class streambuf> streambuf;
    std::shared_ptr<class source> source;
};

} // End of namespace

#endif
