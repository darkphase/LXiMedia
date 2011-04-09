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

#ifndef LXIMEDIACENTER_EXPORT_H
#define LXIMEDIACENTER_EXPORT_H

#if defined(Q_OS_UNIX) && defined(__GNUC__)
# define LXIMEDIACENTER_PUBLIC  __attribute__((visibility("default")))

#elif defined(Q_OS_WIN) && defined(__GNUC__)
# if defined(S_BUILD_LIBLXIMEDIACENTER)
#  define LXIMEDIACENTER_PUBLIC __attribute__((dllexport))
# else
#  define LXIMEDIACENTER_PUBLIC __attribute__((dllimport))
# endif

#else
# define LXIMEDIACENTER_PUBLIC

#endif

#endif
