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

#include "spixels.h"
#include "svideobuffer.h"

namespace LXiStream {
namespace SPixels {

YUVConstData::YUVConstData(const SVideoBuffer &buffer)
{
  if (buffer.format().isYUV())
  {
    y = reinterpret_cast<const uint8_t *>(buffer.scanLine(0, 0));
    u = reinterpret_cast<const uint8_t *>(buffer.scanLine(0, 1));
    v = reinterpret_cast<const uint8_t *>(buffer.scanLine(0, 2));
    yStride = buffer.lineSize(0);
    uStride = buffer.lineSize(1);
    vStride = buffer.lineSize(2);

    wr = hr = 1;
    buffer.format().planarYUVRatio(wr, hr);
  }
  else
  {
    y = u = v = NULL;
    yStride = uStride = vStride = 0;
    wr = hr = 0;
  }
}

YUVData::YUVData(SVideoBuffer &buffer)
{
  if (buffer.format().isYUV())
  {
    y = reinterpret_cast<uint8_t *>(buffer.scanLine(0, 0));
    u = reinterpret_cast<uint8_t *>(buffer.scanLine(0, 1));
    v = reinterpret_cast<uint8_t *>(buffer.scanLine(0, 2));
    yStride = buffer.lineSize(0);
    uStride = buffer.lineSize(1);
    vStride = buffer.lineSize(2);

    wr = hr = 1;
    buffer.format().planarYUVRatio(wr, hr);
  }
  else
  {
    y = u = v = NULL;
    yStride = uStride = vStride = 0;
    wr = hr = 0;
  }
}

} } // End of namespaces
