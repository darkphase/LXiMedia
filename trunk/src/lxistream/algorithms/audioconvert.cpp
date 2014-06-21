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

#include "audioconvert.h"
#include <lxivecintrin/vectypes>

namespace LXiStream {
namespace Algorithms {

using namespace lxivec;

void AudioConvert::convert(int16_t * dst, const int8_t * src, int n)
{
  for (int i = 0; i < n; i += int8_v::count)
  {
    int16_dv a = int8_v::load(src + i);
    a <<= 8;

    store(dst + i, a);
  }
}

void AudioConvert::convert(int8_t * dst, const int16_t * src, int n)
{
  for (int i = 0; i < n; i += int16_dv::count)
  {
    int8_v a = int16_dv::load(src + i) >> 8;

    store(dst + i, a);
  }
}

void AudioConvert::convert(uint16_t * dst, const uint8_t * src, int n)
{
  for (int i = 0; i < n; i += uint8_v::count)
  {
    uint16_dv a = uint8_v::load(src + i);
    a <<= 8;

    store(dst + i, a);
  }
}

void AudioConvert::convert(uint8_t * dst, const uint16_t * src, int n)
{
  for (int i = 0; i < n; i += uint16_dv::count)
  {
    uint8_v a = uint16_dv::load(src + i) >> 8;

    store(dst + i, a);
  }
}

void AudioConvert::convert(int16_t * dst, const uint8_t * src, int n)
{
  for (int i = 0; i < n; i += uint8_v::count)
  {
    int16_dv a = uint8_v::load(src + i);
    a = (a << 8) - int16_t(32768);

    store(dst + i, a);
  }
}

void AudioConvert::convert(uint8_t * dst, const int16_t * src, int n)
{
  for (int i = 0; i < n; i += int16_dv::count)
  {
    uint8_v a = (int16_dv::load(src + i) + int16_t(32768)) >> 8;

    store(dst + i, a);
  }
}

void AudioConvert::convert(int32_t * dst, const int16_t * src, int n)
{
  for (int i = 0; i < n; i += int16_v::count)
  {
    int32_dv a = int16_v::load(src + i);
    a <<= 16;

    store(dst + i, a);
  }
}

void AudioConvert::convert(int16_t * dst, const int32_t * src, int n)
{
  for (int i = 0; i < n; i += int32_dv::count)
  {
    int16_v a = int32_dv::load(src + i) >> 16;

    store(dst + i, a);
  }
}

void AudioConvert::convert(uint32_t * dst, const uint16_t * src, int n)
{
  for (int i = 0; i < n; i += uint16_v::count)
  {
    uint32_dv a = uint16_v::load(src + i);
    a <<= 16;

    store(dst + i, a);
  }
}

void AudioConvert::convert(uint16_t * dst, const uint32_t * src, int n)
{
  for (int i = 0; i < n; i += uint32_dv::count)
  {
    uint16_v a = uint32_dv::load(src + i) >> 16;

    store(dst + i, a);
  }
}

void AudioConvert::convert(int16_t * dst, const uint16_t * src, int n)
{
  for (int i = 0; i < n; i += uint16_v::count)
  {
    int16_v a = uint16_v::load(src + i) + 32768;

    store(dst + i, a);
  }
}

void AudioConvert::convert(uint16_t * dst, const int16_t * src, int n)
{
  for (int i = 0; i < n; i += int16_v::count)
  {
    uint16_v a = int16_v::load(src + i) - int16_t(32768);

    store(dst + i, a);
  }
}

void AudioConvert::convert(int32_t * dst, const uint32_t * src, int n)
{
  for (int i = 0; i < n; i += uint32_v::count)
  {
    int32_v a = uint32_v::load(src + i) + 2147483648u;

    store(dst + i, a);
  }
}

void AudioConvert::convert(uint32_t * dst, const int32_t * src, int n)
{
  for (int i = 0; i < n; i += int32_v::count)
  {
    uint32_v a = int32_v::load(src + i) - 2147483648u;

    store(dst + i, a);
  }
}

void AudioConvert::convert(float * dst, const int16_t * src, int n)
{
  static const float mul = 1.0f / 32768.0f;

  for (int i = 0; i < n; i += int16_v::count)
  {
    float_dv a = int16_v::load(src + i);
    a *= mul;

    store(dst + i, a);
  }
}

void AudioConvert::convert(int16_t * dst, const float * src, int n)
{
  for (int i = 0; i < n; i += float_dv::count)
  {
    int16_v a = float_dv::load(src + i) * 32768.0f;

    store(dst + i, a);
  }
}

void AudioConvert::convert(float * dst, const int32_t * src, int n)
{
  static const float mul = 1.0f / 2147483648.0f;

  for (int i = 0; i < n; i += int32_v::count)
  {
    float_v a = int32_v::load(src + i);
    a *= mul;

    store(dst + i, a);
  }
}

void AudioConvert::convert(int32_t * dst, const float * src, int n)
{
  for (int i = 0; i < n; i += float_dv::count)
  {
    int32_v a = float_v::load(src + i) * 2147483648.0f;

    store(dst + i, a);
  }
}

void AudioConvert::convert(double * dst, const int16_t * src, int n)
{
  static const double mul = 1.0 / 32768.0;

  for (int i = 0; i < n; i += int16_v::count)
  {
    double_qv a = int16_v::load(src + i);
    a *= mul;

    store(dst + i, a);
  }
}

void AudioConvert::convert(int16_t * dst, const double * src, int n)
{
  for (int i = 0; i < n; i += float_dv::count)
  {
    int16_v a = double_qv::load(src + i) * 32768.0;

    store(dst + i, a);
  }
}

void AudioConvert::convert(double * dst, const int32_t * src, int n)
{
  static const double mul = 1.0 / 2147483648.0;

  for (int i = 0; i < n; i += int32_v::count)
  {
    double_dv a = int32_v::load(src + i);
    a *= mul;

    store(dst + i, a);
  }
}

void AudioConvert::convert(int32_t * dst, const double * src, int n)
{
  for (int i = 0; i < n; i += float_dv::count)
  {
    int32_v a = double_dv::load(src + i) * 2147483648.0;

    store(dst + i, a);
  }
}

} } // End of namespaces
