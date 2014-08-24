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

#include "settingspage.h"
#include "../settings.h"
#include "../string.h"
#include "../translator.h"

static const char settings_css[] = {
#include "settings.css.h"
}, settings_svg[] = {
#include "settings.svg.h"
};

namespace html {

settingspage::settingspage(class mainpage &mainpage, class settings &settings)
    : mainpage(mainpage),
      settings(settings)
{
    using namespace std::placeholders;

    mainpage.add_file("/css/settings.css", mainpage::file { pupnp::upnp::mime_text_css, settings_css });
    mainpage.add_file("/img/settings.svg", mainpage::file { pupnp::upnp::mime_image_svg, settings_svg });

    mainpage.add_page("/settings", mainpage::page
    {
        tr("Settings"),
        "/css/settings.css",
        "/img/settings.svg",
        std::bind(&settingspage::render_page, this, _1, _2)
    });
}

settingspage::~settingspage()
{
    mainpage.remove_page("/settings");
}

static const char * is_enabled(bool enabled)
{
    return enabled ? "" : " disabled";
}

static const char * is_checked(bool checked)
{
    return checked ? " checked=\"checked\"" : "";
}

static const char * is_selected(bool selected)
{
    return selected ? " selected=\"selected\"" : "";
}

static void render_http_settings(const class settings &settings, std::ostream &out)
{
    out << "<fieldset id=\"httpserver\">"
           "<legend>" << tr("Server") << "</legend>"
           "<p>" << tr("This configures the internal server. By default, the server only binds "
                       "local/private networks (i.e. 10.0.0.0/8, 127.0.0.0/8, 169.254.0.0/16, "
                       "172.16.0.0/12, and 192.168.0.0/16), all other networks are not bound. "
                       "This can be overridden by checking the \"Bind all networks\" option, "
                       "but note that this might expose this server to the internet; so before "
                       "enabling \"Bind all networks\" make sure the local router/firewall is "
                       "properly configured.") << "</p>"
           "<form name=\"httpsettings\" action=\"/settings\" method=\"get\">"
           "<input type=\"hidden\" name=\"save_settings\" value=\"http\" />"
           "<p>" << tr("Preferred HTTP port number") << ": "
           "<input type=\"text\" size=\"6\" name=\"http_port\" value=\"" << settings.http_port() << "\" />"
           "<input type=\"checkbox\" name=\"bindallnetworks\" value=\"on\"" << is_checked(settings.bindallnetworks()) << " />"
           << tr("Bind all networks") << "</p>"
           "<p>" << tr("Device name") << ": "
           "<input type=\"text\" size=\"40\" name=\"devicename\" value=\"" << escape_xml(settings.devicename()) << "\" /></p>"
//           "<p><input type=\"checkbox\" name=\"allowshutdown\" value=\"on\" {ALLOWSHUTDOWN} />{TR_ALLOW_SHUTDOWN}</p>"
//           "<p><input type=\"checkbox\" name=\"republishrootdevice\" value=\"on\" {REPUBLISHROOTDEVICE} />{TR_REPUBLISH_ROOTDEVICE}</p>"
           "<p class=\"buttons\"><input type=\"submit\" name=\"save\" value=\"" << tr("Save") << "\" /></p>"
           "</form>\n"
           "</fieldset>\n";
}

static void save_http_settings(class settings &settings, const std::map<std::string, std::string> &query)
{
    auto http_port = query.find("http_port");
    if (http_port != query.end())
    {
        try { settings.set_http_port(uint16_t(std::stoi(http_port->second))); }
        catch (const std::invalid_argument &) { }
        catch (const std::out_of_range &) { }
    }

    auto bindallnetworks = query.find("bindallnetworks");
    settings.set_bindallnetworks(bindallnetworks != query.end());

    auto devicename = query.find("devicename");
    if (devicename != query.end())
        settings.set_devicename(from_percent(devicename->second));
}

static void render_dlna_settings(const class settings &settings, std::ostream &out)
{
    out << "<fieldset id=\"httpserver\">"
           "<legend>" << tr("DLNA") << "</legend>"
           "<p>" << tr("This form will allow adjustment of the transcode settings for the DLNA "
                       "clients. Note that higher settings require more CPU power and more "
                       "network bandwidth.") << "</p>"
           "<form name=\"dlnasettings\" action=\"/settings\" method=\"get\">"
           "<input type=\"hidden\" name=\"save_settings\" value=\"dlna\" />"
           "<p>" << tr("Video settings") << ":</p>"
           "<p><select name=\"encode_mode\">\n"
           "<option value=\"slow\""         << is_selected(settings.encode_mode() == encode_mode::slow      ) << ">" << tr("Slow; High-quality") << "</option>"
           "<option value=\"fast\""         << is_selected(settings.encode_mode() == encode_mode::fast      ) << ">" << tr("Fast; Lower-quality") << "</option>"
           "</select><select name=\"video_mode\">\n"
           "<option value=\"auto\""         << is_selected(settings.video_mode() == video_mode::auto_       ) << ">" << tr("Automatic") << "</option>"
           "<option value=\"vcd\""          << is_selected(settings.video_mode() == video_mode::vcd         ) << ">" << tr("Video-CD; 288p") << "</option>"
           "<option value=\"dvd\""          << is_selected(settings.video_mode() == video_mode::dvd         ) << ">" << tr("DVD; 576p") << "</option>"
           "<option value=\"hdtv_720\""     << is_selected(settings.video_mode() == video_mode::hdtv_720    ) << ">" << tr("HDTV; 720p") << "</option>"
           "<option value=\"hdtv_1080\""    << is_selected(settings.video_mode() == video_mode::hdtv_1080   ) << ">" << tr("HDTV; 1080p") << "</option>"
           "</select><select name=\"canvas_mode\"" << is_enabled(settings.canvas_mode_enabled()) << ">\n"
           "<option value=\"none\""         << is_selected(settings.canvas_mode() == canvas_mode::none      ) << ">" << tr("None") << "</option>"
           "<option value=\"pad\""          << is_selected(settings.canvas_mode() == canvas_mode::pad       ) << ">" << tr("Add black bars") << "</option>"
           "<option value=\"crop\""         << is_selected(settings.canvas_mode() == canvas_mode::crop      ) << ">" << tr("Crop video") << "</option>"
           "</select></p>\n"
           "<p>" << tr("Audio settings") << ":</p>"
           "<p><select name=\"surround_mode\"" << is_enabled(settings.surround_mode_enabled()) << ">\n"
           "<option value=\"stereo\""       << is_selected(settings.surround_mode() == surround_mode::stereo    ) << ">" << tr("Stereo") << "</option>"
           "<option value=\"surround51\""   << is_selected(settings.surround_mode() == surround_mode::surround51) << ">" << tr("5.1 surround") << "</option>"
           "</select></p>\n"
           "<p class=\"buttons\"><input type=\"submit\" name=\"save\" value=\"" << tr("Save") << "\" /></p>"
           "</form>\n"
           "</fieldset>\n";
}

static void save_dlna_settings(class settings &settings, const std::map<std::string, std::string> &query)
{
    auto encode_mode = query.find("encode_mode");
    if (encode_mode != query.end())
    {
        if      (encode_mode->second == "slow") settings.set_encode_mode(::encode_mode::slow);
        else if (encode_mode->second == "fast") settings.set_encode_mode(::encode_mode::fast);
    }

    auto video_mode = query.find("video_mode");
    if (video_mode != query.end())
    {
        if      (video_mode->second == "auto"       ) settings.set_video_mode(::video_mode::auto_       );
        else if (video_mode->second == "vcd"        ) settings.set_video_mode(::video_mode::vcd         );
        else if (video_mode->second == "dvd"        ) settings.set_video_mode(::video_mode::dvd         );
        else if (video_mode->second == "hdtv_720"   ) settings.set_video_mode(::video_mode::hdtv_720    );
        else if (video_mode->second == "hdtv_1080"  ) settings.set_video_mode(::video_mode::hdtv_1080   );
    }

    auto canvas_mode = query.find("canvas_mode");
    if (canvas_mode != query.end())
    {
        if      (canvas_mode->second == "none"      ) settings.set_canvas_mode(::canvas_mode::none      );
        else if (canvas_mode->second == "pad"       ) settings.set_canvas_mode(::canvas_mode::pad       );
        else if (canvas_mode->second == "crop"      ) settings.set_canvas_mode(::canvas_mode::crop      );
    }

    auto surround_mode = query.find("surround_mode");
    if (surround_mode != query.end())
    {
        if      (surround_mode->second == "stereo"      ) settings.set_surround_mode(::surround_mode::stereo      );
        else if (surround_mode->second == "surround51"  ) settings.set_surround_mode(::surround_mode::surround51  );
    }
}

int settingspage::render_page(const struct pupnp::upnp::request &request, std::ostream &out)
{
    const auto save = request.url.query.find("save_settings");
    if (save != request.url.query.end())
    {
        if      (save->second == "http") save_http_settings(settings, request.url.query);
        else if (save->second == "dlna") save_dlna_settings(settings, request.url.query);
    }

    render_http_settings(settings, out);
    render_dlna_settings(settings, out);

    return pupnp::upnp::http_ok;
}

}
