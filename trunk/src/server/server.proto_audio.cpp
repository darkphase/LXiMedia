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

void server::add_audio_protocols()
{
    const auto surround_mode = settings.surround_mode();
    const bool has_surround51 = (surround_mode == ::surround_mode::surround51);

    // See: http://www.videolan.org/doc/streaming-howto/en/ch03.html

//    connection_manager.add_source_audio_protocol(
//                "LPCM",
//                pupnp::upnp::mime_audio_lpcm_48000_2, "lpcm",
//                48000, 2,
//                "acodec=lpcm",
//                "dummy");

    connection_manager.add_source_audio_protocol(
                "MP2",
                pupnp::upnp::mime_audio_mpeg, "mp2",
                44100, 2,
                "acodec=mpga,ab=256",
                "dummy");

    connection_manager.add_source_audio_protocol(
                "MP3",
                pupnp::upnp::mime_audio_mp3, "mp3",
                44100, 2,
                "acodec=mp3,ab=160",
                "dummy");

    connection_manager.add_source_audio_protocol(
                "AC3",
                pupnp::upnp::mime_audio_ac3, "ac3",
                48000, 2,
                "acodec=a52,ab=192",
                "dummy");

    if (has_surround51)
        connection_manager.add_source_audio_protocol(
                    "AC3",
                    pupnp::upnp::mime_audio_ac3, "ac3",
                    48000, 6,
                    "acodec=a52,ab=504",
                    "dummy");
}
