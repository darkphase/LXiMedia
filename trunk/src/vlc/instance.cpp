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

#include "instance.h"
#include "platform/string.h"
#include <vlc/vlc.h>
#include <cstdlib>
#include <iostream>

namespace vlc {

void instance::initialize(int argc, const char *argv[])
{
#if defined(__APPLE__)
    static std::string vlc_plugin_path;

    if (argc > 0)
    {
        std::string path = argv[0];
        while (!path.empty() && (path.back() != '/'))
            path.pop_back();

        vlc_plugin_path = "VLC_PLUGIN_PATH=" + path + "plugins";
        ::putenv(&vlc_plugin_path[0]);
    }
#endif
}

std::string instance::version()
{
    return libvlc_get_version();
}

static libvlc_instance_t * create_instance(const std::vector<std::string> &options) noexcept
{
    std::vector<const char *> argv;
#if !defined(TEST_H)
    argv.push_back("-vvv");
#endif

    if (compare_version(instance::version(), "2.2") >= 0)
    {
        argv.push_back("--avcodec-hw");
        argv.push_back("disable");
    }

    for (auto &i : options)
        argv.push_back(i.c_str());

#if !defined(TEST_H)
    std::clog << "vlc::instance: creating new libvlc instance ("
              << libvlc_get_version() << ") with arguments:";

    for (auto i : argv)
        std::clog << ' ' << i;

    std::clog << std::endl;
#endif

    return libvlc_new(int(argv.size()), argv.data());
}

instance::instance() noexcept
    : libvlc_instance(create_instance(std::vector<std::string>()))
{
}

instance::instance(const std::vector<std::string> &options) noexcept
    : libvlc_instance(create_instance(options))
{
}

instance::instance(instance &&from) noexcept
    : libvlc_instance(from.libvlc_instance)
{
    from.libvlc_instance = nullptr;
}

instance & instance::operator=(instance &&from)
{
    libvlc_instance = from.libvlc_instance;
    from.libvlc_instance = nullptr;

    return *this;
}

instance::~instance() noexcept
{
    if (libvlc_instance)
        libvlc_release(libvlc_instance);
}

} // End of namespace
