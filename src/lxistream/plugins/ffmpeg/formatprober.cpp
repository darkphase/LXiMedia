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

#include "formatprober.h"
#include "bufferreader.h"
#include "videodecoder.h"
#include "videoencoder.h"
#include "videoresizer.h"

namespace LXiStream {
namespace FFMpegBackend {


FormatProber::FormatProber(const QString &, QObject *parent)
  : SInterfaces::FormatProber(parent)
{
}

FormatProber::~FormatProber()
{
}

QList<FormatProber::Format> FormatProber::probeFormat(const QByteArray &buffer, const QString &filePath)
{
  QByteArray fileName("");
  if (!filePath.isEmpty())
    fileName = QFileInfo(filePath).fileName().toUtf8();

  ::AVProbeData probeData;
  probeData.filename = fileName.data();
  probeData.buf = (uchar *)buffer.data();
  probeData.buf_size = buffer.size();

  ::AVInputFormat * format = ::av_probe_input_format(&probeData, true);
  if (format)
    return QList<Format>() << Format(format->name, 0);

  return QList<Format>();
}

void FormatProber::probeMetadata(ProbeInfo &pi, ReadCallback *readCallback)
{
  struct ProduceCallback : SInterfaces::BufferReader::ProduceCallback
  {
    SEncodedVideoBufferList videoBuffers;
    bool waitForKeyFrame;

    ProduceCallback(void) : waitForKeyFrame(true)
    {
    }

    virtual void produce(const SEncodedAudioBuffer &)
    {
    }

    virtual void produce(const SEncodedVideoBuffer &videoBuffer)
    {
      if (!waitForKeyFrame || videoBuffer.isKeyFrame())
      {
        videoBuffers += videoBuffer;
        waitForKeyFrame = false;
      }
    }

    virtual void produce(const SEncodedDataBuffer &)
    {
    }
  };

  if (readCallback)
  {
    // Detect format.
    QByteArray data(SInterfaces::FormatProber::defaultProbeSize, 0);
    data.resize(readCallback->read(reinterpret_cast<uchar *>(data.data()), data.size()));

    QList<SInterfaces::FormatProber::Format> formats = FormatProber::probeFormat(data, readCallback->path);
    if (!formats.isEmpty())
    {
      pi.format = formats.first().name;

      BufferReader bufferReader(QString::null, this);
      if (bufferReader.openFormat(pi.format))
      {
        readCallback->seek(0, SEEK_SET);

        ProduceCallback produceCallback;
        if (bufferReader.start(readCallback, &produceCallback, 0, false))
        {
          pi.title = bestOf(pi.title, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->title)).trimmed());
          pi.author = bestOf(pi.author, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->author)).trimmed());
          pi.copyright = bestOf(pi.copyright, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->copyright)).trimmed());
          pi.comment = bestOf(pi.comment, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->comment)).trimmed());
          pi.album = bestOf(pi.album, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->album)).trimmed());
          pi.genre = bestOf(pi.genre, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->genre)).trimmed());
          pi.year = bestOf(pi.year, bufferReader.context()->year);
          pi.track = bestOf(pi.track, bufferReader.context()->track);

          if (pi.programs.isEmpty())
            pi.programs.append(ProbeInfo::Program());

          ProbeInfo::Program &program = pi.programs.first();

          const STime duration = bufferReader.duration();
          if (duration.isValid())
            program.duration = duration;

          const QList<Chapter> chapters = bufferReader.chapters();
          if (!chapters.isEmpty())
            program.chapters = chapters;

          const QList<AudioStreamInfo> audioStreams = bufferReader.audioStreams();
          if (!audioStreams.isEmpty())
            program.audioStreams = audioStreams;

          const QList<VideoStreamInfo> videoStreams = bufferReader.videoStreams();
          if (!videoStreams.isEmpty())
            program.videoStreams = videoStreams;

          pi.isProbed = true;
          pi.isReadable = true;

          // Get thumbnails
          for (QList<VideoStreamInfo>::Iterator videoStream = program.videoStreams.begin();
               videoStream != program.videoStreams.end();
               videoStream++)
          {
            bufferReader.selectStreams(QList<StreamId>() << (*videoStream));

            // Read the first minute
            {
              STime firstTime;
              for (int i=0, lc=0; (i<128) && (produceCallback.videoBuffers.count()<2048); i++)
              {
                if (!bufferReader.process(true))
                  break;

                if (produceCallback.videoBuffers.count() != lc)
                {
                  const SEncodedVideoBuffer &last = produceCallback.videoBuffers.last();
                  const STime lastTime = last.presentationTimeStamp().isValid()
                                         ? last.presentationTimeStamp()
                                         : last.decodingTimeStamp();

                  if (firstTime.isValid())
                    firstTime = qMin(firstTime, lastTime);
                  else
                    firstTime = lastTime;

                  if ((lastTime - firstTime).toSec() >= 60)
                    break;

                  if (last.isKeyFrame())
                  while (produceCallback.videoBuffers.count() > 1)
                    produceCallback.videoBuffers.takeFirst();

                  i = 0;
                  lc = produceCallback.videoBuffers.count();
                }
              }

              for (int i=0; (i<4096) && (produceCallback.videoBuffers.count()<128); i++)
              if (!bufferReader.process(true))
                break;
            }

            if (!produceCallback.videoBuffers.isEmpty() &&
                !produceCallback.videoBuffers.first().codec().isNull())
            {
              VideoDecoder videoDecoder(QString::null, this);
              if (videoDecoder.openCodec(produceCallback.videoBuffers.first().codec()))
              {
                SVideoBuffer bestThumb;
                int bestDist = 0, counter = 0;

                foreach (const SEncodedVideoBuffer &coded, produceCallback.videoBuffers)
                foreach (const SVideoBuffer &thumb, videoDecoder.decodeBuffer(coded))
                {
                  // Get all greyvalues
                  QVector<quint8> pixels;
                  pixels.reserve(4096);
                  for (unsigned y=0, n=thumb.format().size().height(), i=n/32; y<n; y+=i)
                  {
                    const quint8 * const line = reinterpret_cast<const quint8 *>(thumb.scanLine(y, 0));
                    for (int x=0, n=thumb.format().size().width(), i=n/32; x<n; x+=i)
                      pixels += line[x];
                  }

                  qSort(pixels);
                  const int dist = (counter++ / 10) + (int(pixels[pixels.count() * 3 / 4]) - int(pixels[pixels.count() / 4]));
                  if (dist >= bestDist)
                  {
                    bestThumb = thumb;
                    bestDist = dist;
                  }
                }

                const SInterval frameRate = bestThumb.format().frameRate();
                if (qAbs(videoStream->codec.frameRate().toFrequency() - frameRate.toFrequency()) > 0.1f)
                  videoStream->codec.setFrameRate(frameRate);

                // Build thumbnail
                if (program.thumbnail.isEmpty() && !bestThumb.isNull())
                {
                  VideoResizer videoResizer("bilinear", this);
                  videoResizer.setSize(SSize(128, 128));
                  videoResizer.setAspectRatioMode(Qt::KeepAspectRatio);
                  bestThumb = videoResizer.processBuffer(bestThumb);

                  VideoEncoder videoEncoder(QString::null, this);
                  if (videoEncoder.openCodec(SVideoCodec("MJPEG", bestThumb.format().size(), SInterval::fromFrequency(25)),
                                             SInterfaces::VideoEncoder::Flag_LowQuality))
                  {
                    const SEncodedVideoBufferList thumbnail = videoEncoder.encodeBuffer(bestThumb);
                    if (!thumbnail.isEmpty())
                      program.thumbnail = QByteArray((const char *)thumbnail.first().data(), thumbnail.first().size());
                  }
                }
              }
            }
          }

          // Data streams are set now, as subtitles are not all detected before
          // scanning the file.
          const QList<DataStreamInfo> dataStreams = bufferReader.dataStreams();
          if (!dataStreams.isEmpty())
            program.dataStreams = dataStreams;

          bufferReader.stop();
        }
      }
    }
  }
}

QString FormatProber::bestOf(const QString &a, const QString &b)
{
  if (a.length() > b.length())
    return a;
  else
    return b;
}

unsigned FormatProber::bestOf(unsigned a, unsigned b)
{
  return qMax(a, b);
}


} } // End of namespaces
