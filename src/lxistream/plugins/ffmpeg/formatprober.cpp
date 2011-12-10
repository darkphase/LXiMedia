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

void FormatProber::probeFormat(ProbeInfo &pi, QIODevice *ioDevice)
{
  foreach (const SInterfaces::FormatProber::Format &format, FormatProber::probeFormat(ioDevice->peek(defaultProbeSize), pi.filePath))
  {
    BufferReader bufferReader(QString::null, this);
    if (bufferReader.openFormat(format.name))
    {
      ioDevice->seek(0);

      if (bufferReader.start(ioDevice, NULL, false, true))
      {
        pi.format = format.name;
        pi.fileTypeName = bufferReader.formatName();

        if (!bufferReader.audioStreams().isEmpty() && !bufferReader.videoStreams().isEmpty())
          pi.fileType = ProbeInfo::FileType_Video;
        else if (!bufferReader.audioStreams().isEmpty())
          pi.fileType = ProbeInfo::FileType_Audio;

        pi.isFormatProbed = true;

        bufferReader.stop();

        return;
      }
    }
  }
}

void FormatProber::probeContent(ProbeInfo &pi, QIODevice *ioDevice, const QSize &thumbSize)
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

//  QTime timer; timer.start();
  const QByteArray buffer = ioDevice->peek(defaultProbeSize);
  foreach (const SInterfaces::FormatProber::Format &format, FormatProber::probeFormat(buffer, pi.filePath))
  {
    BufferReader bufferReader(QString::null, this);
    if (bufferReader.openFormat(format.name))
    {
      ioDevice->seek(0);

      ProduceCallback produceCallback;
      if (bufferReader.start(ioDevice, &produceCallback, false, true))
      {
        const STime duration = bufferReader.duration();
        if (duration.isValid() && (duration > pi.duration))
          pi.duration = duration;

        const QList<Chapter> chapters = bufferReader.chapters();
        if (!chapters.isEmpty())
          pi.chapters = chapters;

        const QList<AudioStreamInfo> audioStreams = bufferReader.audioStreams();
        if (!audioStreams.isEmpty())
          pi.audioStreams = audioStreams;

        const QList<VideoStreamInfo> videoStreams = bufferReader.videoStreams();
        if (!videoStreams.isEmpty())
          pi.videoStreams = videoStreams;

        pi.format = format.name;
        pi.fileTypeName = bufferReader.formatName();

        if (!audioStreams.isEmpty() && !videoStreams.isEmpty())
          pi.fileType = ProbeInfo::FileType_Video;
        else if (!audioStreams.isEmpty())
          pi.fileType = ProbeInfo::FileType_Audio;

        pi.isFormatProbed = true;

        // Get thumbnail
        if (!videoStreams.isEmpty())
        {
          static const int minBufferCount = 25;
          static const int maxBufferCount = 32;
          static const int minDist = 80;
          int iter = 4096;

          bufferReader.selectStreams(QVector<StreamId>() << videoStreams.first());

          while ((produceCallback.videoBuffers.count() < minBufferCount) && (--iter > 0))
          {
            if (!bufferReader.process())
              break;

            while (!produceCallback.videoBuffers.isEmpty() &&
                   produceCallback.videoBuffers.first().codec().isNull())
            {
              produceCallback.videoBuffers.takeFirst();
            }
          }

          // Skip first 5%
          if (!pi.format.contains("mpegts")) // mpegts has very slow seeking.
            bufferReader.setPosition(pi.duration / 20);

          // Extract the thumbnail
          if (!produceCallback.videoBuffers.isEmpty() &&
              !produceCallback.videoBuffers.first().codec().isNull())
          {
            VideoDecoder videoDecoder(QString::null, this);
            if (videoDecoder.openCodec(
                    produceCallback.videoBuffers.first().codec(),
                    &bufferReader,
                    VideoDecoder::Flag_Fast))
            {
              SVideoBuffer thumbnail;
              int bestDist = -1, counter = 0;

              do
              {
                const SEncodedVideoBufferList videoBuffers = produceCallback.videoBuffers;
                produceCallback.videoBuffers.clear();

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

                if (bestDist < minDist)
                while ((produceCallback.videoBuffers.count() < minBufferCount) && (--iter > 0))
                {
                  if (!bufferReader.process())
                    break;
                }
              }
              while (!produceCallback.videoBuffers.isEmpty() && (bestDist < minDist) && (counter < maxBufferCount));

              // Build thumbnail
              if (!thumbnail.isNull())
              {
                VideoResizer videoResizer("bilinear", this);
                videoResizer.setSize(thumbSize);
                videoResizer.setAspectRatioMode(Qt::KeepAspectRatio);
                pi.thumbnail = videoResizer.processBuffer(thumbnail);
              }
            }
          }

          // Subtitle streams may not be visible after reading some data (as the
          // first subtitle usually appears after a few minutes).
          const QList<DataStreamInfo> dataStreams = bufferReader.dataStreams();
          if (!dataStreams.isEmpty())
            pi.dataStreams = dataStreams;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
          for (AVDictionaryEntry *e = av_dict_get(bufferReader.context()->metadata, "", NULL, AV_DICT_IGNORE_SUFFIX);
               e != NULL;
               e = av_dict_get(bufferReader.context()->metadata, "", e, AV_DICT_IGNORE_SUFFIX))
          {
            setMetadata(pi, e->key, QString::fromUtf8(e->value));
          }
#else
          setMetadata(pi, "title", SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->title)).trimmed());
          setMetadata(pi, "author", SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->author)).trimmed());
          setMetadata(pi, "copyright", SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->copyright)).trimmed());
          setMetadata(pi, "comment", SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->comment)).trimmed());
          setMetadata(pi, "album", SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->album)).trimmed());
          setMetadata(pi, "genre", SStringParser::removeControl(QString::fromUtf8(bufferReader.context()->genre)).trimmed());
          setMetadata(pi, "year", bufferReader.context()->year > 0 ? QString::number(bufferReader.context()->year) : QString::null);
          setMetadata(pi, "track", bufferReader.context()->track > 0 ? QString::number(bufferReader.context()->track) : QString::null);
#endif
          bufferReader.stop();
        }
      }
    }
  }

//  qDebug() << pi.filePath << pi.format << timer.elapsed();
}

void FormatProber::setMetadata(ProbeInfo &pi, const char *name, const QString &value)
{
  if (!value.isEmpty())
  {
    QMap<QString, QVariant>::Iterator i = pi.metadata.find(name);
    if (i != pi.metadata.end())
    {
      if (i->toString().length() < value.length())
        *i = QVariant(value);
    }
    else
      pi.metadata.insert(QString::fromUtf8(name), value);
  }
}


} } // End of namespaces
