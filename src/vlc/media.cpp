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

#include "vlc/media.h"
#include "vlc/instance.h"
#include <vlc/vlc.h>
#include <algorithm>
#include <map>

namespace vlc {

media media::from_file(class instance &instance, const std::string &path) noexcept
{
#if defined(__unix__) || defined(__APPLE__)
    return libvlc_media_new_path(instance, path.c_str());
#elif defined(WIN32)
    std::string bspath = path;
    std::replace(bspath.begin(), bspath.end(), '/', '\\');
    return libvlc_media_new_path(instance, bspath.c_str());
#endif
}

media media::from_mrl(class instance &instance, const std::string &mrl) noexcept
{
    return libvlc_media_new_location(instance, mrl.c_str());
}

media::media()
    : libvlc_media(nullptr)
{
}

media::media(libvlc_media_t *libvlc_media) noexcept
    : libvlc_media(libvlc_media)
{
}

media::media(const media &from) noexcept
    : libvlc_media(from.libvlc_media)
{
    if (libvlc_media) libvlc_media_retain(libvlc_media);
}

media::media(media &&from) noexcept
    : libvlc_media(from.libvlc_media)
{
    from.libvlc_media = nullptr;
}

media::~media() noexcept
{
    if (libvlc_media) libvlc_media_release(libvlc_media);
}

media & media::operator=(const media &from) noexcept
{
    if (from.libvlc_media) libvlc_media_retain(from.libvlc_media);
    if (libvlc_media) libvlc_media_release(libvlc_media);
    libvlc_media = from.libvlc_media;

    return *this;
}

media & media::operator=(media &&from) noexcept
{
    if (libvlc_media) libvlc_media_release(libvlc_media);
    libvlc_media = from.libvlc_media;
    from.libvlc_media = nullptr;

    return *this;
}

std::string media::mrl() const noexcept
{
    return libvlc_media ? libvlc_media_get_mrl(libvlc_media) : "";
}

} // End of namespace
