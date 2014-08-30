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
#include "platform/string.h"
#include <algorithm>
#include <sstream>

static const char base_css[] = {
#include "base.css.h"
}, main_css[] = {
#include "main.css.h"
}, main_svg[] = {
#include "main.svg.h"
}, running_svg[] = {
#include "running.svg.h"
}, vlc_icon_svg[] = {
#include "vlc-icon.svg.h"
};

namespace html {

mainpage::mainpage(class pupnp::upnp &upnp, class pupnp::connection_manager &connection_manager)
    : upnp(upnp),
      connection_manager(connection_manager)
{
    using namespace std::placeholders;

    upnp.http_callback_register("/", std::bind(&mainpage::handle_http_request, this, _1, _2, _3));
    upnp.http_callback_register("/css", std::bind(&mainpage::handle_http_request, this, _1, _2, _3));
    upnp.http_callback_register("/img", std::bind(&mainpage::handle_http_request, this, _1, _2, _3));

    add_file("/css/base.css", file { pupnp::upnp::mime_text_css, base_css, sizeof(base_css) });
    add_file("/css/main.css", file { pupnp::upnp::mime_text_css, main_css, sizeof(main_css) });
    add_file("/img/main.svg", file { pupnp::upnp::mime_image_svg, main_svg, sizeof(main_svg) });
    add_file("/img/running.svg", file { pupnp::upnp::mime_image_svg, running_svg, sizeof(running_svg) });
    add_file("/img/vlc-icon.svg", file { pupnp::upnp::mime_image_svg, vlc_icon_svg, sizeof(vlc_icon_svg) });

    page_order.push_back("/");
}

mainpage::~mainpage()
{
    upnp.http_callback_unregister("/");
}

void mainpage::set_devicename(const std::string &devicename)
{
    using namespace std::placeholders;

    this->devicename = devicename;

    add_page("/", page
    {
        devicename,
        "/img/main.svg",
        std::bind(&mainpage::render_headers, this, _1, _2),
        std::bind(&mainpage::render_mainpage, this, _1, _2)
    });
}

void mainpage::add_page(const std::string &path, const struct page &page)
{
    pages[path] = page;

    if (std::find(page_order.begin(), page_order.end(), path) == page_order.end())
        page_order.push_back(path);
}

void mainpage::remove_page(const std::string &path)
{
    pages.erase(path);

    auto i = std::find(page_order.begin(), page_order.end(), path);
    if (i != page_order.end())
        page_order.erase(i);
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
           "<link rel=\"stylesheet\" href=\"/css/base.css\" type=\"text/css\" media=\"screen, handheld, projection\" />";

    if (page.render_headers)
        page.render_headers(request, out);

    out << "</head>"
           "<body>"
           "<div class=\"main_navigator\">"
           "<div class=\"root\">" << page.title << "</div>";

    for (const auto &i : page_order)
    {
        auto page = pages.find(i);
        if (page != pages.end())
        {
            out << "<div><a href=\"" << page->first << "\">"
                   "<img src=\"" << page->second.icon << "\" alt=\"" << page->second.title << "\" />"
                   "</a></div>";
        }
    }

    out << "</div>\n";

    const int result = page.render_content(request, out);

    out << "</body></html>";

    return result;
}

void mainpage::render_headers(const struct pupnp::upnp::request &request, std::ostream &out)
{
    out << "<link rel=\"stylesheet\" href=\"/css/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />"
           "<meta http-equiv=\"Refresh\" content=\"10; url=http://" << request.url.host << request.url.path << "\" />";
}

int mainpage::render_mainpage(const struct pupnp::upnp::request &, std::ostream &out)
{
    out << "<div class=\"buttons\">";

    for (const auto &i : page_order)
        if (i != "/")
        {
            auto page = pages.find(i);
            if (page != pages.end())
            {
                out << "<div class=\"button\"><a href=\"" << page->first << "\">"
                       "<img src=\"" << page->second.icon << "\" alt=\"" << page->second.title << "\" />"
                       "<div class=\"title\">" << page->second.title << "</div></a></div>";
            }
        }

    const auto output_connections = connection_manager.output_connections();
    if (!output_connections.empty())
    {
        out << "</div><div class=\"streams\"><div class=\"stream_list\"><div>";

        for (const auto &i : output_connections)
        {
            std::string title = from_percent(i.mrl);
            if (starts_with(title, "file:"))
            {
                const size_t ls = title.find_last_of("/\\");
                if (ls != title.npos)
                    title = title.substr(ls + 1);
            }

            out << "<div class=\"stream\"><img src=\"/img/running.svg\">"
                   "<p class=\"stream_title\">" << title << "</p>"
                   "<p class=\"stream_dir\">";

            switch (i.direction)
            {
            case pupnp::connection_manager::connection_info::input  : out << "&larr;"; break;
            case pupnp::connection_manager::connection_info::output : out << "&rarr;"; break;
            }

            out << "</p><p class=\"stream_dest\">" << i.endpoint << "</p>"
                   "</div>";
        }

        out << "</div></div>";
    }

    out << "</div><div class=\"footer\"><div class=\"tiles\">"
           "<div><img src=\"/img/vlc-icon.svg\" /><p><a href=\"http://www.videolan.org/vlc/\">Powered by VLC</a></p></div>"
           "</div><div class=\"copyright\">"
           "<p>Copyright &copy; 2014 A.J. Admiraal</p><p>This program is free software: you can redistribute it and/or modify "
           "it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation.</p>"
           "</div></div>";

    return pupnp::upnp::http_ok;
}

}
