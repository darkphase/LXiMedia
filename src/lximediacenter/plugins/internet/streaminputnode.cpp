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

#include "streaminputnode.h"

namespace LXiMediaCenter {
namespace InternetBackend {

StreamInputNode::StreamInputNode(SGraph *parent)
  : SNetworkInputNode(parent),
    outSize(768, 576),
    baseFrameRate(SInterval::fromFrequency(25)),
    baseChannelSetup(SAudioFormat::Channels_Stereo),
    baseSampleRate(48000),
    bufferingTime(STime::null),
    correctTime(),
    streamTime(STime::null),
    startSem(1)
{
}

StreamInputNode::~StreamInputNode()
{
  startSem.acquire();
}

SSize StreamInputNode::size(void) const
{
  return outSize;
}

void StreamInputNode::setSize(const SSize &size)
{
  outSize = size;
}

SInterval StreamInputNode::frameRate(void) const
{
  return baseFrameRate;
}

void StreamInputNode::setFrameRate(const SInterval &f)
{
  baseFrameRate = f;
}

SAudioFormat::Channels StreamInputNode::channelSetup(void) const
{
  return baseChannelSetup;
}

void StreamInputNode::setChannelSetup(SAudioFormat::Channels c)
{
  baseChannelSetup = c;
}

unsigned StreamInputNode::sampleRate(void) const
{
  return baseSampleRate;
}

void StreamInputNode::setSampleRate(unsigned r)
{
  baseSampleRate = r;
}

void StreamInputNode::setUrl(const QUrl &url, bool generateVideo)
{
  if (generateVideo)
  {
    QList< QFuture<SImage> > futures;
    futures += QtConcurrent::run(&SVideoGeneratorNode::drawCorneredImage, outSize);
    for (int i=0; i<180; i+=10)
      futures += QtConcurrent::run(&SVideoGeneratorNode::drawBusyWaitImage, outSize, 180 - i);

    baseImage = futures.takeFirst().result();
    while (!futures.isEmpty())
      bufferingImages += futures.takeFirst().result();
  }

  // Create a silent audio buffer
  const SAudioFormat audioFormat(SAudioFormat::Format_PCM_S16, baseChannelSetup, 48000);

  audioBuffer.setFormat(audioFormat);
  audioBuffer.setNumSamples(audioFormat.sampleRate() / baseFrameRate.toFrequency());
  memset(audioBuffer.data(), 0, audioBuffer.size());

  SNetworkInputNode::setUrl(url);
}

bool StreamInputNode::start(void)
{
  bufferingTime = STime::null;
  correctTime = STime();
  streamTime = STime::null;

  // Open the network stream in the background.
  struct StartThread : QThread
  {
    StartThread(StreamInputNode *parent)
      : QThread(parent), parent(parent)
    {
    }

    virtual void run(void)
    {
      parent->startTask();

      deleteLater();
    }

    StreamInputNode * const parent;
  };

  if (startSem.tryAcquire(1, 0))
    (new StartThread(this))->start();

  return true;
}

void StreamInputNode::stop(void)
{
  startSem.acquire();
  startSem.release();

  SNetworkInputNode::stop();

  bufferingImages.clear();

  bufferingTime = STime::null;
  correctTime = STime();
  streamTime = STime::null;
}

bool StreamInputNode::process(void)
{
  if (bufferReady())
  {
    return SNetworkInputNode::process();
  }
  else
  {
    if (startSem.available() > 0)
      SNetworkInputNode::fillBuffer();

    if (bufferingTime.isNull())
      bufferingTimer.setTimeStamp(STime::fromSec(1));

    if (bufferingTime < bufferingTimer.timeStamp())
    {
      computeBufferingFrame(streamTime);

      bufferingTime += STime(1, baseFrameRate);
      streamTime += STime(1, baseFrameRate);

      if (correctTime.isValid())
        correctTime += STime(1, baseFrameRate);

      return true;
    }
  }

  return false;
}

void StreamInputNode::produce(const SEncodedAudioBuffer &buffer)
{
  SEncodedAudioBuffer outBuffer(buffer);
  outBuffer.setDecodingTimeStamp(correct(buffer.decodingTimeStamp()));
  outBuffer.setPresentationTimeStamp(correct(buffer.presentationTimeStamp()));

  if (outBuffer.decodingTimeStamp().isValid())
    streamTime = outBuffer.decodingTimeStamp() + STime(1, baseFrameRate);
  else if (outBuffer.presentationTimeStamp().isValid())
    streamTime = outBuffer.presentationTimeStamp() + STime(1, baseFrameRate);

  emit SNetworkInputNode::output(outBuffer);
}

void StreamInputNode::produce(const SEncodedVideoBuffer &buffer)
{
  SEncodedVideoBuffer outBuffer(buffer);
  outBuffer.setDecodingTimeStamp(correct(buffer.decodingTimeStamp()));
  outBuffer.setPresentationTimeStamp(correct(buffer.presentationTimeStamp()));

  emit SNetworkInputNode::output(outBuffer);
}

void StreamInputNode::produce(const SEncodedDataBuffer &buffer)
{
  SEncodedDataBuffer outBuffer(buffer);
  outBuffer.setDecodingTimeStamp(correct(buffer.decodingTimeStamp()));
  outBuffer.setPresentationTimeStamp(correct(buffer.presentationTimeStamp()));

  emit SNetworkInputNode::output(outBuffer);
}

STime StreamInputNode::correct(const STime &ts)
{
  if (ts.isValid())
  {
    if (!correctTime.isValid())
    {
      correctTime = bufferingTime - ts;
      bufferingTime = STime::null;
    }

    return ts + correctTime;
  }

  return ts;
}

void StreamInputNode::computeBufferingFrame(const STime &time)
{
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  if (!bufferingImages.isEmpty())
  {
    SImage img = baseImage;

    QPainter p;
    p.begin(&img);
      const QImage overlay = bufferingImages.takeFirst();
      bufferingImages.append(overlay);
      p.drawImage(
          (img.width() / 2) - (overlay.width() / 2),
          (img.height() / 2) - (overlay.height() / 2),
          overlay);

      const QRect progressBar(
          (img.width() / 2) - (img.width() / 16),
          (img.height() / 2) + (img.height() / 3),
          img.width() / 8,
          img.height() / 32);

      const qreal radius = img.width() / 256;

      p.setRenderHint(QPainter::Antialiasing);
      p.setPen(QPen(Qt::gray, 2.0));
      p.setBrush(Qt::transparent);
      p.drawRoundedRect(progressBar, radius, radius);

      QRect filledBar = progressBar.adjusted(radius, radius, -radius, -radius);
      filledBar.setWidth(0.5f + (float(filledBar.width()) * bufferProgress()));

      p.setPen(Qt::gray);
      p.setBrush(Qt::gray);
      p.drawRoundedRect(filledBar, radius / 2, radius / 2);
    p.end();

    SVideoBuffer videoBuffer = img.toVideoBuffer(baseFrameRate);
    videoBuffer.setTimeStamp(time);
    emit output(videoBuffer);
  }

  audioBuffer.setTimeStamp(time);
  emit output(audioBuffer);
}

void StreamInputNode::startTask(void)
{
  SNetworkInputNode::start();

  startSem.release();
}

} } // End of namespaces
