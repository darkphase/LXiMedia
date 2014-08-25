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

#include "mainpage.h"
#include <sstream>

static const char base_css[] = {
#include "base.css.h"
}, main_css[] = {
#include "main.css.h"
}, main_svg[] = {
#include "main.svg.h"
};

namespace html {

mainpage::mainpage(class pupnp::upnp &upnp)
    : upnp(upnp)
{
    using namespace std::placeholders;

    upnp.http_callback_register("/", std::bind(&mainpage::handle_http_request, this, _1, _2, _3));
    upnp.http_callback_register("/css", std::bind(&mainpage::handle_http_request, this, _1, _2, _3));
    upnp.http_callback_register("/img", std::bind(&mainpage::handle_http_request, this, _1, _2, _3));

    add_file("/css/base.css", file { pupnp::upnp::mime_text_css, base_css, sizeof(base_css) });
    add_file("/css/main.css", file { pupnp::upnp::mime_text_css, main_css, sizeof(main_css) });
    add_file("/img/main.svg", file { pupnp::upnp::mime_image_svg, main_svg, sizeof(main_svg) });
}

mainpage::~mainpage()
{
    upnp.http_callback_unregister("/");
}

void mainpage::set_devicename(const std::string &devicename)
{
    using namespace std::placeholders;

    this->devicename = devicename;

    pages["/"] = page
    {
        devicename,
        "/css/main.css",
        "/img/main.svg",
        std::bind(&mainpage::render_mainpage, this, _1, _2)
    };
}

void mainpage::add_page(const std::string &path, const struct page &page)
{
    pages[path] = page;
}

void mainpage::remove_page(const std::string &path)
{
    pages.erase(path);
}

void mainpage::add_file(const std::string &path, const struct file &file)
{
    files[path] = file;
}

void mainpage::add_file(const std::string &path, const struct bin_file &file)
{
    files[path] = reinterpret_cast<const struct file &>(file);
}

int mainpage::handle_http_request(const struct pupnp::upnp::request &request, std::string &content_type, std::shared_ptr<std::istream> &response)
{
    auto page = pages.find(request.url.path);
    if (page != pages.end())
    {
        auto stream = std::make_shared<std::stringstream>();
        content_type = pupnp::upnp::mime_text_html;
        response = stream;
        return render_page(request, content_type, *stream, page->second);
    }

    auto file = files.find(request.url.path);
    if (file != files.end())
    {
        content_type = file->second.content_type;
        response = std::make_shared<std::stringstream>(std::string(file->second.data, file->second.size));
        return pupnp::upnp::http_ok;
    }

    return pupnp::upnp::http_not_found;
}

int mainpage::render_page(const struct pupnp::upnp::request &request, const std::string &content_type, std::ostream &out, const struct page &page)
{
    out << "<!DOCTYPE html>\n"
           "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">"
           "<head>"
           "<title>" << devicename << "</title>"
           "<meta http-equiv=\"Content-Type\" content=\"" << content_type << "\" />"
           "<link rel=\"stylesheet\" href=\"/css/base.css\" type=\"text/css\" media=\"screen, handheld, projection\" />"
           "<link rel=\"stylesheet\" href=\"" << page.stylesheet << "\" type=\"text/css\" media=\"screen, handheld, projection\" />"
           "</head>"
           "<body>"
           "<div class=\"main_navigator\">"
           "<div class=\"root\">" << page.title << "</div>";

    for (const auto &i : pages)
    {
        out << "<div><a href=\"" << i.first << "\">"
               "<img src=\"" << i.second.icon << "\" alt=\"" << i.second.title << "\" />"
               "</a></div>";
    }

    out << "</div>\n";

    const int result = page.render(request, out);

    out << "</body></html>";

    return result;
}

int mainpage::render_mainpage(const struct pupnp::upnp::request &, std::ostream &out)
{
    for (const auto &i : pages)
    {
        out << "<div class=\"button\"><a href=\"" << i.first << "\">"
               "<img src=\"" << i.second.icon << "\" alt=\"" << i.second.title << "\" />"
               "<div class=\"title\">" << i.second.title << "</div></a></div>";
    }

    return pupnp::upnp::http_ok;
}

}
