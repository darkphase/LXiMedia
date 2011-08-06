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
extern "C" void LXiMediaCenter_MediaPlayerBackend_SlideShowNode_blendImages(
    void * dstData, const void * srcDataA, const void * srcDataB,
    unsigned numPixels, int factor);

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

SlideShowNode::SlideShowNode(SGraph *parent, const SMediaInfoList &files)
  : SInterfaces::SourceNode(parent),
    files(files),
    outSize(768, 576),
    time(STime::null),
    currentPicture(-1),
    currentFrame(-1)
{
}

SlideShowNode::~SlideShowNode()
{
  loadFuture.waitForFinished();
  future.waitForFinished();
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
  return STime::fromClock(files.count() * slideFrameCount, frameRate);
}

bool SlideShowNode::start(void)
{
  time = STime::null;

  // Create a silent audio buffer
  const SAudioFormat audioFormat(SAudioFormat::Format_PCM_S16,
                                 SAudioFormat::Channels_Stereo,
                                 48000);

  audioBuffer.setFormat(audioFormat);
  audioBuffer.setNumSamples(audioFormat.sampleRate() / frameRate);
  memset(audioBuffer.data(), 0, audioBuffer.size());

  // Create a black video buffer
  lastBuffer = blackBuffer();

  currentPicture = 0;
  currentFrame = 0;

  if (currentPicture < files.count())
  {
    const SMediaInfo node = files[currentPicture++];
    if (!node.isNull())
      loadFuture = QtConcurrent::run(this, &SlideShowNode::loadImage, node.filePath());
    else
      nextBuffer = blackBuffer();
  }
  else
    currentPicture = -1;

  return true;
}

void SlideShowNode::stop(void)
{
  future.waitForFinished();

  audioBuffer.clear();
  lastBuffer.clear();
  currentBuffer.clear();
  nextBuffer.clear();

  time = STime::null;
  currentPicture = -1;
  currentFrame = -1;
}

void SlideShowNode::process(void)
{
  LXI_PROFILE_WAIT(future.waitForFinished());
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  if (currentFrame == 0)
  {
    if (currentPicture >= 0)
    {
      loadFuture.waitForFinished();
      currentBuffer = nextBuffer;

      // Start loading next
      if (currentPicture < files.count())
      {
        const SMediaInfo node = files[currentPicture++];
        if (!node.isNull())
          loadFuture = QtConcurrent::run(this, &SlideShowNode::loadImage, node.filePath());
        else
          nextBuffer = blackBuffer();
      }
      else if (currentPicture == files.count())
      {
        nextBuffer = blackBuffer();
        currentPicture++;
      }
      else
        currentPicture = -1;
    }
    else
    {
      emit output(SAudioBuffer());
      emit output(SVideoBuffer());
      emit finished();
    }
  }

  if (currentFrame >= 0)
  {
    const int fade = qMin(255, (++currentFrame) * 256 / frameRate);

    future = QtConcurrent::run(this, &SlideShowNode::computeVideoBuffer, lastBuffer, currentBuffer, fade);

    if (currentFrame >= slideFrameCount)
    {
      lastBuffer = currentBuffer;
      currentFrame = 0;
    }
  }
}

void SlideShowNode::loadImage(const QString &fileName)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  SImage img(outSize, QImage::Format_RGB32);

  QPainter p;
  p.begin(&img);
    p.fillRect(img.rect(), Qt::black);
    const float ar = outSize.aspectRatio();

    SImage src(fileName);
    if (!src.isNull())
    {
      QSize size = src.size().size();
      size.scale(int(img.width() * ar), img.height(), Qt::KeepAspectRatio);
      src = src.scaled(int(size.width() / ar), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      p.drawImage((img.width() / 2) - (src.width() / 2),
                  (img.height() / 2) - (src.height() / 2),
                  src);
    }
  p.end();

  nextBuffer = img.toVideoBuffer(SInterval::fromFrequency(frameRate));
}

void SlideShowNode::computeVideoBuffer(const SVideoBuffer &a, const SVideoBuffer &b, int fade)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  SVideoBuffer videoBuffer;
  if (fade <= 0)
    videoBuffer = a;
  else if (fade >= 255)
    videoBuffer = b;
  else
  {
    videoBuffer = SVideoBuffer(a.format());
    LXiMediaCenter_MediaPlayerBackend_SlideShowNode_blendImages(
        videoBuffer.data(),
        a.data(),
        b.data(),
        videoBuffer.size(),
        fade);
  }

  videoBuffer.setTimeStamp(time);
  emit output(videoBuffer);

  audioBuffer.setTimeStamp(time);
  emit output(audioBuffer);

  time += STime(1, SInterval::fromFrequency(frameRate));
}

SVideoBuffer SlideShowNode::blackBuffer(void) const
{
  SImage img(outSize, QImage::Format_RGB32);

  QPainter p;
  p.begin(&img);
    p.fillRect(img.rect(), Qt::black);
  p.end();

  return img.toVideoBuffer(SInterval::fromFrequency(frameRate));
}

} } // End of namespaces
