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

#include "nodes/ssubpicturerendernode.h"
#include "../../algorithms/videoprocess.h"
#include "ssubpicturebuffer.h"
#include "svideobuffer.h"

namespace LXiStream {

struct SSubpictureRenderNode::Data
{
  volatile bool                 enabled;
  QMap<STime, SSubpictureBuffer> subpictures;
  SSubpictureBuffer             subpicture;
  STime                         subpictureTime;
  bool                          subpictureVisible;

  struct Rect : Algorithms::VideoProcess::SubpictureRect
  {
    inline                      Rect(void) { lines = NULL; }

    QByteArray                  palette;
  };

  QList<Rect>                   subpictureRects;
};

SSubpictureRenderNode::SSubpictureRenderNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->enabled = false;
  d->subpictureVisible = false;
}

SSubpictureRenderNode::~SSubpictureRenderNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SSubpictureRenderNode::start(void)
{
  return true;
}

void SSubpictureRenderNode::stop(void)
{
}

void SSubpictureRenderNode::input(const SSubpictureBuffer &subpictureBuffer)
{
  if (subpictureBuffer.duration().toSec() <= 10)
  {
    d->subpictures.insert(subpictureBuffer.timeStamp(), subpictureBuffer);
  }
  else // Prevent showing subtitles too long.
  {
    SSubpictureBuffer corrected = subpictureBuffer;
    corrected.setDuration(STime::fromSec(10));
    d->subpictures.insert(subpictureBuffer.timeStamp(), corrected);
  }

  d->enabled = true;
}

void SSubpictureRenderNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull() && d->enabled)
  {
    const SSize size = videoBuffer.format().size();

    for (QMap<STime, SSubpictureBuffer>::Iterator i=d->subpictures.begin(); i!=d->subpictures.end(); )
    {
      const STime timeStamp = videoBuffer.timeStamp();

      // Can the current subpicture be removed.
      if (d->subpictureVisible && ((i.key() + i->duration()) < timeStamp))
      {
        i = d->subpictures.erase(i);
        d->subpicture.clear();
        d->subpictureTime = STime();
        d->subpictureVisible = false;
        continue;
      }

      // Can the next subpicture be displayed yet.
      QMap<STime, SSubpictureBuffer>::Iterator n = i + 1;
      if (n != d->subpictures.end())
      if ((n.key() != d->subpictureTime) && (n.key() <= timeStamp))
      {
        i = d->subpictures.erase(i);
        d->subpicture.clear();
        d->subpictureTime = STime();
        d->subpictureVisible = false;
        continue;
      }

      // Render the next subpicture.
      if ((i.key() != d->subpictureTime) && (i.key() <= timeStamp))
      {
        d->subpicture = *i;
        d->subpictureRects.clear();

        foreach (const SSubpictureBuffer::Rect &rect, d->subpicture.rects())
        {
          Data::Rect r;

          if ((rect.x + int(rect.width) < size.width()) &&
              (rect.y + int(rect.height) < size.height()))
          {
            r.x = rect.x;
            r.y = rect.y;
          }
          else // Subtitle rect is outside of picture rect; center it.
          {
            r.x = (size.width() / 2) - (rect.width / 2);
            r.y = size.height() - rect.height - (size.height() / 16);
          }

          r.width = rect.width;
          r.height = rect.height;
          r.lineStride = rect.lineStride;
          r.lines = d->subpicture.lines(d->subpictureRects.count());

          buildPalette(
              d->subpicture.palette(d->subpictureRects.count()),
              rect.paletteSize,
              videoBuffer.format().format(),
              r.palette);

          d->subpictureRects += r;
        }

        d->subpictureVisible = true;
        d->subpictureTime = i.key();
      }

      break;
    }

    if (d->subpictureVisible && !d->subpictures.isEmpty())
    {
      SVideoBuffer buffer = videoBuffer;

      foreach (const Data::Rect &rect, d->subpictureRects)
      if ((rect.x >= 0) && (rect.y >=0) &&
          (rect.x + int(rect.width) < size.width()) &&
          (rect.y + int(rect.height) < size.height()))
      {
        switch (buffer.format().format())
        {
        case SVideoFormat::Format_YUV410P:
        case SVideoFormat::Format_YUV411P:
        case SVideoFormat::Format_YUV420P:
        case SVideoFormat::Format_YUV422P:
        case SVideoFormat::Format_YUV444P:
          {
            Algorithms::VideoProcess::YUVData yuvData;
            yuvData.y = reinterpret_cast<uint8_t *>(buffer.scanLine(0, 0));
            yuvData.u = reinterpret_cast<uint8_t *>(buffer.scanLine(0, 1));
            yuvData.v = reinterpret_cast<uint8_t *>(buffer.scanLine(0, 2));
            yuvData.yStride = buffer.lineSize(0);
            yuvData.uStride = buffer.lineSize(1);
            yuvData.vStride = buffer.lineSize(2);

            yuvData.wr = yuvData.hr = 1;
            buffer.format().planarYUVRatio(yuvData.wr, yuvData.hr);

            Algorithms::VideoProcess::mixSubpictureYUV(
                yuvData, rect, rect.palette.data(), rect.palette.size());

            break;
          }

        default:
          break;
        }
      }

      emit output(buffer);
    }
    else
      emit output(videoBuffer);
  }
  else
    emit output(videoBuffer);
}

void SSubpictureRenderNode::buildPalette(const quint32 *palette, unsigned paletteSize, SVideoFormat::Format dstFormat, QByteArray &dstPalette)
{
  struct T
  {
#if defined(__GNUC__)
    union RGBAPixel { struct __attribute__((packed)) { uint8_t b, g, r, a; }; quint32 p; };
    union YUVAPixel { struct __attribute__((packed)) { uint8_t y, u, v, a; }; quint32 p; };
#elif defined(_MSC_VER)
# pragma pack(1)
    union RGBAPixel { struct { uint8_t b, g, r, a; }; quint32 p; };
    union YUVAPixel { struct { uint8_t y, u, v, a; }; quint32 p; };
# pragma pack()
#else
    union RGBAPixel { struct { uint8_t b, g, r, a; }; quint32 p; };
    union YUVAPixel { struct { uint8_t y, u, v, a; }; quint32 p; };
#endif

    static inline quint32 RGBAtoYUVA(quint32 src)
    {
      YUVAPixel result;
      RGBAPixel p;
      p.p = src;

      result.y = (uint8_t)(((( 66 * (int)(p.r)) + (129 * (int)(p.g)) + ( 25 * (int)(p.b)) + 128) >> 8) +  16);
      result.u = (uint8_t)((((-38 * (int)(p.r)) - ( 74 * (int)(p.g)) + (112 * (int)(p.b)) + 128) >> 8) + 128);
      result.v = (uint8_t)((((112 * (int)(p.r)) - ( 94 * (int)(p.g)) - ( 18 * (int)(p.b)) + 128) >> 8) + 128);
      result.a = p.a;

      return result.p;
    }
  };

  switch (dstFormat)
  {
  case SVideoFormat::Format_YUV410P:
  case SVideoFormat::Format_YUV411P:
  case SVideoFormat::Format_YUV420P:
  case SVideoFormat::Format_YUV422P:
  case SVideoFormat::Format_YUV444P:
    {
      dstPalette.resize(paletteSize * sizeof(quint32));
      for (unsigned i=0; i<paletteSize; i++)
        reinterpret_cast<quint32 *>(dstPalette.data())[i] = T::RGBAtoYUVA(palette[i]);
    }
    break;

  default:
    dstPalette.clear();
    break;
  }
}

} // End of namespace
