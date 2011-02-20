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

#include "nodes/ssubpicturerendernode.h"
#include "sdebug.h"
#include "sgraph.h"
#include "ssubpicturebuffer.h"
#include "svideobuffer.h"

// Implemented in ssubpicturerendernode.mix.c
extern "C" void LXiStream_SSubpictureRenderNode_mixSubpictureYUV
    (const LXiStream::SPixels::YUVData &yuvData,
     const void *rect, const void *palette, unsigned paletteSize);

namespace LXiStream {

struct SSubpictureRenderNode::Data
{
  SDependency                 * dependency;

  QMutex                        mutex;
  volatile bool                 enabled;
  QMap<STime, SSubpictureBuffer> subpictures;
  SSubpictureBuffer             subpicture;
  STime                         subpictureTime;
  bool                          subpictureVisible;

  struct Rect
  {
    inline                      Rect(void) : lines(NULL) { }

    int                         x, y;
    unsigned                    width, height;
    unsigned                    lineStride;
    const quint8              * lines;

    QByteArray                  palette;
  };

  QList<Rect>                   subpictureRects;
};

SSubpictureRenderNode::SSubpictureRenderNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->dependency = parent ? new SDependency() : NULL;
  d->enabled = false;
  d->subpictureVisible = false;
}

SSubpictureRenderNode::~SSubpictureRenderNode()
{
  delete d->dependency;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SSubpictureRenderNode::input(const SSubpictureBuffer &subpictureBuffer)
{
  if (!subpictureBuffer.isNull())
  {
    if (graph)
      graph->queue(this, &SSubpictureRenderNode::processTask, subpictureBuffer, d->dependency);
    else
      processTask(subpictureBuffer);
  }
}

void SSubpictureRenderNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull() && d->enabled)
  {
    if (graph)
      graph->queue(this, &SSubpictureRenderNode::processTask, videoBuffer, d->dependency);
    else
      processTask(videoBuffer);
  }
  else
    emit output(videoBuffer);
}

void SSubpictureRenderNode::processTask(const SSubpictureBuffer &subpictureBuffer)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

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

void SSubpictureRenderNode::processTask(const SVideoBuffer &videoBuffer)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

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
        r.x = rect.x;
        r.y = rect.y;
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
    const int width = buffer.format().size().width(), height = buffer.format().size().height();

    foreach (const Data::Rect &rect, d->subpictureRects)
    if ((rect.x >= 0) && (rect.y >=0) &&
        (rect.x + int(rect.width) < width) && (rect.y + int(rect.height) < height))
    {
      switch (buffer.format().format())
      {
      case SVideoFormat::Format_YUV410P:
      case SVideoFormat::Format_YUV411P:
      case SVideoFormat::Format_YUV420P:
      case SVideoFormat::Format_YUV422P:
      case SVideoFormat::Format_YUV444P:
        LXiStream_SSubpictureRenderNode_mixSubpictureYUV(
            SPixels::YUVData::fromVideoBuffer(buffer),
            &rect, rect.palette.data(), rect.palette.size());

        break;

      default:
        break;
      }
    }

    emit output(buffer);
  }
  else
    emit output(videoBuffer);
}

void SSubpictureRenderNode::buildPalette(const SPixels::RGBAPixel *palette, int paletteSize, SVideoFormat::Format dstFormat, QByteArray &dstPalette)
{
  switch (dstFormat)
  {
  case SVideoFormat::Format_YUV410P:
  case SVideoFormat::Format_YUV411P:
  case SVideoFormat::Format_YUV420P:
  case SVideoFormat::Format_YUV422P:
  case SVideoFormat::Format_YUV444P:
    {
      dstPalette.resize(paletteSize * sizeof(SPixels::YUVAPixel));
      for (int i=0; i<paletteSize; i++)
        reinterpret_cast<SPixels::YUVAPixel *>(dstPalette.data())[i] = SPixels::RGBA2YUVA(palette[i]);
    }
    break;

  default:
    dstPalette.clear();
    break;
  }
}

} // End of namespace
