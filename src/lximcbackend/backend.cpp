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

backend::backend(class messageloop &messageloop)
  : messageloop(messageloop),
    settings(messageloop),
    vlc_instance(),
    upnp(messageloop),
    rootdevice(messageloop, upnp, settings.uuid(), "urn:schemas-upnp-org:device:MediaServer:1"),
    connection_manager(messageloop, rootdevice),
    content_directory(messageloop, upnp, rootdevice, connection_manager),
    mediareceiver_registrar(messageloop, rootdevice),
    mediaplayer(messageloop, vlc_instance, connection_manager, content_directory, settings.encode_mode())
{
  using namespace std::placeholders;

  upnp.http_callback_register("/"     , std::bind(&backend::http_request, this, _1, _2, _3));
  upnp.http_callback_register("/css"  , std::bind(&backend::http_request, this, _1, _2, _3));
  upnp.http_callback_register("/img"  , std::bind(&backend::http_request, this, _1, _2, _3));
  upnp.http_callback_register("/js"   , std::bind(&backend::http_request, this, _1, _2, _3));
  upnp.http_callback_register("/help" , std::bind(&backend::http_request, this, _1, _2, _3));

  rootdevice.set_devicename(settings.devicename());
}

backend::~backend()
{
}

bool backend::initialize()
{
  return upnp.initialize(settings.http_port(), false);
}

int backend::http_request(const pupnp::upnp::request &request, std::string &content_type, std::shared_ptr<std::istream> &response)
{
  if (request.url.path == "/exit")
  {
    messageloop.post([this] { messageloop.stop(0); });
    return pupnp::upnp::http_no_content;
  }

  return pupnp::upnp::http_not_found;
}
