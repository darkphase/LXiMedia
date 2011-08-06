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

#include "nodes/svideogeneratornode.h"
#include "simage.h"

namespace LXiStreamGui {

struct SVideoGeneratorNode::Data
{
  SImage                        image;
  SInterval                     frameRate;
  SVideoBuffer                  videoBuffer;
  STime                         videoTime;
};

SVideoGeneratorNode::SVideoGeneratorNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->frameRate = SInterval::fromFrequency(25);
}

SVideoGeneratorNode::~SVideoGeneratorNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SVideoGeneratorNode::setImage(const SImage &image)
{
  d->image = image;
}

const SImage & SVideoGeneratorNode::image(void) const
{
  return d->image;
}

void SVideoGeneratorNode::setFrameRate(const SInterval &frameRate)
{
  d->frameRate = frameRate;
}

const SInterval & SVideoGeneratorNode::frameRate(void) const
{
  return d->frameRate;
}

bool SVideoGeneratorNode::start(void)
{
  if (!d->image.isNull())
  {
    d->videoBuffer = d->image.toVideoBuffer(SInterval::fromFrequency(25));
    d->videoBuffer.setTimeStamp(STime::null);
  }

  return true;
}

void SVideoGeneratorNode::stop(void)
{
  d->videoBuffer = SVideoBuffer();
}

void SVideoGeneratorNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  if (!audioBuffer.isNull())
  {
    if (!d->videoTime.isValid() || (qAbs(d->videoTime - audioBuffer.timeStamp()).toSec() >= 3))
      d->videoTime = audioBuffer.timeStamp();

    while (d->videoTime <= audioBuffer.timeStamp())
    {
      if (!d->videoBuffer.isNull())
      {
        d->videoBuffer.setTimeStamp(d->videoTime);
        emit output(d->videoBuffer);
      }

      d->videoTime += d->frameRate;
    }
  }
}

SImage SVideoGeneratorNode::drawCorneredImage(const SSize &size)
{
  SImage img(size, QImage::Format_RGB32);
  img.fill(0);

  QPainter p(&img);

  const int blockSize = qMin(size.width(), size.height()) / 8;
  const int blockWidth = blockSize / size.aspectRatio();
  const int blockHeight = blockSize;

  const QRect block1(-blockWidth, -blockHeight, blockWidth * 2, blockHeight * 2);
  QRadialGradient gradient1(0, 0, blockSize);
  gradient1.setColorAt(0.0, Qt::gray);
  gradient1.setColorAt(1.0, Qt::transparent);

  p.setPen(Qt::transparent);
  p.setBrush(gradient1);
  p.drawPie(block1, 270 * 16, 90 * 16);

  const QRect block2 = block1.translated(size.width(), size.height());
  QRadialGradient gradient2(size.width(), size.height(), blockSize);
  gradient2.setColorAt(0.0, Qt::gray);
  gradient2.setColorAt(1.0, Qt::transparent);

  p.setPen(Qt::transparent);
  p.setBrush(gradient2);
  p.drawPie(block2,  90 * 16, 90 * 16);

  return img;
}

SImage SVideoGeneratorNode::drawBusyWaitImage(const SSize &size, int angle)
{
  const int blockSize = qMin(size.width(), size.height()) / 2;

  SImage img(SSize(blockSize, blockSize, size.aspectRatio()), QImage::Format_ARGB32);
  img.fill(0);

  QPainter p(&img);

  QRadialGradient gradient(img.width() / 2, img.height() / 2, blockSize / 2);
  gradient.setColorAt(0.0, Qt::gray);
  gradient.setColorAt(1.0, Qt::transparent);

  p.setPen(Qt::transparent);
  p.setBrush(gradient);
  p.drawPie(img.rect(),  90 * 16, 90 * 16);
  p.drawPie(img.rect(), 270 * 16, 90 * 16);

  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(Qt::white);
  p.setBrush(Qt::white);

  const QRectF outerRect = img.rect().adjusted(blockSize / 16, blockSize / 16, -blockSize / 16, -blockSize / 16);
  const QRectF innerRect = outerRect.adjusted(blockSize / 16, blockSize / 16, -blockSize / 16, -blockSize / 16);

  QPainterPath path1;
  path1.arcMoveTo(outerRect, angle);
  path1.arcTo(outerRect, angle, 90);
  path1.arcTo(innerRect, angle + 90, -90);
  path1.closeSubpath();
  p.drawPath(path1);

  QPainterPath path2;
  path2.arcMoveTo(outerRect, angle + 180);
  path2.arcTo(outerRect, angle + 180, 90);
  path2.arcTo(innerRect, angle + 270, -90);
  path2.closeSubpath();
  p.drawPath(path2);

  return img;
}

} // End of namespace
