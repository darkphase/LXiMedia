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

#ifndef SERVER_H
#define SERVER_H

#include <cstdint>
#include <functional>
#include <memory>
#include "html/mainpage.h"
#include "html/helppage.h"
#include "html/logpage.h"
#include "html/settingspage.h"
#include "html/setuppage.h"
#include "platform/messageloop.h"
#include "pupnp/connection_manager.h"
#include "pupnp/content_directory.h"
#include "pupnp/mediareceiver_registrar.h"
#include "pupnp/rootdevice.h"
#include "pupnp/upnp.h"
#include "vlc/instance.h"
#include "settings.h"

class files;
class recommended;
class setup;
class test;

class server
{
public:
    static std::function<void()> recreate_server;

public:
    server(
            class platform::messageloop_ref &,
            class settings &,
            class pupnp::upnp &,
            const std::string &,
            class platform::inifile &media_cache_file,
            class platform::inifile &watchlist_file);

    ~server();

    bool initialize();

    const std::set<std::string> & bound_addresses() const;
    uint16_t bound_port() const;

private:
    void republish_rootdevice();
    bool apply_settings();
    void force_apply_settings();
    void add_audio_protocols();
    void add_video_protocols();
    void add_image_protocols();

private:
    class platform::messageloop_ref messageloop;
    class settings &settings;
    class platform::inifile &media_cache_file;
    class platform::inifile &watchlist_file;

    class pupnp::upnp &upnp;
    class pupnp::rootdevice rootdevice;
    class pupnp::connection_manager connection_manager;
    class pupnp::content_directory content_directory;
    class pupnp::mediareceiver_registrar mediareceiver_registrar;

    std::unique_ptr<class vlc::instance> vlc_instance;
    std::unique_ptr<class recommended> recommended;
    std::unique_ptr<class files> files;
    std::unique_ptr<class setup> setup;
    std::unique_ptr<class test> test;

    class html::mainpage mainpage;
    class html::settingspage settingspage;
    class html::logpage logpage;
    class html::helppage helppage;
    class html::setuppage setuppage;

    platform::timer republish_timer;
    const std::chrono::seconds republish_timeout;
    bool republish_required;

    platform::timer recreate_server_timer;
    const std::chrono::milliseconds recreate_server_timeout;
};

#endif
