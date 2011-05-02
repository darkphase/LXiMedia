/***************************************************************************
 *   Copyright (C) 2011 by A.J. Admiraal                                   *
 *   code@admiraal.dds.nl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#ifndef LXICORE_SPLATFORM_H
#define LXICORE_SPLATFORM_H

#if defined(__unix__) && defined(__GNUC__)
# define _lxi_internal          __attribute__((visibility("hidden")))
# define _lxi_pure              __attribute__((pure))
# define _lxi_packed            __attribute__((packed))
# define _lxi_align             __attribute__((aligned(16)))

#elif defined(WIN32) && defined(__GNUC__)
# define _lxi_internal
# define _lxi_pure              __attribute__((pure))
# define _lxi_packed            __attribute__((packed))
# define _lxi_align             __attribute__((aligned(16)))

#elif defined(WIN32) && defined(_MSC_VER)
# define _lxi_internal
# define _lxi_pure              __declspec(noalias)
# define _lxi_packed
# define _lxi_align             __declspec(align(16))

#else
# error Platform not supported
#endif

#endif
