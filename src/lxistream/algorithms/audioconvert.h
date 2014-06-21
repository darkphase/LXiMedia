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

#include <sys/types.h>
#include <stdint.h>

namespace LXiStream {
namespace Algorithms {

class AudioConvert
{
public:
  static void convert(int16_t * dst, const int8_t * src, int n);
  static void convert(int8_t * dst, const int16_t * src, int n);
  static void convert(uint16_t * dst, const uint8_t * src, int n);
  static void convert(uint8_t * dst, const uint16_t * src, int n);
  static void convert(int16_t * dst, const uint8_t * src, int n);
  static void convert(uint8_t * dst, const int16_t * src, int n);
  static void convert(int32_t * dst, const int16_t * src, int n);
  static void convert(int16_t * dst, const int32_t * src, int n);
  static void convert(uint32_t * dst, const uint16_t * src, int n);
  static void convert(uint16_t * dst, const uint32_t * src, int n);
  static void convert(int16_t * dst, const uint16_t * src, int n);
  static void convert(uint16_t * dst, const int16_t * src, int n);
  static void convert(int32_t * dst, const uint32_t * src, int n);
  static void convert(uint32_t * dst, const int32_t * src, int n);
  static void convert(float * dst, const int16_t * src, int n);
  static void convert(int16_t * dst, const float * src, int n);
  static void convert(float * dst, const int32_t * src, int n);
  static void convert(int32_t * dst, const float * src, int n);
  static void convert(double * dst, const int16_t * src, int n);
  static void convert(int16_t * dst, const double * src, int n);
  static void convert(double * dst, const int32_t * src, int n);
  static void convert(int32_t * dst, const double * src, int n);
};

} } // End of namespaces
