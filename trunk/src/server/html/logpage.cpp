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

#include "logpage.h"
#include "platform/fstream.h"
#include "platform/string.h"
#include "platform/translator.h"

static const char log_css[] = {
#include "log.css.h"
}, log_svg[] = {
#include "log.svg.h"
};

namespace html {

logpage::logpage(class mainpage &mainpage, const std::string &logfilename)
    : mainpage(mainpage),
      logfilename(logfilename)
{
    using namespace std::placeholders;

    if (!logfilename.empty())
    {
        mainpage.add_file("/css/log.css", mainpage::file { pupnp::upnp::mime_text_css, log_css, sizeof(log_css) });
        mainpage.add_file("/img/log.svg", mainpage::file { pupnp::upnp::mime_image_svg, log_svg, sizeof(log_svg) });

        mainpage.add_page("/log", mainpage::page
        {
            tr("Log"),
            "/img/log.svg",
            std::bind(&logpage::render_headers, this, _1, _2),
            std::bind(&logpage::render_page, this, _1, _2)
        });
    }
}

logpage::~logpage()
{
    mainpage.remove_page("/log");
}

void logpage::render_headers(const struct pupnp::upnp::request &, std::ostream &out)
{
    out << "<link rel=\"stylesheet\" href=\"/css/log.css\" type=\"text/css\" media=\"screen, handheld, projection\" />";
}

int logpage::render_page(const struct pupnp::upnp::request &, std::ostream &out)
{
    platform::ifstream logfile(logfilename);
    if (logfile.is_open())
    {
        out << "<p class=\"filename\">" << escape_xml(logfilename) << ":</p>";
        for (std::string line; std::getline(logfile, line); )
            out << "<p>" << escape_xml(line) << "</p>";

        return pupnp::upnp::http_ok;
    }

    return pupnp::upnp::http_not_found;
}

}
