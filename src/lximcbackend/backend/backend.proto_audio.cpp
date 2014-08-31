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

void backend::add_audio_protocols()
{
    const auto surround_mode = settings.surround_mode();
    const bool has_surround51 = surround_mode == ::surround_mode::surround51;

    // See: http://www.videolan.org/doc/streaming-howto/en/ch03.html

    connection_manager.add_source_audio_protocol(
                "LPCM",
                "audio/L16;rate=48000;channels=2", "lpcm",
                48000, 2,
                "acodec=lpcm",
                "dummy");

    connection_manager.add_source_audio_protocol(
                "MP2",
                "audio/mpeg", "mp2",
                44100, 2,
                "acodec=mpga,ab=256",
                "dummy");

    connection_manager.add_source_audio_protocol(
                "MP3",
                "audio/mpeg", "mp3",
                44100, 2,
                "acodec=mp3,ab=160",
                "dummy");

    connection_manager.add_source_audio_protocol(
                "AAC_ADTS",
                "audio/aac", "aac",
                48000, 2,
                "acodec=mp4a,ab=128",
                "dummy");

    if (has_surround51)
        connection_manager.add_source_audio_protocol(
                    "AC3",
                    "audio/x-ac3", "ac3",
                    48000, 6,
                    "acodec=a52,ab=640",
                    "dummy");
}
