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

#ifndef VLC_MEDIA_H
#define VLC_MEDIA_H

#include <string>

struct libvlc_media_t;

namespace vlc {

class instance;

class media
{
public:
    static media from_file(class instance &, const std::string &path) noexcept;
    static media from_mrl(class instance &, const std::string &mrl) noexcept;

    media();
    media(const media &) noexcept;
    media(media &&) noexcept;
    ~media() noexcept;

    media & operator=(const media &) noexcept;
    media & operator=(media &&) noexcept;
    inline operator libvlc_media_t *() noexcept { return libvlc_media; }

    std::string mrl() const noexcept;

private:
    media(libvlc_media_t *) noexcept;

    libvlc_media_t *libvlc_media;
};

} // End of namespace

#endif
