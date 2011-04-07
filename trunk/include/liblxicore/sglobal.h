/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
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

#ifndef LXICORE_SGLOBAL_H
#define LXICORE_SGLOBAL_H

#if defined(Q_OS_UNIX) && defined(__GNUC__)
# define S_DSO_PUBLIC  __attribute__((visibility("default")))
# define S_DSO_PRIVATE __attribute__((visibility("hidden")))
# define S_FUNC_PURE   __attribute__((pure))
#else
# define S_DSO_PUBLIC
# define S_DSO_PRIVATE
# define S_FUNC_PURE
#endif

#endif
