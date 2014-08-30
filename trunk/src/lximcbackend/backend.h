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

#ifndef BACKEND_H
#define BACKEND_H

#include <cstdint>
#include <functional>
#include <memory>
#include "html/mainpage.h"
#include "html/helppage.h"
#include "html/logpage.h"
#include "html/settingspage.h"
#include "pupnp/connection_manager.h"
#include "pupnp/content_directory.h"
#include "pupnp/mediareceiver_registrar.h"
#include "pupnp/rootdevice.h"
#include "pupnp/upnp.h"
#include "vlc/instance.h"
#include "mediaplayer.h"
#include "settings.h"
#include "setup.h"

class messageloop;

class backend
{
public:
    static std::function<void()> recreate_backend;

public:
    backend(class messageloop &, const std::string &);
    ~backend();

    bool initialize();

private:
    void republish_rootdevice();
    bool apply_settings();
    void add_audio_protocols();
    void add_video_protocols();

private:
    class messageloop &messageloop;
    class settings settings;

    class pupnp::upnp upnp;
    class pupnp::rootdevice rootdevice;
    class pupnp::connection_manager connection_manager;
    class pupnp::content_directory content_directory;
    class pupnp::mediareceiver_registrar mediareceiver_registrar;

    class html::mainpage mainpage;
    class html::settingspage settingspage;
    class html::logpage logpage;
    class html::helppage helppage;

    std::unique_ptr<class vlc::instance> vlc_instance;
    std::unique_ptr<class mediaplayer> mediaplayer;
    std::unique_ptr<class setup> setup;

    timer republish_timer;
    const std::chrono::seconds republish_timeout;
    bool republish_required;

    timer recreate_backend_timer;
    const std::chrono::milliseconds recreate_backend_timeout;

    //  static const int              upnpRepublishTimout;
    //  bool                          upnpRepublishRequired;
    //  QTimer                        upnpRepublishTimer;
};

#endif
