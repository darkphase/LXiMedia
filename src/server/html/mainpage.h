/******************************************************************************
 *   Copyright (C) 2015  A.J. Admiraal                                        *
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

#ifndef HTML_MAINPAGE_H
#define HTML_MAINPAGE_H

#include "pupnp/connection_manager.h"
#include "pupnp/upnp.h"
#include <functional>
#include <list>
#include <map>
#include <ostream>
#include <string>

class settings;

namespace html {

class mainpage
{
public:
    struct page
    {
        std::string title;
        std::string icon;
        std::function<void(const struct pupnp::upnp::request &, std::ostream &)> render_headers;
        std::function<int(const struct pupnp::upnp::request &, std::ostream &)> render_content;
    };

    struct file
    {
        const char *content_type;
        const char *data;
        size_t size;
    };

    struct bin_file
    {
        const char *content_type;
        const unsigned char *data;
        size_t size;
    };

public:
    mainpage(
            class platform::messageloop_ref &,
            class pupnp::upnp &,
            class pupnp::connection_manager &,
            class settings &);

    ~mainpage();

    void set_devicename(const std::string &);

    void add_page(const std::string &, const struct page &);
    void remove_page(const std::string &);
    void add_file(const std::string &, const struct file &);
    void add_file(const std::string &, const struct bin_file &);

private:
    int handle_http_request(const struct pupnp::upnp::request &, std::string &, std::shared_ptr<std::istream> &);

    int render_page(const struct pupnp::upnp::request &, const std::string &, std::ostream &, const struct page &);
    void render_headers(const struct pupnp::upnp::request &, std::ostream &);
    int render_mainpage(const struct pupnp::upnp::request &, std::ostream &);

private:
    class platform::messageloop_ref messageloop;
    class pupnp::upnp &upnp;
    class pupnp::connection_manager &connection_manager;
    class settings &settings;
    std::string devicename;

    std::map<std::string, page> pages;
    std::list<std::string> page_order;
    std::map<std::string, file> files;
};

} // End of namespace

#endif
