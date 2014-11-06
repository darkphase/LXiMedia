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
#include <istream>
#include <memory>
#include <string>

namespace pupnp {

class connection_proxy : public std::istream
{
public:
    connection_proxy();
    connection_proxy(std::unique_ptr<std::istream> &&input);

    ~connection_proxy();

    bool attach(connection_proxy &);

    void subscribe_close(platform::messageloop_ref &, const std::function<void()> &);
    void subscribe_detach(platform::messageloop_ref &, const std::function<void()> &);

private:
    class streambuf;
    class source;

    std::shared_ptr<class source> source;
};

} // End of namespace

#endif
