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
  : SInterfaces::FormatProber(parent),
    lastFilePath(":"),
    bufferReader(NULL),
    lastIoDevice(NULL),
    waitForKeyFrame(true)
{
}

FormatProber::~FormatProber()
{
  if (bufferReader)
  {
    bufferReader->stop();
    delete bufferReader;
  }
}

QList<FormatProber::Format> FormatProber::probeFormat(const QByteArray &buffer, const QString &filePath)
{
  if (lastFilePath != filePath)
  {
    formats.clear();

    QByteArray fileName("");
    if (!filePath.isEmpty())
      fileName = QFileInfo(filePath).fileName().toUtf8();

    ::AVProbeData probeData;
    probeData.filename = fileName.data();
    probeData.buf = (uchar *)buffer.data();
    probeData.buf_size = buffer.size();

    ::AVInputFormat * format = ::av_probe_input_format(&probeData, true);
    if (format)
      formats += Format(format->name, 0);

    lastFilePath = filePath;
  }

  return formats;
}

void FormatProber::probeFormat(ProbeInfo &pi, QIODevice *ioDevice)
{
  BufferReader * const bufferReader = createBufferReader(ioDevice, pi.filePath);
  if (bufferReader && !formats.isEmpty())
  {
    pi.format.format = formats.first().name;
    pi.format.fileTypeName = bufferReader->formatName();

    for (int title=0, n=bufferReader->numTitles();
         (title<n) && (pi.format.fileType!=ProbeInfo::FileType_Video);
         title++)
    {
      if (!bufferReader->audioStreams(title).isEmpty() &&
          !bufferReader->videoStreams(title).isEmpty())
      {
        pi.format.fileType = ProbeInfo::FileType_Video;
      }
      else if (!bufferReader->audioStreams(title).isEmpty())
        pi.format.fileType = ProbeInfo::FileType_Audio;
    }

    setMetadata(pi, bufferReader);

    pi.isFormatProbed = true;
  }
}

void FormatProber::probeContent(ProbeInfo &pi, QIODevice *ioDevice, const QSize &thumbSize)
{
//  QTime timer; timer.start();

  BufferReader * const bufferReader = createBufferReader(ioDevice, pi.filePath);
  if (bufferReader && !formats.isEmpty())
  {
    for (int title=0, n=bufferReader->numTitles(); title<n; title++)
    {
      while (pi.content.titles.count() <= title)
        pi.content.titles += ProbeInfo::Title();

      ProbeInfo::Title &mainTitle = pi.content.titles[title];

      const STime duration = bufferReader->duration();
      if (duration.isValid() && (duration > mainTitle.duration))
        mainTitle.duration = duration;

      const QList<Chapter> chapters = bufferReader->chapters();
      if (!chapters.isEmpty())
        mainTitle.chapters = chapters;

      const QList<AudioStreamInfo> audioStreams = bufferReader->audioStreams(title);
      if (!audioStreams.isEmpty())
        mainTitle.audioStreams = audioStreams;

      const QList<VideoStreamInfo> videoStreams = bufferReader->videoStreams(title);
      if (!videoStreams.isEmpty())
        mainTitle.videoStreams = videoStreams;

      // Get thumbnail
      if (!videoStreams.isEmpty())
      {
        static const int minBufferCount = 25;
        static const int maxBufferCount = 50;
        static const int minDist = 80;
        int iter = 4096;

        bufferReader->selectStreams(title, QVector<StreamId>() << videoStreams.first());

        while ((videoBuffers.count() < minBufferCount) && (--iter > 0))
        {
          if (!bufferReader->process())
            break;

          while (!videoBuffers.isEmpty() && videoBuffers.first().codec().isNull())
          {
            videoBuffers.takeFirst();
          }
        }

        // Skip first 5%
        if (!formats.first().name.contains("mpegts")) // mpegts has very slow seeking.
          bufferReader->setPosition(mainTitle.duration / 20);

        // Extract the thumbnail
        if (!videoBuffers.isEmpty() && !videoBuffers.first().codec().isNull())
        {
          VideoDecoder videoDecoder(QString::null, this);
          if (videoDecoder.openCodec(
                  videoBuffers.first().codec(),
                  bufferReader,
                  VideoDecoder::Flag_Fast))
          {
            SVideoBuffer thumbnail;
            int bestDist = -1, counter = 0;

            do
            {
              while (!videoBuffers.isEmpty())
              foreach (const SVideoBuffer &decoded, videoDecoder.decodeBuffer(videoBuffers.takeFirst()))
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
              while ((videoBuffers.count() < minBufferCount) && (--iter > 0))
              {
                if (!bufferReader->process())
                  break;
              }
            }
            while (!videoBuffers.isEmpty() && (bestDist < minDist) && (counter < maxBufferCount));

            // Build thumbnail
            if (!thumbnail.isNull())
            {
              VideoResizer videoResizer("bilinear", this);
              videoResizer.setSize(thumbSize);
              videoResizer.setAspectRatioMode(Qt::KeepAspectRatio);
              mainTitle.thumbnail = videoResizer.processBuffer(thumbnail);
            }
          }
        }

        videoBuffers.clear();

        // Subtitle streams may not be visible after reading some data (as the
        // first subtitle usually appears after a few minutes).
        const QList<DataStreamInfo> dataStreams = bufferReader->dataStreams(title);
        if (!dataStreams.isEmpty())
          mainTitle.dataStreams = dataStreams;
      }
    }

    setMetadata(pi, bufferReader);

    pi.isContentProbed = true;
  }

//  qDebug() << pi.filePath << pi.format << timer.elapsed();
}

void FormatProber::produce(const SEncodedAudioBuffer &)
{
}

void FormatProber::produce(const SEncodedVideoBuffer &videoBuffer)
{
  if (!waitForKeyFrame || videoBuffer.isKeyFrame())
  {
    videoBuffers += videoBuffer;
    waitForKeyFrame = false;
  }
}

void FormatProber::produce(const SEncodedDataBuffer &)
{
}

BufferReader * FormatProber::createBufferReader(QIODevice *ioDevice, const QString &filePath)
{
  if (ioDevice != lastIoDevice)
  {
    if (bufferReader)
    {
      bufferReader->stop();
      delete bufferReader;
      bufferReader = NULL;
    }

    foreach (
        const SInterfaces::FormatProber::Format &format,
        FormatProber::probeFormat(ioDevice->peek(defaultProbeSize), filePath))
    {
      bufferReader = new BufferReader(QString::null, this);
      if (bufferReader->openFormat(format.name))
      if (bufferReader->start(ioDevice, this, false, true))
        break;

      delete bufferReader;
      bufferReader = NULL;
    }

    lastIoDevice = ioDevice;
  }
  else if (bufferReader)
    bufferReader->setPosition(STime::null);

  videoBuffers.clear();
  waitForKeyFrame = true;

  return bufferReader;
}

void FormatProber::setMetadata(ProbeInfo &pi, const BufferReader *bufferReader)
{
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  for (AVDictionaryEntry *e = av_dict_get(bufferReader->context()->metadata, "", NULL, AV_DICT_IGNORE_SUFFIX);
       e != NULL;
       e = av_dict_get(bufferReader->context()->metadata, "", e, AV_DICT_IGNORE_SUFFIX))
  {
    setMetadata(pi, e->key, QString::fromUtf8(e->value));
  }
#else
  setMetadata(pi, "title", SStringParser::removeControl(QString::fromUtf8(bufferReader->context()->title)).trimmed());
  setMetadata(pi, "author", SStringParser::removeControl(QString::fromUtf8(bufferReader->context()->author)).trimmed());
  setMetadata(pi, "copyright", SStringParser::removeControl(QString::fromUtf8(bufferReader->context()->copyright)).trimmed());
  setMetadata(pi, "comment", SStringParser::removeControl(QString::fromUtf8(bufferReader->context()->comment)).trimmed());
  setMetadata(pi, "album", SStringParser::removeControl(QString::fromUtf8(bufferReader->context()->album)).trimmed());
  setMetadata(pi, "genre", SStringParser::removeControl(QString::fromUtf8(bufferReader->context()->genre)).trimmed());
  setMetadata(pi, "year", bufferReader->context()->year > 0 ? QString::number(bufferReader->context()->year) : QString::null);
  setMetadata(pi, "track", bufferReader->context()->track > 0 ? QString::number(bufferReader->context()->track) : QString::null);
#endif
}

void FormatProber::setMetadata(ProbeInfo &pi, const char *name, const QString &value)
{
  if (!value.isEmpty())
    pi.format.metadata.insert(QString::fromUtf8(name), value);
}

} } // End of namespaces
