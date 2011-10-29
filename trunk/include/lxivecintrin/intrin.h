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

#ifndef LXIVECINTRIN_VECTYPES
#error Please include <lxivecintrin/vectypes>
#endif
#ifndef LXIVECINTRIN_INTRIN_H
#define LXIVECINTRIN_INTRIN_H

#include "platform.h"

namespace lxivec {
namespace _private {

#ifdef _MSC_VER
# pragma pack(1)
#endif

  struct lxivec_packed lxivec_align VecObject
  {
#if defined(__SSE__)
    lxivec_always_inline void * operator new(size_t size)
    {
      return _mm_malloc(size, vecsize);
    }

    lxivec_always_inline void operator delete(void *ptr)
    {
      _mm_free(ptr);
    }
#endif

  protected:
    lxivec_always_inline VecObject(void)
    {
    }

    lxivec_always_inline ~VecObject()
    {
    }
  };

#ifdef _MSC_VER
# pragma pack()
#endif
} // End of namespace

} // End of namespace

#endif
