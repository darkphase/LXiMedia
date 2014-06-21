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

#include "nodes/svideoboxnode.h"
#include "svideobuffer.h"
#include <cstring>
#include <QtConcurrent>

namespace LXiStream {

struct SVideoBoxNode::Data
{
  SSize                         destSize;
};

SVideoBoxNode::SVideoBoxNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->destSize = SSize(768, 576);
}

SVideoBoxNode::~SVideoBoxNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

SSize SVideoBoxNode::size(void) const
{
  return d->destSize;
}

void SVideoBoxNode::setSize(const SSize &s)
{
  d->destSize = s;
}

bool SVideoBoxNode::start(void)
{
  return true;
}

void SVideoBoxNode::stop(void)
{
}

void SVideoBoxNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
  {
    if (videoBuffer.format().size() != d->destSize)
    {
      SVideoBuffer destBuffer(SVideoFormat(videoBuffer.format().format(),
                                           d->destSize,
                                           videoBuffer.format().frameRate(),
                                           videoBuffer.format().fieldMode()));

      const int threadCount = QThreadPool::globalInstance()->maxThreadCount();
      const int h = d->destSize.height();
      const int sh = (((h + threadCount - 1) / threadCount) + 1) & ~1;

      QVector< QFuture<void> > future;
      future.reserve(threadCount);

      for (int y=0; y<h; y+=sh)
        future += QtConcurrent::run(&SVideoBoxNode::boxSlice, &destBuffer, &videoBuffer, y, qMin(y + sh, h));

      for (int i=0; i<future.count(); i++)
        future[i].waitForFinished();

      destBuffer.setTimeStamp(videoBuffer.timeStamp());
      emit output(destBuffer);
    }
    else
      emit output(videoBuffer);
  }
  else
    emit output(videoBuffer);
}

void SVideoBoxNode::boxSlice(SVideoBuffer *dst, const SVideoBuffer *src, int top, int bottom)
{
  const SSize dstSize = dst->format().size();
  const SSize srcSize = src->format().size();

  const int inLineOffset = (dstSize.height() < srcSize.height()) ? ((((srcSize.height() - dstSize.height()) / 2) + 1) & ~1) : 0;
  const int inPixelOffset = (dstSize.width() < srcSize.width()) ? ((srcSize.width() - dstSize.width()) / 2) : 0;
  const int inHeight = srcSize.height() & ~1;
  const int outLineOffset = (srcSize.height() < dstSize.height()) ? ((((dstSize.height() - srcSize.height()) / 2) + 1) & ~1) : 0;
  const int outPixelOffset = (srcSize.width() < dstSize.width()) ? ((dstSize.width() - srcSize.width()) / 2) : 0;
  const int outWidth = (srcSize.width() < dstSize.width()) ? srcSize.width() : dstSize.width();

  int line = top;

  switch (src->format().format())
  {
  case SVideoFormat::Format_Invalid:
    break;

  case SVideoFormat::Format_RGB555:
  case SVideoFormat::Format_BGR555:
  case SVideoFormat::Format_RGB565:
  case SVideoFormat::Format_BGR565:
  case SVideoFormat::Format_RGB24:
  case SVideoFormat::Format_BGR24:
  case SVideoFormat::Format_RGB32:
  case SVideoFormat::Format_BGR32:
  case SVideoFormat::Format_GRAY8:
  case SVideoFormat::Format_GRAY16BE:
  case SVideoFormat::Format_GRAY16LE:

  case SVideoFormat::Format_BGGR8:
  case SVideoFormat::Format_GBRG8:
  case SVideoFormat::Format_GRBG8:
  case SVideoFormat::Format_RGGB8:
  case SVideoFormat::Format_BGGR10:
  case SVideoFormat::Format_GBRG10:
  case SVideoFormat::Format_GRBG10:
  case SVideoFormat::Format_RGGB10:
  case SVideoFormat::Format_BGGR16:
  case SVideoFormat::Format_GBRG16:
  case SVideoFormat::Format_GRBG16:
  case SVideoFormat::Format_RGGB16:
    {
      const int sampleSize = dst->format().sampleSize();

      // Draw black bar on top.
      for (; (line < outLineOffset) && (line < bottom); line++)
        ::memset(dst->scanLine(line, 0), 0, dstSize.width() * sampleSize);

      // Copy image data with black bars left and right.
      for (; (line < inHeight + outLineOffset) && (line < bottom); line++)
      {
        const char * const srcLine = src->scanLine(line + inLineOffset - outLineOffset, 0) + (inPixelOffset * sampleSize);
        char * const dstLine = dst->scanLine(line, 0);

        ::memset(dstLine, 0, outPixelOffset * sampleSize);
        ::memcpy(dstLine + (outPixelOffset * sampleSize), srcLine, outWidth * sampleSize);
        ::memset(dstLine + ((outPixelOffset + outWidth) * sampleSize), 0, (dstSize.width() - (outPixelOffset + outWidth)) * sampleSize);
      }

      // Draw black bar on the bottom.
      for (; (line < dstSize.height()) && (line < bottom); line++)
        ::memset(dst->scanLine(line, 0), 0, dstSize.width() * sampleSize);
    }
    break;

  case SVideoFormat::Format_YUYV422:
  case SVideoFormat::Format_UYVY422:
  case SVideoFormat::Format_YUV410P:
  case SVideoFormat::Format_YUV411P:
  case SVideoFormat::Format_YUV420P:
  case SVideoFormat::Format_YUV422P:
  case SVideoFormat::Format_YUV444P:
    {
      int wf = 1, hf = 1;
      if (dst->format().planarYUVRatio(wf, hf))
      {
        // Draw black bar on the top.
        for (; (line < outLineOffset) && (line < bottom); line++)
        {
          ::memset(dst->scanLine(line, 0), 0, dstSize.width());

          if ((line % hf) == 0)
          {
            ::memset(dst->scanLine(line / hf, 1), 0x7F, dstSize.width() / wf);
            ::memset(dst->scanLine(line / hf, 2), 0x7F, dstSize.width() / wf);
          }
        }

        // Copy image data with black bars left and right.
        for (; (line < inHeight + outLineOffset) && (line < bottom); line++)
        {
          const char * const srcLine0 = src->scanLine(line + inLineOffset - outLineOffset, 0) + inPixelOffset;
          char * const dstLine0 = dst->scanLine(line, 0);

          ::memset(dstLine0, 0, outPixelOffset);
          ::memcpy(dstLine0 + outPixelOffset, srcLine0, outWidth);
          ::memset(dstLine0 + (outPixelOffset + outWidth), 0, dstSize.width() - (outPixelOffset + outWidth));

          if ((line % hf) == 0)
          {
            const char * const srcLine1 = src->scanLine((line + inLineOffset - outLineOffset) / hf, 1) + (inPixelOffset / wf);
            char * const dstLine1 = dst->scanLine(line / hf, 1);

            ::memset(dstLine1, 0x7F, outPixelOffset / wf);
            ::memcpy(dstLine1 + (outPixelOffset / wf), srcLine1, outWidth / wf);
            ::memset(dstLine1 + (outPixelOffset / wf) + (outWidth / wf), 0x7F, (dstSize.width() / wf) - ((outPixelOffset / wf) + (outWidth / wf)));

            const char * const srcLine2 = src->scanLine((line + inLineOffset - outLineOffset) / hf, 2) + (inPixelOffset / wf);
            char * const dstLine2 = dst->scanLine(line / hf, 2);

            ::memset(dstLine2, 0x7F, outPixelOffset / wf);
            ::memcpy(dstLine2 + (outPixelOffset / wf), srcLine2, outWidth / wf);
            ::memset(dstLine2 + (outPixelOffset / wf) + (outWidth / wf), 0x7F, (dstSize.width() / wf) - ((outPixelOffset / wf) + (outWidth / wf)));
          }
        }

        // Draw black bar on the bottom.
        for (; (line < dstSize.height()) && (line < bottom); line++)
        {
          ::memset(dst->scanLine(line, 0), 0, dstSize.width());

          if ((line % hf) == 0)
          {
            ::memset(dst->scanLine(line / hf, 1), 0x7F, dstSize.width() / wf);
            ::memset(dst->scanLine(line / hf, 2), 0x7F, dstSize.width() / wf);
          }
        }
      }
    }
    break;
  }
}

} // End of namespace
