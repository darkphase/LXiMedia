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

#ifndef VLC_INSTANCE_H
#define VLC_INSTANCE_H

#include <string>
#include <vector>

struct libvlc_instance_t;

namespace vlc {

class instance
{
public:
    /*! returns:
     *  <0 if the actual version is older than the specified version.
     *   0 if the actual version equals the specified version.
     *  >0 if the actual version is newer than the specified version.
     */
    static int compare_version(int major, int minor = -1, int patch = -1);

    instance() noexcept;
    explicit instance(const std::vector<std::string> &options) noexcept;
    instance(instance &&) noexcept;
    ~instance() noexcept;

    instance & operator=(instance &&);

    instance(const instance &) = delete;
    instance & operator=(const instance &) = delete;

    inline operator libvlc_instance_t *() noexcept { return libvlc_instance; }

private:
    libvlc_instance_t * libvlc_instance;
};

} // End of namespace

#endif
