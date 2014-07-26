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
#include <vlc/vlc.h>

namespace vlc {

#ifndef NDEBUG
static const int argc = 1;
static const char * const argv[argc] = { "-vvv" };
#else
static const int argc = 0;
static const char * const * const argv = nullptr;
#endif

instance::instance() noexcept
  : libvlc_instance(libvlc_new(argc, argv))
{
}

instance::~instance() noexcept
{
  libvlc_release(libvlc_instance);
}

} // End of namespace
