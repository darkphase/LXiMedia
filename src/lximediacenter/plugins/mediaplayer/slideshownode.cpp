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

#include "slideshownode.h"
#include <LXiStreamGui>

// Implemented in slideshowsource.blend.c
extern "C" void LXiMediaCenter_SlideShowNode_blendImages(
    void * dstData, const void * srcDataA, const void * srcDataB,
    unsigned numPixels, int factor) __attribute__((nonnull));


namespace LXiMediaCenter {

SlideShowNode::SlideShowNode(SGraph *parent, const QStringList &pictures)
  : QObject(parent),
    SInterfaces::SourceNode(parent),
    pictures(pictures),
    outSize(768, 576),
    time(STime::null),
    currentPicture(-1),
    nextTime(STime::null),
    fade(0)
{
}

SlideShowNode::~SlideShowNode()
{
  loadFuture.waitForFinished();
}

SSize SlideShowNode::size(void) const
{
  return outSize;
}

void SlideShowNode::setSize(const SSize &size)
{
  outSize = size;
}

STime SlideShowNode::duration(void) const
{
  return STime::fromClock(pictures.count() * slideFrameCount, frameRate);
}

bool SlideShowNode::start(void)
{
  time = STime::null;

  // Create a silent audio buffer
  const SAudioFormat audioFormat(SAudioFormat::Format_PCM_S16,
                                 SAudioFormat::Channel_Stereo,
                                 48000);

  audioBuffer.setFormat(audioFormat);
  audioBuffer.setNumSamples(audioFormat.sampleRate() / frameRate);
  memset(audioBuffer.data(), 0, audioBuffer.size());

  // Create a black video buffer
  videoBuffer = current = next = blackBuffer();

  nextTime = time;
  currentPicture = 0;

  return true;
}

void SlideShowNode::stop(void)
{
  audioBuffer.clear();
  videoBuffer.clear();
  current.clear();
  next.clear();
}

void SlideShowNode::process(void)
{
  if (currentPicture == 0)
  {
    if (graph)
      graph->queue(this, &SlideShowNode::loadImage, currentPicture++, &mutex);
    else
      loadImage(currentPicture++);
  }
  else if (currentPicture <= pictures.count()) // <= because last picture is black.
  {
    for (int i=0; i<slideFrameCount; i++)
    if (graph)
      graph->queue(this, &SlideShowNode::computeVideoBuffer, &mutex);
    else
      computeVideoBuffer();

    if (graph)
      graph->queue(this, &SlideShowNode::loadImage, currentPicture++, &mutex);
    else
      loadImage(currentPicture++);
  }
  else
  {
    if (graph)
      graph->queue(this, &SlideShowNode::sendFlush, &mutex);
    else
      sendFlush();
  }
}

void SlideShowNode::loadImage(const int &index)
{
  SImage img(outSize.size(), QImage::Format_RGB32);

  QPainter p;
  p.begin(&img);
    p.fillRect(img.rect(), Qt::black);
    const qreal ar = outSize.aspectRatio();

    if (index < pictures.count())
    {
      SImage src(pictures[index]);
      if (!src.isNull())
      {
        QSize size = src.size();
        size.scale(int(img.width() * ar), img.height(), Qt::KeepAspectRatio);
        src = src.scaled(int(size.width() / ar), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        p.drawImage((img.width() / 2) - (src.width() / 2),
                    (img.height() / 2) - (src.height() / 2),
                    src);
      }
    }
  p.end();

  next = img.toVideoBuffer(ar, SInterval::fromFrequency(frameRate));

  if (index > 0)
    fade = 256;
  else
    videoBuffer = current = next;
}

void SlideShowNode::computeVideoBuffer(void)
{
  if (fade > 0)
  {
    fade -= 16;

    if (fade > 0)
    {
      videoBuffer = SVideoBuffer(next.format());
      LXiMediaCenter_SlideShowNode_blendImages(videoBuffer.data(),
                                               next.data(),
                                               current.data(),
                                               videoBuffer.size(),
                                               fade);
    }
    else
      videoBuffer = current = next;
  }

  videoBuffer.setTimeStamp(time);
  emit output(videoBuffer);

  audioBuffer.setTimeStamp(time);
  emit output(audioBuffer);

  time += STime(1, SInterval::fromFrequency(frameRate));
}

void SlideShowNode::sendFlush(void)
{
  emit output(SAudioBuffer());
  emit output(SVideoBuffer());
  emit finished();
}

SVideoBuffer SlideShowNode::blackBuffer(void) const
{
  SImage img(outSize.size(), QImage::Format_RGB32);

  QPainter p;
  p.begin(&img);
    p.fillRect(img.rect(), Qt::black);
  p.end();

  return img.toVideoBuffer(outSize.aspectRatio(), SInterval::fromFrequency(frameRate));
}

} // End of namespace
