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

static void render_http_settings(const class settings &settings, std::ostream &out)
{
    out << "<fieldset id=\"httpserver\">"
           "<legend>" << tr("Server") << "</legend>"
           << tr("This configures the internal server. By default, the server only binds "
                 "local/private networks (i.e. 10.0.0.0/8, 127.0.0.0/8, 169.254.0.0/16, "
                 "172.16.0.0/12, and 192.168.0.0/16), all other networks are not bound. "
                 "This can be overridden by checking the \"Bind all networks\" option, "
                 "but note that this might expose this server to the internet; so before "
                 "enabling \"Bind all networks\" make sure the local router/firewall is "
                 "properly configured.") <<
           "<br /><br />"
           "<form name=\"httpsettings\" action=\"/settings\" method=\"get\">"
           "<input type=\"hidden\" name=\"save_settings\" value=\"http\" />"
           "<table>"
           "<tr><td>"
           << tr("Preferred HTTP port number") <<
           "<input type=\"text\" size=\"6\" name=\"http_port\" value=\"" << settings.http_port() << "\" />"
           "<input type=\"checkbox\" name=\"bindallnetworks\" value=\"on\"" << (settings.bindallnetworks() ? " checked=\"checked\"" : "") << " />"
           << tr("Bind all networks") <<
           "</td></tr><tr><td>"
           << tr("Device name") <<
           "<input type=\"text\" size=\"40\" name=\"devicename\" value=\"" << escape_xml(settings.devicename()) << "\" />"
//           "</td></tr><tr><td>"
//           "<input type=\"checkbox\" name=\"allowshutdown\" value=\"on\" {ALLOWSHUTDOWN} />{TR_ALLOW_SHUTDOWN}"
//           "</td></tr><tr><td>"
//           "<input type=\"checkbox\" name=\"republishrootdevice\" value=\"on\" {REPUBLISHROOTDEVICE} />{TR_REPUBLISH_ROOTDEVICE}"
           "</td></tr>"
           "</table>"
           "<br />"
           "<input type=\"submit\" name=\"save\" value=\"" << tr("Save") << "\" />"
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

int settingspage::render_page(const struct pupnp::upnp::request &request, std::ostream &out)
{
    const auto save = request.url.query.find("save_settings");
    if (save != request.url.query.end())
    {
        if (save->second == "http") save_http_settings(settings, request.url.query);
    }

    render_http_settings(settings, out);

    return pupnp::upnp::http_ok;
}

}
