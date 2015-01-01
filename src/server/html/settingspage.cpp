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
#include "server/settings.h"
#include "platform/path.h"
#include "platform/string.h"
#include "platform/translator.h"
#include <algorithm>

static const char settings_css[] = {
#include "settings.css.h"
}, settings_svg[] = {
#include "settings.svg.h"
};

namespace html {

settingspage::settingspage(
        class mainpage &mainpage,
        class settings &settings,
        const std::function<bool()> &apply)
    : mainpage(mainpage),
      settings(settings),
      apply(apply),
      applying(false)
{
    using namespace std::placeholders;

    mainpage.add_file("/css/settings.css", mainpage::file { pupnp::upnp::mime_text_css_utf8, settings_css, sizeof(settings_css) });
    mainpage.add_file("/img/settings.svg", mainpage::file { pupnp::upnp::mime_image_svg, settings_svg, sizeof(settings_svg) });

    mainpage.add_page("/settings", mainpage::page
    {
        tr("Settings"),
        "/img/settings.svg",
        std::bind(&settingspage::render_headers, this, _1, _2),
        std::bind(&settingspage::render_page, this, _1, _2)
    });
}

settingspage::~settingspage()
{
    mainpage.remove_page("/settings");
}

static const char * is_enabled(bool enabled)
{
    return enabled ? "" : " disabled=\"disabled\"";
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
    out << "<fieldset>"
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
           "<input type=\"checkbox\" name=\"bind_all_networks\" value=\"on\"" << is_checked(settings.bind_all_networks()) << " />"
           << tr("Bind all networks") << "</p>"
           "<p>" << tr("Server name") << ": "
           "<input type=\"text\" size=\"40\" name=\"upnp_devicename\" value=\"" << escape_xml(settings.upnp_devicename()) << "\" /></p>"
           "<p><input type=\"checkbox\" name=\"republish_rootdevice\" value=\"on\"" << is_checked(settings.republish_rootdevice()) << " />"
           << tr("Regularly reinitialize the server while inactive to force all clients "
                 "to update their device lists.") << "</p>"
           "<p class=\"buttons\"><input type=\"submit\" name=\"save_button\" value=\"" << tr("Save") << "\" /></p>"
           "</form></fieldset>";
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

    auto bind_all_networks = query.find("bind_all_networks");
    settings.set_bind_all_networks(bind_all_networks != query.end());

    auto devicename = query.find("upnp_devicename");
    if (devicename != query.end())
        settings.set_upnp_devicename(from_percent(devicename->second));

    auto republish_rootdevice = query.find("republish_rootdevice");
    settings.set_republish_rootdevice(republish_rootdevice != query.end());
}

static void render_dlna_settings(const class settings &settings, std::ostream &out)
{
    out << "<fieldset>"
           "<legend>" << tr("Encoding") << "</legend>"
           "<p>" << tr("This form will allow adjustment of the encoding settings for the DLNA "
                       "clients. Note that higher settings require more CPU power and more "
                       "network bandwidth.") << "</p>"
           "<form name=\"dlnasettings\" action=\"/settings\" method=\"get\">"
           "<input type=\"hidden\" name=\"save_settings\" value=\"dlna\" />"
           "<p>" << tr("Video settings") << ":</p>"
           "<p><select name=\"encode_mode\">"
           "<option value=\"slow\""                 << is_selected(settings.encode_mode() == encode_mode::slow      ) << ">" << tr("Slow; High-quality") << "</option>"
           "<option value=\"fast\""                 << is_selected(settings.encode_mode() == encode_mode::fast      ) << ">" << tr("Fast; Lower-quality") << "</option>"
           "</select><select name=\"video_mode\">"
           "<option value=\"auto\""                 << is_selected(settings.video_mode() == video_mode::auto_       ) << ">" << tr("Automatic") << "</option>"
           "<option value=\"vcd\""                  << is_selected(settings.video_mode() == video_mode::vcd         ) << ">" << tr("Video-CD; 288p") << "</option>"
           "<option value=\"dvd\""                  << is_selected(settings.video_mode() == video_mode::dvd         ) << ">" << tr("DVD; 576p") << "</option>"
           "<option value=\"hdtv_720\""             << is_selected(settings.video_mode() == video_mode::hdtv_720    ) << ">" << tr("HDTV; 720p") << "</option>"
           "<option value=\"hdtv_1080\""            << is_selected(settings.video_mode() == video_mode::hdtv_1080   ) << ">" << tr("HDTV; 1080p") << "</option>"
           "</select><select name=\"canvas_mode\""  << is_enabled(settings.canvas_mode_enabled()) << ">"
           "<option value=\"none\""                 << is_selected(settings.canvas_mode() == canvas_mode::none      ) << ">" << tr("None") << "</option>"
           "<option value=\"pad\""                  << is_selected(settings.canvas_mode() == canvas_mode::pad       ) << ">" << tr("Add black bars") << "</option>"
           "<option value=\"crop\""                 << is_selected(settings.canvas_mode() == canvas_mode::crop      ) << ">" << tr("Crop video") << "</option>"
           "</select></p>";

    if (!settings.canvas_mode_enabled())
    {
        out << "<p class=\"bug\">Adding black bars or cropping video is disabled due to a "
               "<a href=\"https://trac.videolan.org/vlc/ticket/10148\">bug</a> in the current VLC version.</p>";
    }

    out << "<p>" << tr("Audio settings") << ":</p>"
           "<p><select name=\"surround_mode\"" << is_enabled(settings.surround_mode_enabled()) << ">"
           "<option value=\"stereo\""       << is_selected(settings.surround_mode() == surround_mode::stereo    ) << ">" << tr("Stereo") << "</option>"
           "<option value=\"surround51\""   << is_selected(settings.surround_mode() == surround_mode::surround51) << ">" << tr("5.1 surround") << "</option>"
           "</select></p>";

    if (!settings.surround_mode_enabled())
    {
        out << "<p class=\"bug\">Surround audio encoding is disabled due to a "
               "<a href=\"https://trac.videolan.org/vlc/ticket/1897\">bug</a> in the current VLC version.</p>";
    }

    out << "<p>" << tr("Subtitle settings") << ":</p>"
           "<p><select name=\"font_size\">"
           "<option value=\"small\""                << is_selected(settings.font_size() == font_size::small ) << ">" << tr("Small font") << "</option>"
           "<option value=\"normal\""               << is_selected(settings.font_size() == font_size::normal) << ">" << tr("Normal font") << "</option>"
           "<option value=\"large\""                << is_selected(settings.font_size() == font_size::large ) << ">" << tr("Large font") << "</option>"
           "</select></p>";

    out << "<p class=\"advanced\" id=\"dlna_expand\"><input type=\"button\" value=\"" << tr("Advanced") << "\" onclick=\""
               "document.getElementById('dlna_details').style.display='';"
               "document.getElementById('dlna_expand').style.display='none';\" /></p>"
           "<div id=\"dlna_details\" style=\"display:none;\">"
           "<p>" << tr("Codecs and formats") << ":</p>"
           "<table><tr><td>"
           "<p><input type=\"checkbox\" name=\"mpeg2\" value=\"on\"" << is_checked(settings.mpeg2_enabled()) << " />MPEG 1 / MPEG 2</p>"
           "<p><input type=\"checkbox\" name=\"mpeg4\" value=\"on\"" << is_checked(settings.mpeg4_enabled()) << " />MPEG 4 AVC (h264)</p>"
           "</td><td>"
           "<p><input type=\"checkbox\" name=\"video_mpegm2ts\" value=\"on\"" << is_checked(settings.video_mpegm2ts_enabled()) << " />" << pupnp::upnp::mime_video_mpegm2ts << "</p>"
           "<p><input type=\"checkbox\" name=\"video_mpegts\" value=\"on\"" << is_checked(settings.video_mpegts_enabled()) << " />" << pupnp::upnp::mime_video_mpegts << "</p>"
           "<p><input type=\"checkbox\" name=\"video_mpeg\" value=\"on\"" << is_checked(settings.video_mpeg_enabled()) << " />" << pupnp::upnp::mime_video_mpeg << "</p>"
           "</td></tr></table>"
           "<p>" << tr("Debug") << ":</p>"
           "<p><input type=\"checkbox\" name=\"verbose_logging\" value=\"on\"" << is_checked(settings.verbose_logging_enabled()) << " />"
            << tr("Enable verbose logging for analyzing problems.") << "</p>"
           "</div>";

    out << "<p class=\"buttons\"><input type=\"submit\" name=\"save_button\" value=\"" << tr("Save") << "\" /></p>"
           "</form></fieldset>";
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

    auto font_size = query.find("font_size");
    if (font_size != query.end())
    {
        if      (font_size->second == "small"       ) settings.set_font_size(::font_size::small         );
        else if (font_size->second == "normal"      ) settings.set_font_size(::font_size::normal        );
        else if (font_size->second == "large"       ) settings.set_font_size(::font_size::large         );
    }

    settings.set_mpeg2_enabled(query.find("mpeg2") != query.end());
    settings.set_mpeg4_enabled(query.find("mpeg4") != query.end());
    settings.set_video_mpegm2ts_enabled(query.find("video_mpegm2ts") != query.end());
    settings.set_video_mpegts_enabled(query.find("video_mpegts") != query.end());
    settings.set_video_mpeg_enabled(query.find("video_mpeg") != query.end());

    settings.set_verbose_logging_enabled(query.find("verbose_logging") != query.end());
}

static std::string format_path(const std::string &src)
{
#if defined(__unix__) || defined(__APPLE__)
    return src;
#elif defined(WIN32)
    std::string dst = src;
    std::replace(dst.begin(), dst.end(), '/', '\\');
    return dst;
#endif
}

static void render_path_box(const std::string &full_path, const std::string &current, size_t index, std::ostream &out)
{
    const std::vector<std::string> items = full_path.empty() ? platform::list_root_directories() : platform::list_files(full_path, true);
    if (!items.empty())
    {
        out << "<select name=\"append_path_" << index << "\" onchange=\"if(this.value != 0) { this.form.submit(); }\">";

        if (current.empty() || (std::find(items.begin(), items.end(), current) == items.end()))
        {
            out << "<option value=\"\" disabled=\"disabled\" selected=\"selected\" style=\"display:none;\">"
                << tr("Choose") << "</option>";
        }

        for (auto &i : items)
        {
            std::string pct = to_percent(i);
            std::replace(pct.begin(), pct.end(), '%', '_');
            out << "<option value=\"" << pct << "\"" << is_selected(current == i) << ">";
#ifdef WIN32
            if (full_path.empty())
            {
                const std::string name = platform::volume_name(i);
                if (!name.empty())
                    out << escape_xml(name) << " (" << escape_xml(format_path(i)) << ")";
                else
                    out << escape_xml(format_path(i));
            }
            else
#endif
                out << escape_xml(format_path(i));

            out << "</option>";
        }
        out << "</select>";
    }
}

static void render_path_settings(const std::map<std::string, std::string> &query, const class settings &settings, std::ostream &out)
{
    out << "<fieldset>"
           "<legend>" << tr("Folders") << "</legend>"
           "<form name=\"pathsettings\" action=\"/settings\" method=\"get\">"
           "<input type=\"hidden\" name=\"save_settings\" value=\"path\" />"
           "<p><input type=\"checkbox\" name=\"share_removable_media\" value=\"on\"" << is_checked(settings.share_removable_media()) << " />"
           << tr("Share removable media.") << "</p>"
           "<table>";

    const auto paths = settings.root_paths();
    for (size_t i = 0, n = paths.size(); i < n; i++)
    {
        out << "<tr><td><input type=\"text\" size=\"50\" name=\"path_" << i << "\" disabled=\"disabled\" "
               "value=\"" << escape_xml(format_path(paths[i].path)) << "\" /></td>";
        out << "<td class=\"right\"><select name=\"path_type_" << i << "\">"
               "<option value=\"auto\""     << is_selected(paths[i].type == path_type::auto_    ) << ">" << tr("Automatic") << "</option>"
               "<option value=\"music\""    << is_selected(paths[i].type == path_type::music    ) << ">" << tr("Music") << "</option>"
               "<option value=\"pictures\"" << is_selected(paths[i].type == path_type::pictures ) << ">" << tr("Pictures") << "</option>"
               "<option value=\"videos\""   << is_selected(paths[i].type == path_type::videos   ) << ">" << tr("Videos") << "</option>"
               "</select><input type=\"submit\" name=\"remove_" << i << "\" value=\"" << tr("Remove") << "\" /></td></tr>";
    }

    out << "</table><p>" << tr("Add more paths with the boxes below") << ":</p><table><tr><td>";

    std::vector<std::string> path;
    for (size_t i = 0;; i++)
    {
        auto path_type = query.find("append_path_" + std::to_string(i));
        if ((path_type != query.end()) && (query.find("append") == query.end()))
        {
            std::string pct = path_type->second;
            std::replace(pct.begin(), pct.end(), '_', '%');
            path.emplace_back(from_percent(pct));
        }
        else
            break;
    }

#if defined(__unix__) || defined(__APPLE__)
    std::string full_path = "/";
#elif defined(WIN32)
    std::string full_path;
#endif
    for (size_t i = 0; i < path.size(); i++)
    {
        render_path_box(full_path, path[i], i, out);
        full_path += path[i];
    }

    render_path_box(full_path, std::string(), path.size(), out);

    out << "</td><td class=\"right\"><input type=\"submit\"" << is_enabled(!path.empty()) << " name=\"append\" "
           "value=\"" << tr("Append") << "\" /></td></tr></table>"
           "<p class=\"buttons\"><input type=\"submit\" name=\"save_button\" value=\"" << tr("Save") << "\" /></p>"
           "</form></fieldset>";
}

static void save_path_settings(class settings &settings, const std::map<std::string, std::string> &query)
{
    settings.set_share_removable_media(query.find("share_removable_media") != query.end());

    auto paths = settings.root_paths();
    bool dirty = false;

    for (size_t i = 0, n = paths.size(); i < n; i++)
    {
        auto path_type = query.find("path_type_" + std::to_string(i));
        if (path_type != query.end())
        {
            if      (path_type->second == "auto"    ) paths[i].type = ::path_type::auto_;
            else if (path_type->second == "music"   ) paths[i].type = ::path_type::music;
            else if (path_type->second == "pictures") paths[i].type = ::path_type::pictures;
            else if (path_type->second == "videos"  ) paths[i].type = ::path_type::videos;

            dirty = true;
        }
    }

    for (size_t i = 0, n = paths.size(); i < n; i++)
    {
        auto remove = query.find("remove_" + std::to_string(i));
        if (remove != query.end())
        {
            paths.erase(paths.begin() + i);
            dirty = true;
            break;
        }
    }

    auto append = query.find("append");
    if (append != query.end())
    {
#if defined(__unix__) || defined(__APPLE__)
        std::string full_path = "/";
#elif defined(WIN32)
        std::string full_path;
#endif
        for (size_t i = 0;; i++)
        {
            auto path_type = query.find("append_path_" + std::to_string(i));
            if (path_type != query.end())
            {
                std::string pct = path_type->second;
                std::replace(pct.begin(), pct.end(), '_', '%');
                full_path += from_percent(pct);
            }
            else if (i > 0)
            {
                paths.emplace_back(root_path { ::path_type::auto_, full_path });
                dirty = true;
                break;
            }
            else
                break;
        }
    }

    if (dirty)
        settings.set_root_paths(paths);
}

void settingspage::render_headers(const struct pupnp::upnp::request &request, std::ostream &out)
{
    const uint16_t old_port = settings.http_port();

    out << "<link rel=\"stylesheet\" href=\"/css/settings.css\" type=\"text/css\" media=\"screen, handheld, projection\" />";

    const auto save_settings = request.url.query.find("save_settings");
    if (save_settings != request.url.query.end())
    {
        if      (save_settings->second == "http") save_http_settings(settings, request.url.query);
        else if (save_settings->second == "dlna") save_dlna_settings(settings, request.url.query);
        else if (save_settings->second == "path") save_path_settings(settings, request.url.query);
    }

    if ((request.url.query.find("save_button") != request.url.query.end()) && apply)
    {
        applying = apply();
        if (!applying)
            settings.set_http_port(old_port);

        std::string host = request.url.host;

        uint16_t used_port = old_port;
        const size_t colon = host.find_last_of(':', host.find_first_of(']'));
        if (colon != host.npos)
        {
            try { used_port = uint16_t(std::stoi(host.substr(colon + 1))); }
            catch (const std::invalid_argument &) { }
            catch (const std::out_of_range &) { }
            host = host.substr(0, colon);
        }
        else
            used_port = 80;

        if (used_port == old_port)
            host += ':' + std::to_string(settings.http_port());
        else
            host = request.url.host;

        out << "<meta http-equiv=\"Refresh\" content=\"3; url=http://" << host << request.url.path << "\" />";
    }
}

int settingspage::render_page(const struct pupnp::upnp::request &request, std::ostream &out)
{
    render_http_settings(settings, out);
    render_dlna_settings(settings, out);
    render_path_settings(request.url.query, settings, out);

    if (applying)
    {
        out << "<div class=\"wait\"><div><p>"
               "<img src=\"/img/running.svg\" />"
               << tr("Applying settings...") <<
               "</p></div></div>";
    }
    else if ((request.url.query.find("save_button") != request.url.query.end()) && apply)
    {
        out << "<div class=\"error\"><div><p>"
               << tr("Cannot apply settings while streaming.") <<
               "</p></div></div>";
    }

    return pupnp::upnp::http_ok;
}

}
