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

#include "nodes/svideoletterboxdetectnode.h"
#include "sgraph.h"
#include "svideobuffer.h"

namespace LXiStream {

struct SVideoLetterboxDetectNode::Data
{
  unsigned                      numFrames;
  qreal                         lastRatio;
  QQueue<qreal>                 ratios;
  SVideoBufferList              buffers;
};

SVideoLetterboxDetectNode::SVideoLetterboxDetectNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->numFrames = 2;
  d->lastRatio = 1.0;
}

SVideoLetterboxDetectNode::~SVideoLetterboxDetectNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

unsigned SVideoLetterboxDetectNode::delayFrames(void) const
{
  return d->numFrames - 1;
}

void SVideoLetterboxDetectNode::setDelayFrames(unsigned f)
{
  d->numFrames = f + 1;
}

void SVideoLetterboxDetectNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull())
  {
    d->buffers.append(videoBuffer);
    d->ratios.enqueue(determineAspectRatio(videoBuffer));

    QQueue<qreal>::ConstIterator i = d->ratios.begin();
    qreal ratio = *i;
    for (i++; i!=d->ratios.end(); i++)
    if (ratio != *i)
    {
      ratio = d->lastRatio;
      break;
    }

    d->lastRatio = ratio;

    while (d->buffers.count() >= int(d->numFrames))
    {
      SVideoBuffer destBuffer = d->buffers.takeFirst();
      d->ratios.dequeue();

      if (d->lastRatio < 1.0f)
      {
        SVideoFormat format = destBuffer.format();
        SSize size = format.size();

        const unsigned newLines =
            qMin(unsigned(size.height()),
                 (unsigned(qreal(size.height()) * d->lastRatio) + 7) & ~0x0Fu);
        const unsigned skipLines = size.height() - newLines;

        size.setHeight(newLines);
        format.setSize(size);
        destBuffer.overrideFormat(format);

        if ((format == SVideoFormat::Format_BGR32) || (format == SVideoFormat::Format_RGB32) ||
            (format == SVideoFormat::Format_YUYV422) || (format == SVideoFormat::Format_UYVY422))
        {
          destBuffer.setOffset(0, destBuffer.offset(0) + (destBuffer.lineSize(0) * (skipLines / 2)));
        }
        else if ((format == SVideoFormat::Format_YUV410P) ||
                 (format == SVideoFormat::Format_YUV411P) ||
                 (format == SVideoFormat::Format_YUV420P) ||
                 (format == SVideoFormat::Format_YUV422P) ||
                 (format == SVideoFormat::Format_YUV444P))
        {
          int yuvW = 0, yuvH = 0;
          format.planarYUVRatio(yuvW, yuvH);

          destBuffer.setOffset(0, destBuffer.offset(0) + (destBuffer.lineSize(0) * (skipLines / 2)));
          destBuffer.setOffset(1, destBuffer.offset(1) + (destBuffer.lineSize(1) * ((skipLines / yuvH) / 2)));
          destBuffer.setOffset(2, destBuffer.offset(2) + (destBuffer.lineSize(2) * ((skipLines / yuvH) / 2)));
        }
      }

      emit output(destBuffer);
    }
  }
}

qreal SVideoLetterboxDetectNode::determineAspectRatio(const SVideoBuffer &videoBuffer) const
{
  static const unsigned numLines = 4;
  static const unsigned numSamplesPerLine = 32;
  static const qreal line[numLines] = { 0.04, 0.10, 0.16, 0.28 };
  static const qreal aspect[numLines-1] = { 0.86, 0.75, 0.57 }; // 14:9, 16:9, 21:9

  const SVideoFormat format = videoBuffer.format();
  const unsigned sampleWidth = format.size().width() / numSamplesPerLine;

  QVector<quint8> pixels;
  pixels.reserve(numSamplesPerLine * numLines * 2);

  quint8 top[numLines][numSamplesPerLine], bot[numLines][numSamplesPerLine];
  memset(top, 0, sizeof(top));
  memset(bot, 0, sizeof(bot));

  if ((format == SVideoFormat::Format_YUYV422) ||
      (format == SVideoFormat::Format_UYVY422))
  {
    for (unsigned i=0; i<numLines; i++)
    {
      const quint8 * const topLine =
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(int(line[i] * qreal(format.size().height())), 0));
      const quint8 * const botLine =
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(int((1.0f - line[i]) * qreal(format.size().height())), 0));

      if (format == SVideoFormat::Format_YUYV422)
      {
        for (unsigned x=0, n=0; n<numSamplesPerLine; x+=(sampleWidth*2), n++)
        {
          pixels += topLine[x];
          top[i][n] = topLine[x];

          pixels += botLine[x];
          bot[i][n] = botLine[x];
        }
      }
      else
      {
        for (unsigned x=1, n=0; n<numSamplesPerLine; x+=(sampleWidth*2), n++)
        {
          pixels += topLine[x];
          top[i][n] = topLine[x];

          pixels += botLine[x];
          bot[i][n] = botLine[x];
        }
      }
    }
  }
  else if ((format == SVideoFormat::Format_YUV410P) ||
           (format == SVideoFormat::Format_YUV411P) ||
           (format == SVideoFormat::Format_YUV420P) ||
           (format == SVideoFormat::Format_YUV422P) ||
           (format == SVideoFormat::Format_YUV444P))
  {
    for (unsigned i=0; i<numLines; i++)
    {
      const quint8 * const topLine =
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(int(line[i] * qreal(format.size().height())), 0));
      const quint8 * const botLine =
          reinterpret_cast<const quint8 *>(videoBuffer.scanLine(int((1.0f - line[i]) * qreal(format.size().height())), 0));

      for (unsigned x=0, n=0; n<numSamplesPerLine; x+=sampleWidth, n++)
      {
        pixels += topLine[x];
        top[i][n] = topLine[x];

        pixels += botLine[x];
        bot[i][n] = botLine[x];
      }
    }
  }
  else if ((format == SVideoFormat::Format_BGR32) ||
           (format == SVideoFormat::Format_RGB32))
  {
    for (unsigned i=0; i<numLines; i++)
    {
      const quint32 * const topLine =
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(int(line[i] * qreal(format.size().height())), 0));
      const quint32 * const botLine =
          reinterpret_cast<const quint32 *>(videoBuffer.scanLine(int((1.0f - line[i]) * qreal(format.size().height())), 0));

      for (unsigned x=0, n=0; n<numSamplesPerLine; x+=sampleWidth, n++)
      {
        const quint8 tv = quint8((topLine[x] >> 16) & 0xFF);
        pixels += tv;
        top[i][n] = tv;

        const quint8 bv = quint8((botLine[x] >> 16) & 0xFF);
        pixels += bv;
        bot[i][n] = bv;
      }
    }
  }

  qSort(pixels);
  
  if (qAbs(int(pixels.first()) - int(pixels.last())) >= 32)
  for (unsigned i=0; i<numLines-1; i++)
  {
    const int offset = (pixels.count() * (i + 1)) / (numLines * 2);
    const quint8 darkVal = quint8(qMin(int(pixels[offset]) + 8, 255));

    unsigned dark = 0, light = 0;
    for (unsigned j=0; j<numSamplesPerLine; j++)
    {
      dark += ((top[i][j] < darkVal) && (bot[i][j] < darkVal)) ? 1 : 0;
      light += ((top[i+1][j] > darkVal) && (bot[i+1][j] > darkVal)) ? 1 : 0;
    }

    if (dark > (numSamplesPerLine - (numSamplesPerLine / 8)))
    {
      if (light > (numSamplesPerLine - (numSamplesPerLine / 4)))
        return aspect[i];
    }
    else if ((i == 0) && (dark < (numSamplesPerLine / 2)))
      return 1.0;
  }
  
  return d->lastRatio;
}


} // End of namespace
