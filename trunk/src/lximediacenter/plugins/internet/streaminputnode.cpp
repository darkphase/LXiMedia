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

#include "streaminputnode.h"
#include <LXiStreamGui>

namespace LXiMediaCenter {
namespace InternetBackend {

const qint64 StreamInputNode::minBufferingTimeMs = 3000;

StreamInputNode::StreamInputNode(SGraph *parent, const QUrl &url)
  : SInterfaces::SourceNode(parent),
    outSize(768, 576),
    baseFrameRate(SInterval::fromFrequency(25)),
    baseChannelSetup(SAudioFormat::Channels_Stereo),
    networkInput(NULL, url),
    audioDecoder(parent),
    videoDecoder(parent),
    dataDecoder(parent),
    videoGenerator(parent),
    bufferState(false),
    bufferProgress(0.0f),
    bufferingTime(STime::null),
    correctTime()
{
  networkInput.setParent(this);
}

StreamInputNode::~StreamInputNode()
{
  startFuture.waitForFinished();
  future.waitForFinished();
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

bool StreamInputNode::open(bool hasVideo, bool generateVideo)
{
  connect(&networkInput, SIGNAL(bufferState(bool, float)), SLOT(setBufferState(bool, float)));
  connect(&networkInput, SIGNAL(finished()), SIGNAL(finished()));

  if (hasVideo)
  {
    // Audio
    SGraph::connect(&networkInput, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
    SGraph::connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), this, SLOT(correct(SAudioBuffer)));

    // Video
    SGraph::connect(&networkInput, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
    SGraph::connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), this, SLOT(correct(SVideoBuffer)));

    // Data
    SGraph::connect(&networkInput, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
    SGraph::connect(&dataDecoder, SIGNAL(output(SSubpictureBuffer)), this, SLOT(correct(SSubpictureBuffer)));
    SGraph::connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), this, SLOT(correct(SSubtitleBuffer)));
  }
  else if (generateVideo)
  {
    // Audio
    SGraph::connect(&networkInput, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
    SGraph::connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), this, SLOT(correct(SAudioBuffer)));

    // Video
    SGraph::connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &videoGenerator, SLOT(input(SAudioBuffer)));
    SGraph::connect(&videoGenerator, SIGNAL(output(SVideoBuffer)), this, SLOT(correct(SVideoBuffer)));

    // Data
    SGraph::connect(&networkInput, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
    SGraph::connect(&dataDecoder, SIGNAL(output(SSubpictureBuffer)), this, SLOT(correct(SSubpictureBuffer)));
    SGraph::connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), this, SLOT(correct(SSubtitleBuffer)));

    SImage blackImage(outSize, SImage::Format_RGB32);
    blackImage.fill(0);
    videoGenerator.setImage(blackImage);
    videoGenerator.setFrameRate(baseFrameRate);
  }
  else
  {
    // Audio
    SGraph::connect(&networkInput, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
    SGraph::connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), this, SLOT(correct(SAudioBuffer)));
  }

  // Create images to overlay.
  if (hasVideo || generateVideo)
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

  return true;
}

bool StreamInputNode::start(void)
{
  bufferingTime = STime::null;
  correctTime = STime();

  // Open the network stream in the background.
  if (networkInput.open())
  {
    startFuture = QtConcurrent::run(&networkInput, &SNetworkInputNode::start);

    return true;
  }

  return false;
}

void StreamInputNode::stop(void)
{
  startFuture.waitForFinished();
  future.waitForFinished();

  networkInput.stop();

  bufferingImages.clear();

  bufferingTime = STime::null;
  correctTime = STime();
}

bool StreamInputNode::process(void)
{
  LXI_PROFILE_WAIT(future.waitForFinished());
  LXI_PROFILE_FUNCTION(TaskType_VideoProcessing);

  if (bufferState && (bufferingTime.isNull() || (bufferingTime.toMSec() >= minBufferingTimeMs)))
  {
    return networkInput.process();
  }
  else
  {
    if (startFuture.isFinished())
      networkInput.fillBuffer();

    if (bufferingTime.isNull())
      bufferingTimer.setTimeStamp(bufferingTime);

    // Allow slightly faster than realtime.
    const STime correctedTime = STime::fromMSec(qint64(float(bufferingTime.toMSec()) / 1.1f));
    if (correctedTime < bufferingTimer.timeStamp())
    {
      future = QtConcurrent::run(this, &StreamInputNode::computeBufferingFrame, bufferingTime);
      bufferingTime += STime(1, baseFrameRate);
      if (correctTime.isValid())
        correctTime += STime(1, baseFrameRate);
    }
  }

  return false;
}

void StreamInputNode::setBufferState(bool b, float p)
{
  startFuture.waitForFinished();

  bufferState = b;
  bufferProgress = p;
}

void StreamInputNode::correct(SAudioBuffer buffer)
{
  buffer.setTimeStamp(correct(buffer.timeStamp()));

  emit output(buffer);
}

void StreamInputNode::correct(SVideoBuffer buffer)
{
  buffer.setTimeStamp(correct(buffer.timeStamp()));

  emit output(buffer);
}

void StreamInputNode::correct(SSubpictureBuffer buffer)
{
  buffer.setTimeStamp(correct(buffer.timeStamp()));

  emit output(buffer);
}

void StreamInputNode::correct(SSubtitleBuffer buffer)
{
  buffer.setTimeStamp(correct(buffer.timeStamp()));

  emit output(buffer);
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

      if (!qFuzzyCompare(bufferProgress, 0.0f))
      {
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
        filledBar.setWidth(
            0.5f + (float(filledBar.width()) * bufferProgress *
            (float(qMin(time.toMSec(), minBufferingTimeMs)) / float(minBufferingTimeMs))));

        p.setPen(Qt::gray);
        p.setBrush(Qt::gray);
        p.drawRoundedRect(filledBar, radius / 2, radius / 2);
      }
    p.end();

    SVideoBuffer videoBuffer = img.toVideoBuffer(baseFrameRate);
    videoBuffer.setTimeStamp(time);
    emit output(videoBuffer);
  }

  audioBuffer.setTimeStamp(time);
  emit output(audioBuffer);
}

} } // End of namespaces
