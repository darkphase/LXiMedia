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

#include "ssubpicturebuffer.h"

namespace LXiStream {

SSubpictureBuffer::SSubpictureBuffer(void)
  : SBuffer()
{
}

SSubpictureBuffer::SSubpictureBuffer(const QList<Rect> &rects)
  : SBuffer()
{
  setRects(rects);
}

void SSubpictureBuffer::setRects(const QList<Rect> &rects)
{
  int size = 0;
  foreach (const Rect &rect, rects)
    size += rectSize(rect);

  resize(size);

  char * const data = SBuffer::data();
  int pos = 0;
  foreach (const Rect &rect, rects)
  {
    Rect * const destRect = reinterpret_cast<Rect *>(data + pos);
    *destRect = rect;

    pos += rectSize(rect);
  }
}

QList<SSubpictureBuffer::Rect> SSubpictureBuffer::rects(void) const
{
  QList<Rect> result;

  const char * const data = SBuffer::data();
  const int size = SBuffer::size();
  for (int pos=0; int(pos + sizeof(Rect)) < size;)
  {
    const Rect * const rect = reinterpret_cast<const Rect *>(data + pos);
    result += *rect;

    pos += rectSize(*rect);
  }

  return result;
}

const SPixels::RGBAPixel * SSubpictureBuffer::palette(int rectId) const
{
  const char * const data = SBuffer::data();
  const int size = SBuffer::size();
  for (int pos=0; int(pos + sizeof(Rect)) < size;)
  {
    const Rect * const rect = reinterpret_cast<const Rect *>(data + pos);
    if (rectId-- <= 0)
    {
      return reinterpret_cast<const SPixels::RGBAPixel *>
          (data + pos + SBuffer::align(sizeof(*rect), SBuffer::minimumAlignVal));
    }

    pos += rectSize(*rect);
  }

  return NULL;
}

SPixels::RGBAPixel * SSubpictureBuffer::palette(int rectId)
{
  char * const data = SBuffer::data();
  const int size = SBuffer::size();
  for (int pos=0; int(pos + sizeof(Rect)) < size;)
  {
    const Rect * const rect = reinterpret_cast<const Rect *>(data + pos);
    if (rectId-- <= 0)
    {
      return reinterpret_cast<SPixels::RGBAPixel *>
          (data + pos + SBuffer::align(sizeof(*rect), SBuffer::minimumAlignVal));
    }

    pos += rectSize(*rect);
  }

  return NULL;
}

const quint8 * SSubpictureBuffer::lines(int rectId) const
{
  const char * const data = SBuffer::data();
  const int size = SBuffer::size();
  for (int pos=0; int(pos + sizeof(Rect)) < size;)
  {
    const Rect * const rect = reinterpret_cast<const Rect *>(data + pos);
    if (rectId-- <= 0)
    {
      return reinterpret_cast<const quint8 *>
          (data + pos + SBuffer::align(sizeof(*rect), SBuffer::minimumAlignVal) +
           SBuffer::align(rect->paletteSize * sizeof(SPixels::RGBAPixel), SBuffer::minimumAlignVal));
    }

    pos += rectSize(*rect);
  }

  return NULL;
}

quint8 * SSubpictureBuffer::lines(int rectId)
{
  char * const data = SBuffer::data();
  const int size = SBuffer::size();
  for (int pos=0; int(pos + sizeof(Rect)) < size;)
  {
    const Rect * const rect = reinterpret_cast<const Rect *>(data + pos);
    if (rectId-- <= 0)
    {
      return reinterpret_cast<quint8 *>
          (data + pos + SBuffer::align(sizeof(*rect), SBuffer::minimumAlignVal) +
           SBuffer::align(rect->paletteSize * sizeof(SPixels::RGBAPixel), SBuffer::minimumAlignVal));
    }

    pos += rectSize(*rect);
  }

  return NULL;
}

int SSubpictureBuffer::rectSize(const Rect &rect)
{
  return SBuffer::align(sizeof(rect), SBuffer::minimumAlignVal) +
         SBuffer::align(rect.paletteSize * sizeof(SPixels::RGBAPixel), SBuffer::minimumAlignVal) +
         (rect.height * rect.lineStride * sizeof(quint8));
}

} // End of namespace
