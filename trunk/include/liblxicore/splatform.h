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
# define __internal             __attribute__((visibility("hidden")))
# define __pure                 __attribute__((const))
# define __packed               __attribute__((packed))
# define __expect(condition, x) __builtin_expect(condition, x)
# define __popcount(x)          __builtin_popcount(x)

#elif defined(WIN32) && defined(__GNUC__)
# define __internal
# define __pure                 __attribute__((const))
# define __packed               __attribute__((packed))
# define __expect(condition, x) __builtin_expect(condition, x)
# define __popcount(x)          __builtin_popcount(x)

#elif defined(WIN32) && defined(_MSC_VER)
# define __internal
# define __pure                 __declspec(noalias)
# define __packed
# define __expect(condition, x) (condition)
# if _MSC_VER < 1600 // 1600 = VS2010
#  define __popcount(x)         (((x&(1<< 0))?1:0)+((x&(1<< 1))?1:0)+((x&(1<< 2))?1:0)+((x&(1<< 3))?1:0)+((x&(1<< 4))?1:0)+((x&(1<< 5))?1:0)+((x&(1<< 6))?1:0)+((x&(1<< 7))?1:0)+\
                                 ((x&(1<< 8))?1:0)+((x&(1<< 9))?1:0)+((x&(1<<10))?1:0)+((x&(1<<11))?1:0)+((x&(1<<12))?1:0)+((x&(1<<13))?1:0)+((x&(1<<14))?1:0)+((x&(1<<15))?1:0)+\
                                 ((x&(1<<16))?1:0)+((x&(1<<17))?1:0)+((x&(1<<18))?1:0)+((x&(1<<19))?1:0)+((x&(1<<20))?1:0)+((x&(1<<21))?1:0)+((x&(1<<22))?1:0)+((x&(1<<23))?1:0)+\
                                 ((x&(1<<24))?1:0)+((x&(1<<25))?1:0)+((x&(1<<26))?1:0)+((x&(1<<27))?1:0)+((x&(1<<28))?1:0)+((x&(1<<29))?1:0)+((x&(1<<30))?1:0)+((x&(1<<31))?1:0))
# else
#  define __popcount(x)         __popcnt(x)
# endif

#else
# error Platform not supported
#endif

#endif
