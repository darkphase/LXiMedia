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

#include "playlistnode.h"
#include <LXiStreamGui>

namespace LXiMediaCenter {

PlaylistNode::PlaylistNode(SGraph *parent, MediaDatabase *mediaDatabase, Playlist *activePlaylist)
  : QObject(parent),
    SInterfaces::SourceNode(parent),
    mediaDatabase(mediaDatabase),
    activePlaylist(activePlaylist),
    outFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channel_Stereo, 48000),
    mutex(QMutex::Recursive),
    input(NULL),
    timeStamp(STime::null),
    videoEnabled(false)
{
  // Seed the random number generator.
  qsrand(int(QDateTime::currentDateTime().toTime_t()));

  // Ensure the images are loaded.
  GlobalSettings::productLogo().bits();
  defaultBackground().bits();
}

PlaylistNode::~PlaylistNode()
{
  delete input;
}

SAudioFormat::Channels PlaylistNode::channels(void) const
{
  return outFormat.channelSetup();
}

void PlaylistNode::setChannels(SAudioFormat::Channels c)
{
  outFormat.setChannelSetup(c);
}

unsigned PlaylistNode::sampleRate(void) const
{
  return outFormat.sampleRate();
}

void PlaylistNode::setSampleRate(unsigned s)
{
  outFormat.setSampleRate(s);
}

void PlaylistNode::enableVideo(const SSize &size, SInterval frameRate)
{
  videoEnabled = true;
  videoSize = size;
  videoFrameRate = frameRate;
}

Playlist * PlaylistNode::playlist(void)
{
  return activePlaylist;
}

MediaDatabase::UniqueID PlaylistNode::currentSong(void) const
{
  SDebug::MutexLocker ml(&mutex, __FILE__, __LINE__);

  if (input)
    return input->song;

  return 0;
}

bool PlaylistNode::start(void)
{
  timeStamp = STime::null;
  videoTime = STime::null;
  nextSong();

  return true;
}

void PlaylistNode::stop(void)
{
  if (input)
  {
    input->file.stop();
    delete input;
    input = NULL;
  }
}

void PlaylistNode::process(void)
{
  SDebug::MutexLocker ml(&mutex, __FILE__, __LINE__);

  if (graph)
    graph->runTask(this, &PlaylistNode::computeBuffer, input);
  else
    computeBuffer(input);
}

void PlaylistNode::nextSong(void)
{
  SDebug::MutexLocker ml(&mutex, __FILE__, __LINE__);

  if (graph)
    graph->runTask(this, &PlaylistNode::closeInput, input);
  else
    closeInput(input);

  input = NULL;

  // Repeat several times (e.g. if a file was deleted).
  for (unsigned i=0; (i<32) && (input==NULL); i++)
  {
    const MediaDatabase::UniqueID uid = activePlaylist->checkout();
    if (uid != 0)
      playSong(uid, mediaDatabase->readNode(uid));
    else
      break;
  }

  if (input == NULL)
    emit finished();
}

void PlaylistNode::computeBuffer(Input *input)
{
  if (input)
  {
    input->file.process();

    while (!inputQueue.isEmpty())
    {
      SAudioBuffer audioBuffer = inputQueue.takeFirst();
      const STime baseTime = audioBuffer.timeStamp();
      audioBuffer.setTimeStamp(timeStamp);
      emit output(audioBuffer);

      timeStamp += STime::fromClock(audioBuffer.numSamples(), outFormat.sampleRate());

      if (videoEnabled && (videoTime < timeStamp))
      {
        Q_ASSERT(!videoBuffer.isNull() && !videoBufferText.isEmpty());

        QString time = QTime().addSecs(baseTime.toSec()).toString("m:ss");
        if (input->duration.isPositive())
          time += " / " + QTime().addSecs(input->duration.toSec()).toString("m:ss");

        if (time != videoBufferText.first())
        {
          videoBufferText[0] = time;
          videoBufferSub = SSubtitleRenderNode::renderSubtitles(videoBuffer, videoBufferText);
        }

        videoBufferSub.setTimeStamp(videoTime);
        emit output(videoBufferSub);

        videoTime += STime(1, videoFrameRate);
      }
    }
  }
}

void PlaylistNode::closeInput(Input *input)
{
  if (input)
  {
    input->file.stop();
    delete input;
  }
}

void PlaylistNode::playSong(const MediaDatabase::UniqueID &uid, const SMediaInfo &node)
{
  if (!node.isNull())
  {
    mediaDatabase->setLastPlayed(node);

    Q_ASSERT(input == NULL);
    input = new Input(uid, node.filePath());
    input->duration = node.duration();

    if (SAudioFormat::numChannels(outFormat.channelSetup()) <= 6)
      input->audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

    input->audioResampler.setChannels(outFormat.channelSetup());
    input->audioResampler.setSampleRate(outFormat.sampleRate());

    if (input->file.start())
    {
      connect(&(input->file), SIGNAL(finished()), SLOT(nextSong()), Qt::DirectConnection);
      connect(&(input->file), SIGNAL(output(SEncodedAudioBuffer)), &(input->audioDecoder), SLOT(input(SEncodedAudioBuffer)), Qt::DirectConnection);
      connect(&(input->audioDecoder), SIGNAL(output(SAudioBuffer)), &(input->audioResampler), SLOT(input(SAudioBuffer)), Qt::DirectConnection);
      connect(&(input->audioResampler), SIGNAL(output(SAudioBuffer)), SLOT(decoded(SAudioBuffer)), Qt::DirectConnection);

      if (videoEnabled)
      {
        if (graph)
          graph->runTask(this, &PlaylistNode::computeSplash, uid, node);
        else
          computeSplash(uid, node);
      }

      qDebug() << "PlaylistNode: playing file" << node.filePath();
    }
    else
    {
      delete input;
      input = NULL;
    }
  }
}

void PlaylistNode::computeSplash(const MediaDatabase::UniqueID &uid, const SMediaInfo &node)
{
  QImage thumb;
  if (!node.thumbnails().isEmpty())
    thumb = QImage::fromData(node.thumbnails().first());

  if (thumb.isNull())
  foreach (MediaDatabase::UniqueID uid, mediaDatabase->allFilesInDirOf(uid))
  {
    const SMediaInfo node = mediaDatabase->readNode(uid);
    if (!node.thumbnails().isEmpty())
    {
      thumb = QImage::fromData(node.thumbnails().first());
      if (!thumb.isNull())
        break;
    }
  }

  QImage darkThumb;
  if (!thumb.isNull())
    darkThumb = thumb;
  else
    darkThumb = defaultBackground();

  if (!darkThumb.isNull())
  {
    QPainter p;
    p.begin(&darkThumb);
      p.fillRect(darkThumb.rect(), QColor(0, 0, 0, 192));
    p.end();
  }

  SImage img(videoSize.size(), QImage::Format_RGB32);
  QPainter p;
  p.begin(&img);
    p.fillRect(img.rect(), Qt::black);

    const qreal ar = videoSize.aspectRatio();

    if (!darkThumb.isNull())
    {
      QSize size = darkThumb.size();
      size.scale(int(img.width() * ar), img.height(), Qt::KeepAspectRatioByExpanding);
      darkThumb = darkThumb.scaled(int(size.width() / ar), size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

      const QPoint pos((img.width() / 2) - (darkThumb.width() / 2), (img.height() / 2) - (darkThumb.height() / 2));
      p.drawImage(pos, darkThumb);
    }

    const int bsv = qMin(img.width() / 16, img.height() / 16);
    const int bsh = int(bsv / ar);

    const QColor bc(255, 255, 255, 64);
    p.fillRect(0, 0, bsh * 4, bsv * 2, bc);
    p.fillRect(0, 0, bsh * 2, bsv * 4, bc);
    p.fillRect(img.width(), img.height(), -bsh * 4, -bsv * 2, bc);
    p.fillRect(img.width(), img.height(), -bsh * 2, -bsv * 4, bc);

    QImage logo = GlobalSettings::productLogo();
    if ((img.height() != 1080) || !qFuzzyCompare(ar, 1.0))
    {
      logo = logo.scaled(int((logo.width() * img.height() / 1080) / ar),
                         logo.height() * img.height() / 1080,
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    p.drawImage(bsh, img.height() - logo.height() - bsv, logo);

    if (!thumb.isNull())
    {
      if ((img.height() < 576) || !qFuzzyCompare(ar, 1.0))
      {
        thumb = thumb.scaled(int((thumb.width() * img.height() / 576) / ar),
                             thumb.height() * img.height() / 576,
                             Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      }

      const QPoint pos((img.width() * 2 / 3) - (thumb.width() / 2), (img.height() / 3) - (thumb.height() / 2));
      p.drawImage(pos, thumb);

      p.setPen(QPen(Qt::white, 2.0f));
      p.setBrush(Qt::transparent);
      p.drawRect(QRect(pos, thumb.size()));
    }
  p.end();

  // The subtitle renderer is used here to render the text as on Unix fonts are
  // not available if X is not loaded.
  videoBufferText.clear();
  videoBufferText += QString::null;
  videoBufferText += QString::null;
  videoBufferText += node.title();
  videoBufferText += node.author();
  if (!node.album().isEmpty())
    videoBufferText.last() += " - " + node.album();

  videoBuffer = img.toVideoBuffer(videoSize.aspectRatio(), videoFrameRate);
}

const QImage & PlaylistNode::defaultBackground(void)
{
  static const QImage background(":/mediaplayer/music-background.jpeg");

  return background;
}

void PlaylistNode::decoded(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull())
    inputQueue += audioBuffer;
}

} // End of namespace
