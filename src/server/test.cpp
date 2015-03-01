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

#include "test.h"
#include "mpeg/m2ts_filter.h"
#include "mpeg/ps_filter.h"
#include "platform/path.h"
#include "platform/string.h"
#include "platform/translator.h"
#include "resources/resources.h"
#include "server/html/setuppage.h"
#include "vlc/transcode_stream.h"
#include <cmath>
#include <iostream>
#include <sstream>

test::test(
        class platform::messageloop_ref &messageloop,
        class pupnp::connection_manager &connection_manager,
        class pupnp::content_directory &content_directory,
        const class settings &settings)
    : messageloop(messageloop),
      connection_manager(connection_manager),
      content_directory(content_directory),
      settings(settings),
      a440hz_mp2(resources::a440hz_mp2, "mp2"),
      pm5544_png(resources::pm5544_png, "png"),
      pm5644_png(resources::pm5644_png, "png")
{
    content_directory.item_source_register("/", *this);
}

test::~test()
{
    content_directory.item_source_unregister("/");
}

std::vector<pupnp::content_directory::item> test::list_contentdir_items(
        const std::string &client,
        const std::string &path,
        size_t start, size_t &count)
{
    const bool return_all = count == 0;

    clients.insert(client);

    std::vector<pupnp::content_directory::item> items;
    if (path == "/")
    {
        switch (html::setuppage::setup_mode())
        {
        case html::setup_mode::disabled:
        case html::setup_mode::name:
            break;

        case html::setup_mode::network:
        case html::setup_mode::codecs:
            items.emplace_back(get_contentdir_item(client, "/formats/"));
            break;

        case html::setup_mode::high_definition:
            items.emplace_back(get_contentdir_item(client, "/resolutions/"));
            break;
        }
    }
    else if (path == "/formats/")
    {
        items.emplace_back(get_contentdir_item(client, "/formats/pm5544_mp2v_mpg"));
        items.emplace_back(get_contentdir_item(client, "/formats/pm5544_mp2v_m2ts"));
        items.emplace_back(get_contentdir_item(client, "/formats/pm5544_mp2v_ts"));
        items.emplace_back(get_contentdir_item(client, "/formats/pm5544_h264_m2ts"));
        items.emplace_back(get_contentdir_item(client, "/formats/pm5544_h264_ts"));
    }
    else if (path == "/resolutions/")
    {
        items.emplace_back(get_contentdir_item(client, "/resolutions/pm5644_hd_720"));
        items.emplace_back(get_contentdir_item(client, "/resolutions/pm5644_hd_1080"));
    }

    std::vector<pupnp::content_directory::item> result;
    for (size_t i=start, n=0; (i<items.size()) && (return_all || (n<count)); i++, n++)
        result.emplace_back(std::move(items[i]));

    count = items.size();

    return result;
}

pupnp::content_directory::item test::get_contentdir_item(const std::string &, const std::string &path)
{
    pupnp::content_directory::item item;
    item.is_dir = ends_with(path, "/");
    item.path = path;

    if (item.is_dir)
    {
        if (path == "/formats/")
            item.title = "Formats";
        else if (path == "/resolutions/")
            item.title = "Resolutions";
    }
    else
    {
        item.type = pupnp::content_directory::item_type::video_broadcast;

        item.frame_rate = 10.0f;
        item.duration = std::chrono::seconds(10);
        item.sample_rate = 44100;
        item.channels = 2;

        if (starts_with(path, "/formats/"))
        {
            item.mrl = platform::mrl_from_path(pm5544_png);
            item.uuid = platform::uuid("fe23d774-4956-4608-aa17-78fb4af8b5a4");
            item.width = 768;
            item.height = 576;

            if      (path == "/formats/pm5544_mp2v_mpg")    item.title = "MPEG 2 Program Stream";
            else if (path == "/formats/pm5544_mp2v_m2ts")   item.title = "MPEG 2 BDAV Transport Stream";
            else if (path == "/formats/pm5544_mp2v_ts")     item.title = "MPEG 2 Transport Stream";
            else if (path == "/formats/pm5544_h264_m2ts")   item.title = "MPEG 4 AVC BDAV Transport Stream";
            else if (path == "/formats/pm5544_h264_ts")     item.title = "MPEG 4 AVC Transport Stream";
        }
        else if (starts_with(path, "/resolutions/"))
        {
            item.mrl = platform::mrl_from_path(pm5644_png);
            item.uuid = platform::uuid("b6a79f3c-b6ad-457b-b47a-35827d8f171c");

            if (path == "/resolutions/pm5644_hd_720")
            {
                item.width = 1280;
                item.height = 720;
                item.title = "High Definition 720p";
            }
            else if (path == "/resolutions/pm5644_hd_1080")
            {
                item.width = 1920;
                item.height = 1080;
                item.title = "High Definition 1080p";
            }
        }
    }

    return item;
}

bool test::correct_protocol(const pupnp::content_directory::item &item, pupnp::connection_manager::protocol &protocol)
{
    if (item.path == "/formats/pm5544_mp2v_mpg")
    {
        return (protocol.profile == "MPEG_PS_PAL") || (protocol.profile == "MPEG_PS_NTSC");
    }
    else if (item.path == "/formats/pm5544_mp2v_m2ts")
    {
        return (protocol.profile == "MPEG_TS_SD_EU") || (protocol.profile == "MPEG_TS_SD_NA");
    }
    else if (item.path == "/formats/pm5544_mp2v_ts")
    {
        return (protocol.profile == "MPEG_TS_SD_EU_ISO") || (protocol.profile == "MPEG_TS_SD_NA_ISO");
    }
    else if (item.path == "/formats/pm5544_h264_m2ts")
    {
        return protocol.profile == "AVC_TS_MP_SD_AC3";
    }
    else if (item.path == "/formats/pm5544_h264_ts")
    {
        return protocol.profile == "AVC_TS_MP_SD_AC3_ISO";
    }
    else if (starts_with(item.path, "/resolutions/"))
    {
        if ((protocol.profile == "MPEG_TS_HD_EU") || (protocol.profile == "MPEG_TS_HD_NA") ||
            (protocol.profile == "MPEG_TS_HD_EU_ISO") || (protocol.profile == "MPEG_TS_HD_NA_ISO") ||
            (protocol.profile == "MPEG_PS_HD_EU_NONSTD") || (protocol.profile == "MPEG_PS_HD_NA_NONSTD") ||
            (protocol.profile == "AVC_TS_MP_HD_MPEG1_L3") ||
            (protocol.profile == "AVC_TS_MP_HD_MPEG1_L3_ISO"))
        {
            if (item.path == "/resolutions/pm5644_hd_720")
                return protocol.height == 720;
            else if (item.path == "/resolutions/pm5644_hd_1080")
                return protocol.height == 1080;
        }
    }

    return false;
}

int test::play_item(
        const std::string &source_address,
        const pupnp::content_directory::item &item,
        const std::string &profile,
        std::string &content_type,
        std::shared_ptr<std::istream> &response)
{
    auto protocol = connection_manager.get_protocol(profile, item.channels, item.width, item.frame_rate);
    if (!protocol.profile.empty() && correct_protocol(item, protocol))
    {
        float rate = 1.0f;
        std::ostringstream transcode;
        if (!protocol.audio_codec.empty() || !protocol.video_codec.empty())
        {
            // See: http://www.videolan.org/doc/streaming-howto/en/ch03.html
            transcode << "#transcode{";

            if (!protocol.video_codec.empty())
            {
                transcode << "vcodec=" << protocol.video_codec
                          << ",vb=" << protocol.video_rate;

                const float frame_rate = float(protocol.frame_rate_num) / float(protocol.frame_rate_den);
                if (std::abs(frame_rate - item.frame_rate) > 0.3f)
                {
                    transcode << ",fps=" << frame_rate;
                    //                    if (std::abs(protocol.frame_rate - item.frame_rate) < 1.5f)
                    //                        rate = protocol.frame_rate / item.frame_rate;
                }

                transcode << ",width=" << protocol.width << ",height=" << protocol.height;

                if (settings.canvas_mode_enabled() &&
                    ((item.width != protocol.width) || (item.height != protocol.height)))
                {
                    transcode
                            << ",vfilter=canvas{width=" << protocol.width
                            << ",height=" << protocol.height;

                    const float aspect = float(protocol.width * protocol.aspect) / float(protocol.height);
                    if (std::abs(aspect - 1.33333f) < 0.1f)
                        transcode << ",aspect=4:3";
                    else if (std::abs(aspect - 1.77778f) < 0.1f)
                        transcode << ",aspect=16:9";

                    transcode << ",padd=true}";
                }

                switch (settings.encode_mode())
                {
                case ::encode_mode::fast:
                    if (!protocol.fast_encode_options.empty())
                        transcode << ',' << protocol.fast_encode_options;

                    break;

                case ::encode_mode::slow:
                    if (!protocol.slow_encode_options.empty())
                        transcode << ',' << protocol.slow_encode_options;

                    break;
                }

                transcode << ",soverlay";
            }

            if (!protocol.audio_codec.empty())
            {
                if (!protocol.video_codec.empty()) transcode << ',';

                transcode
                        << "acodec=" << protocol.audio_codec
                        << ",ab=" << protocol.audio_rate
                        << ",samplerate=" << protocol.sample_rate
                        << ",channels=" << protocol.channels;
            }

            transcode << '}';
        }

        std::ostringstream opt;
        if (item.chapter > 0)               opt << "@C" << item.chapter;
        else if (item.position.count() > 0) opt << "@" << item.position.count();

        // First try to attach to an already running stream.
        response = connection_manager.try_attach_output_connection(protocol, item.mrl, source_address, opt.str());
        if (!response)
        {
            std::clog << "test: creating new stream " << item.mrl << " transcode=" << transcode.str() << " mux=" << protocol.mux << std::endl;

            std::unique_ptr<vlc::transcode_stream> stream(
                        new vlc::transcode_stream(messageloop));
            stream->add_option(":input-slave=" + platform::mrl_from_path(a440hz_mp2));

            const std::string vlc_mux = (protocol.mux == "m2ts") ? "ts" : protocol.mux;

            if (item.chapter > 0)
                stream->set_chapter(item.chapter);
            else if (item.position.count() > 0)
                stream->set_position(item.position);

            struct vlc::track_ids track_ids;
            stream->set_track_ids(track_ids);

            stream->on_end_reached = [this, item] { items.insert(item.path); };

            if (stream->open(item.mrl, transcode.str(), vlc_mux, rate))
            {
                std::shared_ptr<pupnp::connection_proxy> proxy;
                if (protocol.mux == "ps")
                {
                    std::unique_ptr<mpeg::ps_filter> filter(new mpeg::ps_filter(std::move(stream)));
                    proxy = std::make_shared<pupnp::connection_proxy>(
                                std::move(filter),
                                protocol.data_rate());
                }
                else if (protocol.mux == "m2ts")
                {
                    std::unique_ptr<mpeg::m2ts_filter> filter(new mpeg::m2ts_filter(std::move(stream)));
                    proxy = std::make_shared<pupnp::connection_proxy>(
                                std::move(filter),
                                protocol.data_rate());
                }
                else
                {
                    proxy = std::make_shared<pupnp::connection_proxy>(
                                std::move(stream),
                                protocol.data_rate());
                }

                connection_manager.add_output_connection(proxy, protocol, item.mrl, source_address, opt.str());
                response = proxy;
            }
        }

        if (response)
        {
            content_type = protocol.content_format;
            return pupnp::upnp::http_ok;
        }
    }

    return pupnp::upnp::http_not_found;
}
