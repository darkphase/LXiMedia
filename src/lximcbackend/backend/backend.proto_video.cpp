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
#include "pupnp/upnp.h"

void backend::add_video_protocols()
{
    const auto surround_mode = settings.surround_mode();
    const bool has_surround51 = surround_mode == ::surround_mode::surround51;
    const auto video_mode = settings.video_mode();
    const bool has_vcd          = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::vcd       );
    const bool has_dvd          = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::dvd       );
    const bool has_hdtv_720     = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::hdtv_720  );
    const bool has_hdtv_1080    = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::hdtv_1080 );

    // See: http://www.videolan.org/doc/streaming-howto/en/ch03.html

    /////////////////////////////////////////////////////////////////////////////
    // MPEG1
    if (has_vcd)
        connection_manager.add_source_video_protocol(
                    "MPEG1",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 352, 288, 25000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp1v", "mpeg1",
                    "vb=4096,venc=ffmpeg{keyint=0,vt=2048}",
                    "vb=2048,venc=ffmpeg{bframes=0,vt=1024}");

    if (has_vcd)
        connection_manager.add_source_video_protocol(
                    "MPEG1",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 320, 240, 30000, 1001,
                    "acodec=mpga,ab=256", "vcodec=mp1v", "mpeg1",
                    "vb=4096,venc=ffmpeg{keyint=0,vt=2048}",
                    "vb=2048,venc=ffmpeg{bframes=0,vt=1024}");


    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 PAL/NTSC
    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_PAL",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 720, 576, 25000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_PAL_XAC3",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 720, 576, 25000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_NTSC",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 704, 480, 30000, 1001,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_NTSC_XAC3",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 704, 480, 30000, 1001,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 SD
    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_SD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 720, 576, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_SD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 720, 576, 25000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_SD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 720, 576, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_SD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 720, 576, 25000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_SD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 704, 480, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_SD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 704, 480, 30000, 1001,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_SD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 704, 480, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_SD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 704, 480, 30000, 1001,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 SD - nonstandard program stream
    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_SD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 720, 576, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_SD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 720, 576, 25000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_SD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 720, 576, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_SD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 720, 576, 25000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_SD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 704, 480, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_SD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 704, 480, 30000, 1001,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_SD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 704, 480, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    if (has_dvd && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_SD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 704, 480, 30000, 1001,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                    "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 720p
    if (has_hdtv_720)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 1280, 720, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 1280, 720, 25000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 1280, 720, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 1280, 720, 25000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 1280, 720, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 1280, 720, 30000, 1001,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 1280, 720, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 1280, 720, 30000, 1001,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 720p - nonstandard program stream
    if (has_hdtv_720)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 1280, 720, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 1280, 720, 25000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 1280, 720, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 1280, 720, 25000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 1280, 720, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 1280, 720, 30000, 1001,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 1280, 720, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    if (has_hdtv_720 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 1280, 720, 30000, 1001,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                    "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 1080p
    if (has_hdtv_1080)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 1920, 1080, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 1920, 1080, 25000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 1920, 1080, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_EU_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 1920, 1080, 25000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 1920, 1080, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    44100, 2, 1920, 1080, 30000, 1001,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ts",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 1920, 1080, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_TS_HD_NA_ISO",
                    pupnp::upnp::mime_video_mpegts, "ts",
                    48000, 6, 1920, 1080, 30000, 1001,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ts",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 1080p - nonstandard program stream
    if (has_hdtv_1080)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 1920, 1080, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 1920, 1080, 25000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 1920, 1080, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_EU_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 1920, 1080, 25000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 1920, 1080, 24000, 1000,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2, 1920, 1080, 30000, 1001,
                    "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 1920, 1080, 24000, 1000,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

    if (has_hdtv_1080 && has_surround51)
        connection_manager.add_source_video_protocol(
                    "MPEG_PS_HD_NA_NONSTD",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    48000, 6, 1920, 1080, 30000, 1001,
                    "acodec=a52,ab=640", "vcodec=mp2v", "ps",
                    "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                    "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

}
