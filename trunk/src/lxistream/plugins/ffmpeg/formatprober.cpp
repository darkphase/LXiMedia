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
  static const int maxProbeTime = 250; // msec

  struct ProduceCallback : SInterfaces::BufferReader::ProduceCallback
  {
    SEncodedVideoBufferList videoBuffers;
    bool waitForKeyFrame;

    ProduceCallback(void) : waitForKeyFrame(true)
    {
    }

    static void produceBuffers(SEncodedVideoBufferList *videoBuffers, BufferReader *bufferReader, QTime *timer, int count)
    {
      while ((videoBuffers->count() < count) && (qAbs(timer->elapsed()) < maxProbeTime))
      if (!bufferReader->process(true))
        break;
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
    QTime timer;
    timer.start();

    // Detect format.
    QByteArray data(SInterfaces::FormatProber::defaultProbeSize, 0);
    data.resize(readCallback->read(reinterpret_cast<uchar *>(data.data()), data.size()));

    qDebug() << "A" << timer.elapsed();

    QList<SInterfaces::FormatProber::Format> formats = FormatProber::probeFormat(data, pi.filePath);
    if (!formats.isEmpty())
    {
      pi.format = formats.first().name;

      qDebug() << "B" << timer.elapsed();

      BufferReader bufferReader(QString::null, this);
      if (bufferReader.openFormat(pi.format))
      {
        readCallback->seek(0, SEEK_SET);

        qDebug() << "C" << timer.elapsed();

        ProduceCallback produceCallback;
        if (bufferReader.start(readCallback, &produceCallback, 0, false))
        {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
          pi.title = bestOf(pi.title, SStringParser::removeControl(bufferReader.readMetadata("title")).trimmed());
          pi.author = bestOf(pi.author, SStringParser::removeControl(bufferReader.readMetadata("author")).trimmed());
          pi.copyright = bestOf(pi.copyright, SStringParser::removeControl(bufferReader.readMetadata("copyright")).trimmed());
          pi.comment = bestOf(pi.comment, SStringParser::removeControl(bufferReader.readMetadata("comment")).trimmed());
          pi.album = bestOf(pi.album, SStringParser::removeControl(bufferReader.readMetadata("album")).trimmed());
          pi.genre = bestOf(pi.genre, SStringParser::removeControl(bufferReader.readMetadata("genre")).trimmed());
          pi.year = bestOf(pi.year, bufferReader.readMetadata("year").toInt());
          pi.track = bestOf(pi.track, bufferReader.readMetadata("track").toInt());
#else
          pi.title = bestOf(pi.title, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->title)).trimmed());
          pi.author = bestOf(pi.author, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->author)).trimmed());
          pi.copyright = bestOf(pi.copyright, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->copyright)).trimmed());
          pi.comment = bestOf(pi.comment, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->comment)).trimmed());
          pi.album = bestOf(pi.album, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->album)).trimmed());
          pi.genre = bestOf(pi.genre, SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->genre)).trimmed());
          pi.year = bestOf(pi.year, bufferReader.context()->year);
          pi.track = bestOf(pi.track, bufferReader.context()->track);
#endif

          if (pi.programs.isEmpty())
            pi.programs.append(ProbeInfo::Program(0));

          ProbeInfo::Program &program = pi.programs.first();

          const STime duration = bufferReader.duration();
          if (duration.isValid() && (duration > program.duration))
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

          qDebug() << "D" << timer.elapsed();

          // Get thumbnails
          for (QList<VideoStreamInfo>::Iterator videoStream = program.videoStreams.begin();
               videoStream != program.videoStreams.end();
               videoStream++)
          {
            static const int bufferCount = 25;
            static const int minDist = 80;

            bufferReader.selectStreams(QVector<StreamId>() << (*videoStream));
            produceCallback.produceBuffers(&produceCallback.videoBuffers, &bufferReader, &timer, bufferCount);
            bufferReader.setPosition(program.duration / 20);

            qDebug() << "E" << timer.elapsed();

            // Extract the thumbnail
            if (!produceCallback.videoBuffers.isEmpty() &&
                !produceCallback.videoBuffers.first().codec().isNull())
            {
              VideoDecoder videoDecoder(QString::null, this);
              if (videoDecoder.openCodec(
                      produceCallback.videoBuffers.first().codec(),
                      &bufferReader,
                      VideoDecoder::Flag_KeyframesOnly | VideoDecoder::Flag_Fast))
              {
                SVideoBuffer thumbnail;
                int bestDist = -1, counter = 0;

                while (!produceCallback.videoBuffers.isEmpty() &&
                       (bestDist < minDist) &&
                       (qAbs(timer.elapsed()) < maxProbeTime))
                {
                  const SEncodedVideoBufferList videoBuffers = produceCallback.videoBuffers;
                  produceCallback.videoBuffers.clear();

                  QFuture<void> future = QtConcurrent::run(&ProduceCallback::produceBuffers, &produceCallback.videoBuffers, &bufferReader, &timer, bufferCount);

                  foreach (const SEncodedVideoBuffer &encoded, videoBuffers)
                  foreach (const SVideoBuffer &decoded, videoDecoder.decodeBuffer(encoded))
                  if (!decoded.isNull())
                  {
                    // Get all greyvalues
                    QVector<quint8> pixels;
                    pixels.reserve(4096);
                    for (unsigned y=0, n=decoded.format().size().height(), i=n/32; y<n; y+=i)
                    {
                      const quint8 * const line = reinterpret_cast<const quint8 *>(decoded.scanLine(y, 0));
                      for (int x=0, n=decoded.format().size().width(), i=n/32; x<n; x+=i)
                        pixels += line[x];
                    }

                    qSort(pixels);
                    const int dist = (counter / 10) + (int(pixels[pixels.count() * 3 / 4]) - int(pixels[pixels.count() / 4]));
                    if (dist >= bestDist)
                    {
                      thumbnail = decoded;
                      bestDist = dist;
                    }

                    counter++;
                  }

                  future.waitForFinished();
                }

                qDebug() << "F" << timer.elapsed() << bestDist << counter;

                const SInterval frameRate = thumbnail.format().frameRate();
                if (qAbs(videoStream->codec.frameRate().toFrequency() - frameRate.toFrequency()) > 0.1f)
                  videoStream->codec.setFrameRate(frameRate);

                // Build thumbnail
                if (program.thumbnail.isEmpty() && !thumbnail.isNull())
                {
                  VideoResizer videoResizer("bilinear", this);
                  videoResizer.setSize(SSize(128, 128));
                  videoResizer.setAspectRatioMode(Qt::KeepAspectRatio);
                  thumbnail = videoResizer.processBuffer(thumbnail);

                  VideoEncoder videoEncoder(QString::null, this);
                  if (videoEncoder.openCodec(
                      SVideoCodec("MJPEG", thumbnail.format().size(), SInterval::fromFrequency(25)),
                      NULL,
                      SInterfaces::VideoEncoder::Flag_LowQuality))
                  {
                    const SEncodedVideoBufferList coded = videoEncoder.encodeBuffer(thumbnail);
                    if (!coded.isEmpty())
                      program.thumbnail = QByteArray((const char *)coded.first().data(), coded.first().size());
                  }
                }
              }
            }

            qDebug() << "G" << timer.elapsed();
          }

          // Subtitle streams may not be visible after reading some data (as the
          // first subtitle usually appears after a few minutes).
          const QList<DataStreamInfo> dataStreams = bufferReader.dataStreams();
          if (!dataStreams.isEmpty())
            program.dataStreams = dataStreams;

          bufferReader.stop();

          qDebug() << "H" << timer.elapsed();
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
