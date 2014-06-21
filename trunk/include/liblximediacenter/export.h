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

#ifndef LXIMEDIACENTER_EXPORT_H
#define LXIMEDIACENTER_EXPORT_H

#if defined(__unix__) && defined(__GNUC__)
# define LXIMEDIACENTER_PUBLIC  __attribute__((visibility("default")))

#elif defined(__APPLE__) && defined(__GNUC__)
# define LXIMEDIACENTER_PUBLIC  __attribute__((visibility("default")))

#elif defined(WIN32) && defined(__GNUC__)
# if defined(S_BUILD_LIBLXIMEDIACENTER)
#  define LXIMEDIACENTER_PUBLIC __attribute__((dllexport))
# else
#  define LXIMEDIACENTER_PUBLIC __attribute__((dllimport))
# endif

#elif defined(WIN32) && defined(_MSC_VER)
# if defined(S_BUILD_LIBLXIMEDIACENTER)
#  define LXIMEDIACENTER_PUBLIC __declspec(dllexport)
# else
#  define LXIMEDIACENTER_PUBLIC __declspec(dllimport)
# endif

#else
# define LXIMEDIACENTER_PUBLIC

#endif

#endif
