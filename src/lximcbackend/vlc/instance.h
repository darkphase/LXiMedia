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

#include <istream>
#include <memory>
#include <string>

struct libvlc_instance_t;

namespace lximediacenter {
namespace vlc {

class instance
{
public:
  instance();
  ~instance();
  instance(const instance &) = delete;
  instance & operator=(const instance &) = delete;

  inline operator libvlc_instance_t *() { return libvlc_instance; }

private:
  libvlc_instance_t * const libvlc_instance;
};

} // End of namespace
} // End of namespace

#endif
