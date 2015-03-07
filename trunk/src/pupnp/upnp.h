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

#ifndef PUPNP_UPNP_H
#define PUPNP_UPNP_H

#include "platform/messageloop.h"
#include <cstdint>
#include <functional>
#include <istream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>

namespace pupnp {

class upnp
{
public:
    struct child
    {
        virtual bool initialize() = 0;
        virtual void close() = 0;
    };

    struct url
    {
        url();
        url(const std::string &);
        operator std::string() const;

        std::string host;
        std::string path;
        std::map<std::string, std::string> query;
    };

    struct request
    {
        std::string user_agent;
        std::string source_address;
        struct url url;
    };

    typedef std::function<int(const request &, std::string &, std::shared_ptr<std::istream> &)> http_callback;

    static const int http_ok = 200;
    static const int http_no_content = 204;
    static const int http_not_found = 404;
    static const int http_internal_server_error = 500;

public:
    static std::string hostname();

    explicit upnp(class platform::messageloop_ref &);
    ~upnp();

    const std::string &http_basedir() const;

    void child_add(struct child &);
    void child_remove(struct child &);

    void http_callback_register(const std::string &path, const http_callback &);
    void http_callback_unregister(const std::string &path);

    bool initialize(uint16_t port, bool bind_public = false);
    void close(void);

    static bool is_private_address(const char *);
    static bool is_loopback_address(const char *);
    static bool is_auto_address(const char *);
    static bool is_public_address(const char *);
    bool is_my_address(const std::string &) const;
    const std::set<std::string> & bound_addresses() const;
    uint16_t bound_port() const;

    int handle_http_request(const struct request &, std::string &content_type, std::shared_ptr<std::istream> &);

private:
    void update_interfaces();
    void clear_responses();
    void enable_webserver();
    int get_response(const struct request &, std::string &, std::shared_ptr<std::istream> &, bool);

public:
    static const char             mime_audio_ac3[];
    static const char             mime_audio_lpcm_48000_2[];
    static const char             mime_audio_mp3[];
    static const char             mime_audio_mpeg[];
    static const char             mime_image_jpeg[];
    static const char             mime_image_png[];
    static const char             mime_image_svg[];
    static const char             mime_video_mpeg[];
    static const char             mime_video_mpegm2ts[];
    static const char             mime_video_mpegts[];
    static const char             mime_text_css_utf8[];
    static const char             mime_text_html_utf8[];
    static const char             mime_text_xml_utf8[];

private:
    static upnp * me;

    class platform::messageloop_ref messageloop;
    const std::string basedir;

    std::set<child *> children;

    uint16_t port;
    bool bind_public;

    std::set<std::string> available_address_set;
    std::set<std::string> bound_address_set;
    platform::timer update_interfaces_timer;
    const std::chrono::seconds update_interfaces_interval;

    bool initialized, webserver_enabled;
    std::map<std::string, http_callback> http_callbacks;

    std::mutex responses_mutex;
    struct response { std::string type; std::shared_ptr<std::istream> stream; };
    std::map<std::string, response> responses;
    platform::timer clear_responses_timer;
    const std::chrono::seconds clear_responses_interval;

    std::map<void *, std::shared_ptr<std::istream>> handles;
};

} // End of namespace

#endif
