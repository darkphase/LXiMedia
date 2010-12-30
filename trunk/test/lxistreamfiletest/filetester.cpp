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

#include "filetester.h"

void FileTester::testFile(const QString &fileName)
{
  const SMediaInfo mediaInfo(fileName);
  if (mediaInfo.containsAudio())
  {
    FileTester * const tester = new FileTester(fileName);

    if (tester->setup())
    {
      qDebug() << "Starting" << fileName.toAscii().data();

      tester->start();
      tester->wait();

      qDebug() << "Finished" << fileName.toAscii().data();
    }

    delete tester;
  }
}

FileTester::FileTester(const QString &fileName)
  : SGraph(),
    file(this, fileName),
    audioDecoder(this),
    videoDecoder(this),
    dataDecoder(this),
    timeStampResampler(this),
    audioResampler(this, "linear"),
    letterboxDetectNode(this),
    subtitleRenderer(this),
    sync(this)
{
  // File
  connect(&file, SIGNAL(finished()), SLOT(stop()));
  connect(&file, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&file, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&file, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));

  // Audio
  connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &timeStampResampler, SLOT(input(SAudioBuffer)));
  connect(&timeStampResampler, SIGNAL(output(SAudioBuffer)), &audioResampler, SLOT(input(SAudioBuffer)));
  connect(&audioResampler, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));

  // Video
  connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), &timeStampResampler, SLOT(input(SVideoBuffer)));
  connect(&timeStampResampler, SIGNAL(output(SVideoBuffer)), &letterboxDetectNode, SLOT(input(SVideoBuffer)));
  connect(&letterboxDetectNode, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
  connect(&subtitleRenderer, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));

  // Data
  connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), &timeStampResampler, SLOT(input(SSubtitleBuffer)));
  connect(&timeStampResampler, SIGNAL(output(SSubtitleBuffer)), &subtitleRenderer, SLOT(input(SSubtitleBuffer)));
}

bool FileTester::setup(void)
{
  // Select streams
  if (file.open())
  {
    const QList<SIOInputNode::AudioStreamInfo> audioStreams = file.audioStreams();
    const QList<SIOInputNode::VideoStreamInfo> videoStreams = file.videoStreams();
    const QList<SIOInputNode::DataStreamInfo>  dataStreams  = file.dataStreams();

    QList<quint16> selectedStreams;
    if (!audioStreams.isEmpty())
      selectedStreams += audioStreams.first().streamId;

    if (!videoStreams.isEmpty())
      selectedStreams += videoStreams.first().streamId;

    if (!dataStreams.isEmpty())
      selectedStreams += dataStreams.first().streamId;

    file.selectStreams(selectedStreams);

    // Set stream properties
    if (!audioStreams.isEmpty() && !videoStreams.isEmpty())
    {
      const SVideoCodec videoInCodec = videoStreams.first().codec;

      // Graph options.
      timeStampResampler.setFrameRate(SInterval::fromFrequency(15));
      audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
      audioResampler.setChannels(SAudioFormat::Channel_Stereo);
      audioResampler.setSampleRate(48000);

      return true;
    }
    else if (!audioStreams.isEmpty())
    {
      audioResampler.setChannels(SAudioFormat::Channel_Stereo);
      audioResampler.setSampleRate(48000);

      return true;
    }
  }

  return false;
}
