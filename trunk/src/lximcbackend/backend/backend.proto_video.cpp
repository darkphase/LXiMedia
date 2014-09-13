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

template <size_t _count>
static void add_source_video_protocols(
        class pupnp::connection_manager &connection_manager,
        const char *name,
        const char *mime, const char *suffix,
        unsigned sample_rate, unsigned channels,
        const unsigned (& width)[_count], const unsigned (& height)[_count],
        const unsigned (& frame_rate_num)[_count], const unsigned (& frame_rate_den)[_count],
        const char *acodec, const char *vcodec, const char *mux,
        const char *fast_encode_options, const char *slow_encode_options)
{
    for (size_t i = 0; i < _count; i++)
        connection_manager.add_source_video_protocol(
                    name,
                    mime, suffix,
                    sample_rate, channels,
                    width[i], height[i], frame_rate_num[i], frame_rate_den[i],
                    acodec, vcodec, mux,
                    fast_encode_options,
                    slow_encode_options);
}

template <size_t _count>
static void add_source_video_protocols(
        class pupnp::connection_manager &connection_manager,
        const char *name,
        const char *mime, const char *suffix,
        unsigned sample_rate, unsigned channels,
        unsigned width, unsigned height,
        const unsigned (& frame_rate_num)[_count], const unsigned (& frame_rate_den)[_count],
        const char *acodec, const char *vcodec, const char *mux,
        const char *fast_encode_options, const char *slow_encode_options)
{
    for (size_t i = 0; i < _count; i++)
        connection_manager.add_source_video_protocol(
                    name,
                    mime, suffix,
                    sample_rate, channels,
                    width, height, frame_rate_num[i], frame_rate_den[i],
                    acodec, vcodec, mux,
                    fast_encode_options,
                    slow_encode_options);
}

static void add_source_video_protocols(
        class pupnp::connection_manager &connection_manager,
        const char * const (& name)[4],
        unsigned sample_rate, unsigned channels,
        const unsigned (& width)[2], const unsigned (& height)[2],
        const char *acodec, const char *vcodec,
        const char *fast_encode_options, const char *slow_encode_options)
{
    static const unsigned eu_frame_rate_num[] = { 24000, 25000 }, eu_frame_rate_den[] = { 1000, 1000 };
    static const unsigned na_frame_rate_num[] = { 24000, 30000, 30000 }, na_frame_rate_den[] = { 1001, 1001, 1000 };

    add_source_video_protocols(
                connection_manager,
                name[0],
                pupnp::upnp::mime_video_mpegts, "ts",
                sample_rate, channels,
                width[0], height[0],
                eu_frame_rate_num, eu_frame_rate_den,
                acodec, vcodec, "ts",
                fast_encode_options,
                slow_encode_options);

    add_source_video_protocols(
                connection_manager,
                name[1],
                pupnp::upnp::mime_video_mpeg, "mpg",
                sample_rate, channels,
                width[0], height[0],
                eu_frame_rate_num, eu_frame_rate_den,
                acodec, vcodec, "ps",
                fast_encode_options,
                slow_encode_options);

    add_source_video_protocols(
                connection_manager,
                name[2],
                pupnp::upnp::mime_video_mpegts, "ts",
                sample_rate, channels,
                width[1], height[1],
                na_frame_rate_num, na_frame_rate_den,
                acodec, vcodec, "ts",
                fast_encode_options,
                slow_encode_options);

    add_source_video_protocols(
                connection_manager,
                name[3],
                pupnp::upnp::mime_video_mpeg, "mpg",
                sample_rate, channels,
                width[1], height[1],
                na_frame_rate_num, na_frame_rate_den,
                acodec, vcodec, "ps",
                fast_encode_options,
                slow_encode_options);
}

static void add_source_video_protocols(
        class pupnp::connection_manager &connection_manager,
        const char *name,
        unsigned sample_rate, unsigned channels,
        const unsigned (& width)[2], const unsigned (& height)[2],
        const char *acodec, const char *vcodec,
        const char *fast_encode_options, const char *slow_encode_options)
{
    static const unsigned eu_frame_rate_num[] = { 24000, 25000 }, eu_frame_rate_den[] = { 1000, 1000 };
    static const unsigned na_frame_rate_num[] = { 24000, 30000, 30000 }, na_frame_rate_den[] = { 1001, 1001, 1000 };

    add_source_video_protocols(
                connection_manager,
                name,
                pupnp::upnp::mime_video_mpegts, "ts",
                sample_rate, channels,
                width[0], height[0],
                eu_frame_rate_num, eu_frame_rate_den,
                acodec, vcodec, "ts",
                fast_encode_options,
                slow_encode_options);

    add_source_video_protocols(
                connection_manager,
                name,
                pupnp::upnp::mime_video_mpegts, "ts",
                sample_rate, channels,
                width[1], height[1],
                na_frame_rate_num, na_frame_rate_den,
                acodec, vcodec, "ts",
                fast_encode_options,
                slow_encode_options);
}

void backend::add_video_protocols()
{
    const auto surround_mode = settings.surround_mode();
    const bool has_surround51 = surround_mode == ::surround_mode::surround51;
    const auto video_mode = settings.video_mode();
    const bool has_vcd              = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::vcd           );
    const bool has_dvd              = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::dvd           );
    const bool has_dvd_avc          = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::dvd_avc       );
    const bool has_hdtv_720         = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::hdtv_720      );
    const bool has_hdtv_720_avc     = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::hdtv_720_avc  );
    const bool has_hdtv_1080        = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::hdtv_1080     );
    const bool has_hdtv_1080_avc    = (video_mode == ::video_mode::auto_) || (video_mode == ::video_mode::hdtv_1080_avc );

    // See: http://www.videolan.org/doc/streaming-howto/en/ch03.html

    /////////////////////////////////////////////////////////////////////////////
    // MPEG1
    if (has_vcd)
    {
        static const unsigned width[] = { 352, 320 }, height[] = { 288, 240 };
        static const unsigned frame_rate_num[] = { 25000, 30000 }, frame_rate_den[] = { 1000, 1001 };

        add_source_video_protocols(
                    connection_manager,
                    "MPEG1",
                    pupnp::upnp::mime_video_mpeg, "mpg",
                    44100, 2,
                    width, height,
                    frame_rate_num, frame_rate_den,
                    "acodec=mpga,ab=256", "vcodec=mp1v", "mpeg1",
                    "vb=4096,venc=ffmpeg{keyint=0,vt=2048}",
                    "vb=2048,venc=ffmpeg{bframes=0,vt=1024}");
    }

    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 PAL/NTSC
    {
        if (has_dvd)
        {
            connection_manager.add_source_video_protocol(
                        "MPEG_PS_PAL",
                        pupnp::upnp::mime_video_mpeg, "mpg",
                        44100, 2,
                        720, 576,
                        25000, 1000,
                        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

            connection_manager.add_source_video_protocol(
                        "MPEG_PS_NTSC",
                        pupnp::upnp::mime_video_mpeg, "mpg",
                        44100, 2,
                        704, 480,
                        30000, 1001,
                        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");
        }

        if (has_dvd && has_surround51)
        {
            connection_manager.add_source_video_protocol(
                        "MPEG_PS_PAL_XAC3",
                        pupnp::upnp::mime_video_mpeg, "mpg",
                        48000, 6,
                        720, 576,
                        25000, 1000,
                        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

            connection_manager.add_source_video_protocol(
                         "MPEG_PS_NTSC_XAC3",
                        pupnp::upnp::mime_video_mpeg, "mpg",
                        48000, 6,
                        704, 480,
                        30000, 1001,
                        "acodec=mpga,ab=256", "vcodec=mp2v", "ps",
                        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");
        }
    }

    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 SD
    {
        static const char * const name[] = { "MPEG_TS_SD_EU_ISO", "MPEG_PS_SD_EU_NONSTD", "MPEG_TS_SD_NA_ISO", "MPEG_PS_SD_NA_NONSTD" };
        static const unsigned width[] = { 720, 704 }, height[] = { 576, 480 };

        if (has_dvd)
            add_source_video_protocols(
                        connection_manager,
                        name,
                        44100, 2,
                        width, height,
                        "acodec=mpga,ab=256", "vcodec=mp2v",
                        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");

        if (has_dvd && has_surround51)
            add_source_video_protocols(
                        connection_manager,
                        name,
                        48000, 6,
                        width, height,
                        "acodec=a52,ab=640", "vcodec=mp2v",
                        "vb=8192,venc=ffmpeg{keyint=0,vt=4096}",
                        "vb=4096,venc=ffmpeg{bframes=0,vt=2048}");
    }

    /////////////////////////////////////////////////////////////////////////////
    // MPEG2 HD
    {
        static const char * const name[] = { "MPEG_TS_HD_EU_ISO", "MPEG_PS_HD_EU_NONSTD", "MPEG_TS_HD_NA_ISO", "MPEG_PS_HD_NA_NONSTD" };
        static const unsigned width_720[] = { 1280, 1280 }, height_720[] = { 720, 720 };
        static const unsigned width_1080[] = { 1920, 1920 }, height_1080[] = { 1080, 1080 };

        if (has_hdtv_720)
            add_source_video_protocols(
                        connection_manager,
                        name,
                        44100, 2,
                        width_720, height_720,
                        "acodec=mpga,ab=256", "vcodec=mp2v",
                        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

        if (has_hdtv_720 && has_surround51)
            add_source_video_protocols(
                        connection_manager,
                        name,
                        48000, 6,
                        width_720, height_720,
                        "acodec=a52,ab=640", "vcodec=mp2v",
                        "vb=16384,venc=ffmpeg{keyint=0,vt=8192}",
                        "vb=8192,venc=ffmpeg{bframes=0,vt=4096}");

        if (has_hdtv_1080)
            add_source_video_protocols(
                        connection_manager,
                        name,
                        44100, 2,
                        width_1080, height_1080,
                        "acodec=mpga,ab=256", "vcodec=mp2v",
                        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");

        if (has_hdtv_1080 && has_surround51)
            add_source_video_protocols(
                        connection_manager,
                        name,
                        48000, 6,
                        width_1080, height_1080,
                        "acodec=a52,ab=640", "vcodec=mp2v",
                        "vb=32768,venc=ffmpeg{keyint=0,vt=16384}",
                        "vb=16384,venc=ffmpeg{bframes=0,vt=8192}");
    }

    /////////////////////////////////////////////////////////////////////////////
    // MPEG4 SD
    {
        static const unsigned width[] = { 720, 704 }, height[] = { 576, 480 };

        if (has_dvd_avc)
            add_source_video_protocols(
                        connection_manager,
                        "AVC_TS_MP_SD_MPEG1_L3_ISO",
                        44100, 2,
                        width, height,
                        "acodec=mp3,ab=128", "vcodec=h264",
                        "vb=4096,venc=x264{keyint=1,bframes=0}",
                        "vb=2048,venc=x264{keyint=25,bframes=3}");

        if (has_dvd_avc && has_surround51)
            add_source_video_protocols(
                        connection_manager,
                        "AVC_TS_MP_SD_AC3_ISO",
                        48000, 6,
                        width, height,
                        "acodec=a52,ab=640", "vcodec=h264",
                        "vb=4096,venc=x264{keyint=1,bframes=0}",
                        "vb=2048,venc=x264{keyint=25,bframes=3}");
    }

    /////////////////////////////////////////////////////////////////////////////
    // MPEG4 HD
    {
        static const unsigned width_720[] = { 1280, 1280 }, height_720[] = { 720, 720 };
        static const unsigned width_1080[] = { 1920, 1920 }, height_1080[] = { 1080, 1080 };

        if (has_hdtv_720_avc)
            add_source_video_protocols(
                        connection_manager,
                        "AVC_TS_MP_HD_MPEG1_L3_ISO",
                        44100, 2,
                        width_720, height_720,
                        "acodec=mp3,ab=128", "vcodec=h264",
                        "vb=8192,venc=x264{keyint=1,bframes=0}",
                        "vb=4096,venc=x264{keyint=25,bframes=3}");

        if (has_hdtv_720_avc && has_surround51)
            add_source_video_protocols(
                        connection_manager,
                        "AVC_TS_MP_HD_AC3_ISO",
                        48000, 6,
                        width_720, height_720,
                        "acodec=a52,ab=640", "vcodec=h264",
                        "vb=8192,venc=x264{keyint=1,bframes=0}",
                        "vb=4096,venc=x264{keyint=25,bframes=3}");

        if (has_hdtv_1080_avc)
            add_source_video_protocols(
                        connection_manager,
                        "AVC_TS_MP_HD_MPEG1_L3_ISO",
                        44100, 2,
                        width_1080, height_1080,
                        "acodec=mp3,ab=128", "vcodec=h264",
                        "vb=16384,venc=x264{keyint=1,bframes=0}",
                        "vb=8192,venc=x264{keyint=25,bframes=3}");

        if (has_hdtv_1080_avc && has_surround51)
            add_source_video_protocols(
                        connection_manager,
                        "AVC_TS_MP_HD_AC3_ISO",
                        48000, 6,
                        width_1080, height_1080,
                        "acodec=a52,ab=640", "vcodec=h264",
                        "vb=16384,venc=x264{keyint=1,bframes=0}",
                        "vb=8192,venc=x264{keyint=25,bframes=3}");
    }
}
