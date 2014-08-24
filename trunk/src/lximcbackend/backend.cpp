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
      mainpage(upnp),
      settingspage(mainpage, settings),
      mediaplayer(messageloop, vlc_instance, connection_manager, content_directory, settings)
{
}

backend::~backend()
{
}

bool backend::initialize()
{
    rootdevice.set_devicename(settings.devicename());
    mainpage.set_devicename(settings.devicename());

    add_audio_protocols();
    add_video_protocols();

    return upnp.initialize(settings.http_port(), false);
}
