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

#include "instance.h"
#include "media.h"
#include <vlc/vlc.h>

namespace vlc {

#if defined(TEST_H)
static const int argc = 1;
static const char * const argv[argc] = { "-q" };
#elif defined(NDEBUG)
static const int argc = 0;
static const char * const * const argv = nullptr;
#else
static const int argc = 1;
static const char * const argv[argc] = { "-v" };
#endif

int instance::compare_version(int major, int minor, int patch)
{
    static const char number[] = "0123456789";
    const std::string version = libvlc_get_version();

    const auto major_b = version.find_first_of(number);
    const auto major_e = version.find_first_not_of(number, major_b + 1);
    if ((major_b != version.npos) && (major_e != version.npos))
    {
        const int comp = std::stoi(version.substr(major_b, major_e - major_b)) - major;
        if ((comp != 0) || (minor < 0))
            return comp;

        const auto minor_b = version.find_first_of(number, major_e);
        const auto minor_e = version.find_first_not_of(number, minor_b + 1);
        if ((minor_b != version.npos) && (minor_e != version.npos))
        {
            const int comp = std::stoi(version.substr(minor_b, minor_e - minor_b)) - minor;
            if ((comp != 0) || (patch < 0))
                return comp;

            const auto patch_b = version.find_first_of(number, minor_e);
            if (patch_b != version.npos)
            {
                auto patch_e = version.find_first_not_of(number, patch_b + 1);
                if (patch_e == version.npos) patch_e = version.length();

                return std::stoi(version.substr(patch_b, patch_e - patch_b)) - patch;
            }
        }
    }

    return -1;
}

instance::instance() noexcept
    : libvlc_instance(libvlc_new(argc, argv))
{
}

instance::~instance() noexcept
{
    media::flush_cache();

    libvlc_release(libvlc_instance);
}

} // End of namespace