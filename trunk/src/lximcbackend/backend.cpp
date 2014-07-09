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

#include "backend.h"

namespace lximediacenter {

backend::backend(class messageloop &messageloop)
  : messageloop(messageloop),
    upnp(messageloop),
    rootdevice(messageloop, upnp, "00000000-0000-0000-0000-000000000000", "urn:schemas-upnp-org:device:MediaServer:1"),
    connection_manager(messageloop, rootdevice),
    content_directory(messageloop, upnp, rootdevice, connection_manager),
    mediareceiver_registrar(messageloop, rootdevice)
{
  using namespace std::placeholders;

  upnp.http_callback_register("/"     , std::bind(&backend::http_request, this, _1, _2, _3));
  upnp.http_callback_register("/css"  , std::bind(&backend::http_request, this, _1, _2, _3));
  upnp.http_callback_register("/img"  , std::bind(&backend::http_request, this, _1, _2, _3));
  upnp.http_callback_register("/js"   , std::bind(&backend::http_request, this, _1, _2, _3));
  upnp.http_callback_register("/help" , std::bind(&backend::http_request, this, _1, _2, _3));
}

backend::~backend()
{
}

bool backend::initialize()
{
  return upnp.initialize(default_port, false);
}

int backend::http_request(const upnp::request &, std::string &, std::shared_ptr<std::istream> &)
{
  return upnp::http_not_found;
}

} // End of namespace
