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

#include "setuppage.h"
#include "resources/resources.h"
#include "server/settings.h"
#include "server/server.h"
#include "server/test.h"
#include "platform/fstream.h"
#include "platform/path.h"
#include "platform/string.h"
#include "platform/translator.h"

static const char setup_css[] = {
#include "setup.css.h"
}, video_display_svg[] = {
#include "video-display.svg.h"
};

namespace html {

enum setup_mode setuppage::current_setup_mode = html::setup_mode::disabled;
std::string setuppage::device_type;
std::string setuppage::client_name;

setuppage::setuppage(
        class mainpage &mainpage,
        class settings &settings,
        const std::unique_ptr<class test> &test,
        const std::function<void()> &apply)
    : mainpage(mainpage),
      settings(settings),
      test(test),
      apply(apply),
      applying(false)
{
    using namespace std::placeholders;

    mainpage.add_file("/css/setup.css", mainpage::file { pupnp::upnp::mime_text_css_utf8, setup_css, sizeof(setup_css) });
    mainpage.add_file("/img/video-display.svg", mainpage::file { pupnp::upnp::mime_image_svg, video_display_svg, sizeof(video_display_svg) });
    mainpage.add_file("/img/pm5544.png", mainpage::bin_file { pupnp::upnp::mime_image_png, resources::pm5544_png, sizeof(resources::pm5544_png) });
    mainpage.add_file("/img/pm5644.png", mainpage::bin_file { pupnp::upnp::mime_image_png, resources::pm5644_png, sizeof(resources::pm5644_png) });

    mainpage.add_page("/setup", mainpage::page
    {
        tr("Setup"),
        std::string(),
        std::bind(&setuppage::render_headers, this, _1, _2),
        std::bind(&setuppage::render_page, this, _1, _2)
    });
}

setuppage::~setuppage()
{
    mainpage.remove_page("/setup");
}

enum setup_mode setuppage::setup_mode()
{
    return current_setup_mode;
}

void setuppage::activate_setup()
{
    if (current_setup_mode == html::setup_mode::disabled)
        current_setup_mode = html::setup_mode::name;
}

bool setuppage::setup_required()
{
    return current_setup_mode != html::setup_mode::disabled;
}

void setuppage::render_headers(const struct pupnp::upnp::request &request, std::ostream &out)
{
    out << "<link rel=\"stylesheet\" href=\"/css/setup.css\" type=\"text/css\" media=\"screen, handheld, projection\" />";

    if ((request.url.query.find("apply") != request.url.query.end()) && apply)
    {
        std::string path = request.url.path;
        switch (current_setup_mode)
        {
        case html::setup_mode::disabled:
        case html::setup_mode::name:
            settings.set_configure_required(true);
            {
                auto devicename = request.url.query.find("upnp_devicename");
                if (devicename != request.url.query.end())
                    settings.set_upnp_devicename(from_percent(devicename->second));

                auto device_type = request.url.query.find("device_type");
                if (device_type != request.url.query.end())
                    this->device_type = device_type->second;

                auto client_name = request.url.query.find("client_name");
                if (client_name != request.url.query.end())
                    this->client_name = from_percent(client_name->second);
            }
            settings.set_video_mode(video_mode::dvd);
            settings.set_mpeg2_enabled(true);
            settings.set_mpeg4_enabled(true);
            settings.set_video_mpeg_enabled(true);
            settings.set_video_mpegts_enabled(true);
            settings.set_video_mpegm2ts_enabled(true);
            current_setup_mode = html::setup_mode::network;
            break;

        case html::setup_mode::network:
            current_setup_mode = html::setup_mode::codecs;
            break;

        case html::setup_mode::codecs:
            {
                const bool mp2v_mpg = request.url.query.find("mp2v_mpg") != request.url.query.end();
                const bool mp2v_ts = request.url.query.find("mp2v_ts") != request.url.query.end();
                const bool mp2v_m2ts = request.url.query.find("mp2v_m2ts") != request.url.query.end();
                const bool h264_ts = request.url.query.find("h264_ts") != request.url.query.end();
                const bool h264_m2ts = request.url.query.find("h264_m2ts") != request.url.query.end();

                if (mp2v_mpg || mp2v_ts || mp2v_m2ts)
                {
                    settings.set_mpeg4_enabled(false);
                    settings.set_video_mpeg_enabled(mp2v_mpg);
                    settings.set_video_mpegts_enabled(mp2v_ts);
                    settings.set_video_mpegm2ts_enabled(mp2v_m2ts);
                    settings.set_video_mode(video_mode::auto_);
                    current_setup_mode = html::setup_mode::high_definition;
                }
                else if (h264_ts || h264_m2ts)
                {
                    settings.set_mpeg2_enabled(false);
                    settings.set_video_mpeg_enabled(false);
                    settings.set_video_mpegts_enabled(h264_ts);
                    settings.set_video_mpegm2ts_enabled(h264_m2ts);
                    settings.set_video_mode(video_mode::auto_);
                    current_setup_mode = html::setup_mode::high_definition;
                }
                else
                {
                    settings.set_mpeg2_enabled(true);
                    settings.set_mpeg4_enabled(false);
                    settings.set_video_mpeg_enabled(true);
                    settings.set_video_mpegts_enabled(true);
                    settings.set_video_mpegm2ts_enabled(true);
                    settings.set_video_mode(video_mode::dvd);
                    current_setup_mode = html::setup_mode::finish;
                }
            }
            break;

        case html::setup_mode::high_definition:
            if (request.url.query.find("hd_1080") != request.url.query.end())
                settings.set_video_mode(video_mode::hdtv_1080);
            else if (request.url.query.find("hd_720") != request.url.query.end())
                settings.set_video_mode(video_mode::hdtv_720);
            else
                settings.set_video_mode(video_mode::dvd);

            current_setup_mode = html::setup_mode::finish;
            break;

        case html::setup_mode::finish:
            settings.set_configure_required(false);
            current_setup_mode = html::setup_mode::disabled;
            path = "/";
            break;
        }

        applying = true;
        apply();
        out << "<meta http-equiv=\"Refresh\" content=\"3; url=http://" << request.url.host << path << "\" />";
    }
    else if ((request.url.query.find("next") != request.url.query.end()))
    {
        switch (current_setup_mode)
        {
        case html::setup_mode::disabled:          current_setup_mode = html::setup_mode::network;           break;
        case html::setup_mode::name:              current_setup_mode = html::setup_mode::network;           break;
        case html::setup_mode::network:           current_setup_mode = html::setup_mode::codecs;            break;
        case html::setup_mode::codecs:            current_setup_mode = html::setup_mode::high_definition;   break;
        case html::setup_mode::high_definition:   current_setup_mode = html::setup_mode::finish;            break;
        case html::setup_mode::finish:            current_setup_mode = html::setup_mode::disabled;          break;
        }
    }
    else switch (current_setup_mode)
    {
    case html::setup_mode::network:
        if (test && test->detected_clients().empty())
            out << "<meta http-equiv=\"Refresh\" content=\"2; url=http://" << request.url.host << request.url.path << "\" />";

        break;

    case html::setup_mode::disabled:
    case html::setup_mode::name:
    case html::setup_mode::codecs:
    case html::setup_mode::high_definition:
    case html::setup_mode::finish:
        break;
    }
}

int setuppage::render_page(const struct pupnp::upnp::request &request, std::ostream &out)
{
    if (applying)
    {
        switch (current_setup_mode)
        {
        case html::setup_mode::disabled:          render_setup_finish(request, out);          break;
        case html::setup_mode::name:                                                          break;
        case html::setup_mode::network:           render_setup_name(request, out);            break;
        case html::setup_mode::codecs:            render_setup_network(request, out);         break;
        case html::setup_mode::high_definition:   render_setup_codecs(request, out);          break;
        case html::setup_mode::finish:            render_setup_high_definition(request, out); break;
        }

        out << "<div class=\"wait\"><div><p>"
               "<img src=\"/img/running.svg\" />"
               << tr("Please wait...") <<
               "</p></div></div>";
    }
    else
    {
        switch (current_setup_mode)
        {
        case html::setup_mode::disabled:          render_setup_name(request, out);            break;
        case html::setup_mode::name:              render_setup_name(request, out);            break;
        case html::setup_mode::network:           render_setup_network(request, out);         break;
        case html::setup_mode::codecs:            render_setup_codecs(request, out);          break;
        case html::setup_mode::high_definition:   render_setup_high_definition(request, out); break;
        case html::setup_mode::finish:            render_setup_finish(request, out);          break;
        }
    }

    return pupnp::upnp::http_ok;
}

static std::string tr_device_name(const std::string &device_type)
{
    if      (device_type == "television")   return tr("television");
    else if (device_type == "game_console") return tr("game console");
    else if (device_type == "media_center") return tr("media center");
    else if (device_type == "phone")        return tr("smart phone or tablet");
    else                                    return tr("device");
}

void setuppage::render_setup_name(const struct pupnp::upnp::request &, std::ostream &out)
{
    out << "<p><img class=\"logo\" src=\"/img/settings.svg\" alt=\"Setup assistant\" /></p>"
           "<h1>" << tr("Setup assistant") << "</h1>"
           "<p>" << tr("The setup assistant wil guide you through the process of "
                       "setting up a device, such as a television, game console, "
                       "media center, etc., for use with LXiMediaServer.") << "</p>"
           "<form name=\"setup\" action=\"/setup\" method=\"get\">"
           "<input type=\"hidden\" name=\"setup\" value=\"name\" />"
           "<h2>" << tr("Device type") << "</h2>"
           "<p>" << tr("What kind of device do you want to set up with LXiMediaServer?") << "</p>"
           "<p><select name=\"device_type\">"
           "<option value=\"television\">" << tr_device_name("television") << "</option>"
           "<option value=\"game_console\">" << tr_device_name("game_console") << "</option>"
           "<option value=\"media_center\">" << tr_device_name("media_center") << "</option>"
           "<option value=\"phone\">" << tr_device_name("phone") << "</option>"
           "<option value=\"other\">" << tr_device_name("other") << "</option>"
           "</select></p>"
           "<h2>" << tr("Server name") << "</h2>"
           "<p>" << tr("Provide a name to identify this instance of LXiMediaServer:") << "</p>"
           "<p><input type=\"text\" size=\"40\" name=\"upnp_devicename\" value=\""
        << escape_xml(settings.upnp_devicename()) << "\" /></p>"
           "<p class=\"assist\"><input type=\"submit\" name=\"apply\" value=\""
        << tr("Next") << " &gt;&gt;&gt;\" /></p>"
           "</form></fieldset>";
}

void setuppage::render_setup_network(const struct pupnp::upnp::request &, std::ostream &out)
{
    const std::string device_name = tr_device_name(device_type);

    out << "<p><img class=\"logo\" src=\"/img/video-display.svg\" alt=\"Network\" /></p>"
           "<h1>" << tr("Connect your") << " " << device_name << "</h1>"
           "<p>" << tr("Connect the " + device_name + " you want to set up to your local network "
                       "and use the menus of the " + device_name + " to browse to the server named") << " \""
        << escape_xml(settings.upnp_devicename()) << "\".</p><p>"
        << tr("Please consult the manual of the " + device_name + " if you need more information.") << "</p>"
           "<form name=\"setup\" action=\"/setup\" method=\"get\">";

    if (test && !test->detected_clients().empty())
    {
        out << "<p class=\"assist\"><input type=\"submit\" name=\"next\" "
               "value=\"" << tr("Found it") << " &gt;&gt;&gt;\" /></p>";
    }
    else
        out << "<p class=\"assist\"><img src=\"/img/running.svg\" /> " << tr("Waiting...") << "</p>";

    out << "</form></fieldset>";
}

void setuppage::render_setup_codecs(const struct pupnp::upnp::request &, std::ostream &out)
{
    const std::string device_name = tr_device_name(device_type);

    out << "<p><img class=\"logo\" src=\"/img/pm5544.png\" alt=\"PM5544\" /></p>"
           "<h1>" << tr("Play files") << "</h1>"
           "<p>" << tr("Try to play the available files on your " + device_name + " and select "
                       "the files below that you were able to play.") << "</p><p>"
        << tr("Please consult the manual of the " + device_name + " if you need more information.") << "</p>"
           "<form name=\"setup\" action=\"/setup\" method=\"get\">"
           "<input type=\"hidden\" name=\"setup\" value=\"formats\" />"
           "<div class=\"codecs\"><table><tr><td>"
           "<p><input type=\"checkbox\" name=\"mp2v_mpg\" value=\"on\" />MPEG 2 Program Stream</p>"
           "<p><input type=\"checkbox\" name=\"mp2v_m2ts\" value=\"on\" />MPEG 2 BDAV Transport Stream</p>"
           "<p><input type=\"checkbox\" name=\"mp2v_ts\" value=\"on\" />MPEG 2 Transport Stream</p>"
           "<p><input type=\"checkbox\" name=\"h264_m2ts\" value=\"on\" />MPEG 4 AVC BDAV Transport Stream</p>"
           "<p><input type=\"checkbox\" name=\"h264_ts\" value=\"on\" />MPEG 4 AVC Transport Stream</p>"
           "</td></tr></table></div>"
           "<p class=\"assist\"><input type=\"submit\" name=\"apply\" value=\""
        << tr("Next") << " &gt;&gt;&gt;\" /></p>"
           "</form></fieldset>";
}

void setuppage::render_setup_high_definition(const struct pupnp::upnp::request &, std::ostream &out)
{
    const std::string device_name = tr_device_name(device_type);

    out << "<p><img class=\"logo\" src=\"/img/pm5644.png\" alt=\"PM5644\" /></p>"
           "<h1>" << tr("Play files") << "</h1>"
           "<p>" << tr("Try to play the available files on your " + device_name + " and select "
                       "the files below that you were able to play.") << "</p><p>"
        << tr("Please consult the manual of the " + device_name + " if you need more information.") << "</p>"
           "<form name=\"setup\" action=\"/setup\" method=\"get\">"
           "<input type=\"hidden\" name=\"setup\" value=\"formats\" />"
           "<div class=\"codecs\"><table><tr><td>"
           "<p><input type=\"checkbox\" name=\"hd_720\" value=\"on\" />High Definition 720p</p>"
           "<p><input type=\"checkbox\" name=\"hd_1080\" value=\"on\" />High Definition 1080p</p>"
           "</td></tr></table></div>"
           "<p class=\"assist\"><input type=\"submit\" name=\"apply\" value=\""
        << tr("Next") << " &gt;&gt;&gt;\" /></p>"
           "</form></fieldset>";
}

void setuppage::render_setup_finish(const struct pupnp::upnp::request &, std::ostream &out)
{
    const std::string device_name = tr_device_name(device_type);

    out << "<p><img class=\"logo\" src=\"/img/lximedia.svg\" alt=\"LXiMedia\" /></p>"
           "<h1>" << tr("Finished") << "</h1>"
           "<p>" << tr("Try to play the available files on your " + device_name + " and select "
                       "the files below that you were able to play.") << "</p><p>"
        << tr("Please consult the manual of the " + device_name + " if you need more information.") << "</p>"
           "<form name=\"setup\" action=\"/setup\" method=\"get\">"
           "<p class=\"assist\"><input type=\"submit\" name=\"apply\" value=\""
        << tr("Done") << "\" /></p>"
           "</form></fieldset>";
}

}
