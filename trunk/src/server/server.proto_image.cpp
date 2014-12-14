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

#include "server.h"
#include "pupnp/upnp.h"

void server::add_image_protocols()
{
    const auto video_mode = settings.video_mode();
    const bool has_dvd              = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::dvd       );
    const bool has_hdtv_720         = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::hdtv_720  );
    const bool has_hdtv_1080        = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::hdtv_1080 );

    connection_manager.add_source_image_protocol(
                "PNG_TN",
                pupnp::upnp::mime_image_png, "png",
                160, 160);

    if (has_hdtv_1080)
    {
        connection_manager.add_source_image_protocol(
                    "PNG_LRG",
                    pupnp::upnp::mime_image_png, "png",
                    1920, 1080);
    }
    else if (has_hdtv_720)
    {
        connection_manager.add_source_image_protocol(
                    "PNG_LRG",
                    pupnp::upnp::mime_image_png, "png",
                    1280, 720);
    }
    else if (has_dvd)
    {
        connection_manager.add_source_image_protocol(
                    "PNG_LRG",
                    pupnp::upnp::mime_image_png, "png",
                    768, 576);
    }
}
