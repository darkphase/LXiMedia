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

SlideShowNode::SlideShowNode(SGraph *parent, const QList<QUrl> &files)
  : SInterfaces::SourceNode(parent),
    files(files),
    outSize(1280, 720),
    slideFrameCount(180),
    time(STime::null),
    currentPicture(-1),
    currentFrame(-1)
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

STime SlideShowNode::slideDuration(void) const
{
  return STime::fromClock(slideFrameCount, frameRate);
}

void SlideShowNode::setSlideDuration(const STime &time)
{
  slideFrameCount = time.toClock(frameRate);
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

  baseImage = SVideoGeneratorNode::drawCorneredImage(outSize);

  // Create a black video buffer
  lastBuffer = baseImage;

  currentPicture = 0;
  currentFrame = 0;

  loadFuture = QtConcurrent::run(this, &SlideShowNode::loadNextImage);

  return true;
}

void SlideShowNode::stop(void)
{
  loadFuture.waitForFinished();

  audioBuffer.clear();
  lastBuffer.clear();
  currentBuffer.clear();
  nextBuffer.clear();

  time = STime::null;
  currentPicture = -2;
  currentFrame = -1;
}

bool SlideShowNode::process(void)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  if (currentFrame == 0)
  {
    loadFuture.waitForFinished();

    if (currentPicture >= 0)
    {
      currentBuffer = nextBuffer;
      loadFuture = QtConcurrent::run(this, &SlideShowNode::loadNextImage);
    }
    else if (currentPicture == -1)
    {
      currentBuffer = nextBuffer;
      currentPicture = -2;
    }
    else
    {
      currentFrame = -1;

      emit output(SAudioBuffer());
      emit output(SVideoBuffer());
      emit finished();
      return true;
    }
  }

  if (currentFrame >= 0)
  {
    const int fade = qMin(255, (++currentFrame) * 256 / frameRate);

    computeVideoBuffer(lastBuffer, currentBuffer, fade);

    if (currentFrame >= slideFrameCount)
    {
      lastBuffer = currentBuffer;
      currentFrame = 0;
    }

    return true;
  }

  return false;
}

void SlideShowNode::loadNextImage(void)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  const float ar = outSize.aspectRatio();
  const QSize baseSize = outSize.absoluteSize();

  while (currentPicture < files.count())
  {
    SImage src(files[currentPicture++], baseSize);
    if (!src.isNull())
    {
      SImage img = baseImage;

      QPainter p;
      p.begin(&img);
        QSize size = src.size().size();
        size.scale(baseSize, Qt::KeepAspectRatio);

        const QImage srcs =
            src.scaled(int(size.width() / ar), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        p.drawImage((img.width() / 2) - (srcs.width() / 2),
                    (img.height() / 2) - (srcs.height() / 2),
                    srcs);
      p.end();

      nextBuffer = img.toVideoBuffer(SInterval::fromFrequency(frameRate));
      return;
    }
  }

  currentPicture = -1;
  nextBuffer = baseImage;
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

} } // End of namespaces
