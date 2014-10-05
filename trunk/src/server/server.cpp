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

#include "server.h"
#include <cassert>

std::function<void()> server::recreate_server;

server::server(
        class platform::messageloop &messageloop,
        class settings &settings,
        class pupnp::upnp &upnp,
        const std::string &logfilename)
    : messageloop(messageloop),
      settings(settings),
      watchlist(messageloop),
      upnp(upnp),
      rootdevice(messageloop, upnp, settings.uuid(), "urn:schemas-upnp-org:device:MediaServer:1"),
      connection_manager(messageloop, rootdevice),
      content_directory(messageloop, upnp, rootdevice, connection_manager),
      mediareceiver_registrar(messageloop, rootdevice),
      mainpage(messageloop, upnp, connection_manager),
      settingspage(mainpage, settings, std::bind(&server::apply_settings, this)),
      logpage(mainpage, logfilename),
      helppage(mainpage),
      vlc_instance(nullptr),
      files(nullptr),
      republish_timer(messageloop, std::bind(&server::republish_rootdevice, this)),
      republish_timeout(15),
      republish_required(false),
      recreate_server_timer(messageloop, [this] { if (recreate_server) this->messageloop.post(recreate_server); }),
      recreate_server_timeout(500)
{
}

server::~server()
{
    rootdevice.handled_action.erase(this);
    connection_manager.numconnections_changed.erase(this);
}

bool server::initialize()
{
    const std::string upnp_devicename = settings.upnp_devicename();
    rootdevice.set_devicename(upnp_devicename);
    mainpage.set_devicename(upnp_devicename);

    add_audio_protocols();
    add_video_protocols();

    if (upnp.initialize(settings.http_port(), settings.bind_all_networks()))
    {
        vlc_instance.reset(new class vlc::instance(
                               settings.verbose_logging_enabled()));

        files.reset(new class files(
                        messageloop,
                        *vlc_instance,
                        connection_manager,
                        content_directory,
                        settings,
                        watchlist));

        setup.reset(new class setup(
                        messageloop,
                        content_directory,
                        settings));

        republish_required = settings.republish_rootdevice();
        if (republish_required)
        {
            rootdevice.handled_action[this] = [this]
            {
                if (republish_required)
                    republish_timer.start(republish_timeout * 8);
            };

            connection_manager.numconnections_changed[this] = [this](size_t count)
            {
                if (count > 0)
                {
                    republish_required = false;
                    republish_timer.stop();
                }
                else
                {
                    republish_required = true;
                    republish_timer.start(republish_timeout * 4);
                }
            };

            republish_timer.start(republish_timeout);
        }

        return true;
    }

    return false;
}

const std::set<std::string> & server::bound_addresses() const
{
    return upnp.bound_addresses();
}

uint16_t server::bound_port() const
{
    return upnp.bound_port();
}

void server::republish_rootdevice()
{
    assert(republish_required);
    if (republish_required)
    {
        rootdevice.close();
        rootdevice.initialize();
    }
}

bool server::apply_settings()
{
    if (connection_manager.output_connections().empty())
    {
        settings.save();

        // Wait a bit before restarting to handle pending requests from the webbrowser.
        recreate_server_timer.start(recreate_server_timeout, true);

        return true;
    }

    return false;
}
