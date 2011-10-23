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

#include <sys/types.h>
#include <stdint.h>

namespace LXiStream {
namespace Algorithms {

class Data
{
public:
  static void                   swapBytes(uint16_t * dst, const uint16_t * src, int n);
  inline static void            swapBytes(int16_t * dst, const int16_t * src, int n)    { return swapBytes(reinterpret_cast<uint16_t *>(dst), reinterpret_cast<const uint16_t *>(src), n); }
  static void                   swapBytes(uint32_t * dst, const uint32_t * src, int n);
  inline static void            swapBytes(int32_t * dst, const int32_t * src, int n)    { return swapBytes(reinterpret_cast<uint32_t *>(dst), reinterpret_cast<const uint32_t *>(src), n); }
  inline static void            swapBytes(float * dst, const float * src, int n)        { return swapBytes(reinterpret_cast<uint32_t *>(dst), reinterpret_cast<const uint32_t *>(src), n); }
  static void                   swapBytes(uint64_t * dst, const uint64_t * src, int n);
  inline static void            swapBytes(int64_t * dst, const int64_t * src, int n)    { return swapBytes(reinterpret_cast<uint64_t *>(dst), reinterpret_cast<const uint64_t *>(src), n); }
  inline static void            swapBytes(double * dst, const double * src, int n)      { return swapBytes(reinterpret_cast<uint64_t *>(dst), reinterpret_cast<const uint64_t *>(src), n); }
};

} } // End of namespaces
