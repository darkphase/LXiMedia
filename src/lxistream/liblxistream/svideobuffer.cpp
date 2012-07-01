/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#include "svideobuffer.h"

namespace LXiStream {


SVideoBuffer::SVideoBuffer(void)
{
  for (int i=0; i<d.maxPlanes; i++)
    d.lineSize[i] = d.offset[i] = 0;
}

SVideoBuffer::SVideoBuffer(const SVideoFormat &format)
{
  for (int i=0; i<d.maxPlanes; i++)
    d.lineSize[i] = d.offset[i] = 0;

  setFormat(format);
}

SVideoBuffer::SVideoBuffer(const SVideoFormat &format, const MemoryPtr &memory, const int *offset, const int *lineSize)
  : SBuffer(memory)
{
  d.format = format;

  int i = 0;
  for (int n=format.numPlanes(); i<n; i++)
  {
    d.offset[i] = offset[i];
    d.lineSize[i] = lineSize[i];
  }

  for (; i<d.maxPlanes; i++)
    d.lineSize[i] = d.offset[i] = 0;
}

/*! Sets the format of the video buffer. This method ensures enough memory is
    allocated for the format and ensures lineSize() and offset() return the
    correct values. Any data already in the buffer may be corrupted by this
    method.

    \sa overrideFormat()
 */
void SVideoBuffer::setFormat(const SVideoFormat &format)
{
  for (int i=0; i<d.maxPlanes; i++)
    d.lineSize[i] = d.offset[i] = 0;

  d.format = format;

  int size = 0;
  const int numPlanes = format.numPlanes();
  if (numPlanes > 1)
  {
    int wr = 1, hr = 1;
    format.planarYUVRatio(wr, hr);

    d.offset[0] = size;
    d.lineSize[0] = align(format.size().width() * format.sampleSize(), minimumAlignVal);

    size += d.lineSize[0] * format.size().height();

    for (int i=1; i<numPlanes; i++)
    {
      d.offset[i] = size;
      d.lineSize[i] = align((format.size().width() / wr) * format.sampleSize(), minimumAlignVal);

      size += d.lineSize[i] * (format.size().height() / hr);
    }
  }
  else
  {
    d.offset[0] = size;
    d.lineSize[0] = align(format.size().width() * format.sampleSize(), minimumAlignVal);

    size += d.lineSize[0] * format.size().height();
  }

  resize(size);
}

/*! Sets the format of teh video buffer. This method does not guarantee the
    buffer is large enough to contain an image of the specified format.

    \sa setFormat()
 */
void SVideoBuffer::overrideFormat(const SVideoFormat &format)
{
  d.format = format;
}

const char * SVideoBuffer::scanLine(int y, int plane) const
{
  Q_ASSERT(plane < d.maxPlanes);

  if (d.lineSize[plane] > 0)
    return data() + d.offset[plane] + (d.lineSize[plane] * y);

  return NULL;
}

char * SVideoBuffer::scanLine(int y, int plane)
{
  Q_ASSERT(plane < d.maxPlanes);

  if (d.lineSize[plane] > 0)
    return data() + d.offset[plane] + (d.lineSize[plane] * y);

  return NULL;
}


} // End of namespace
