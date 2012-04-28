/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#include "smediainfo.h"
#include "sinterfaces.h"
#include "smediafilesystem.h"
#include "ssubtitlefile.h"
#include <LXiCore>

namespace LXiStream {

const int SMediaInfo::tvShowSeason = 100;

SMediaInfo::SMediaInfo(void)
  : pi(NULL)
{
}

SMediaInfo::SMediaInfo(const SMediaInfo &from)
  : pi(from.pi)
{
}

SMediaInfo::SMediaInfo(const QUrl &filePath)
  : pi(new ProbeInfo())
{
  pi->filePath = filePath;
  pi->isReadable = true;
}

SMediaInfo::SMediaInfo(const QSharedDataPointer<ProbeInfo> &pi)
  : pi(pi)
{
}

SMediaInfo::~SMediaInfo()
{
}

SMediaInfo & SMediaInfo::operator=(const SMediaInfo &from)
{
  pi = from.pi;

  return *this;
}

bool SMediaInfo::isNull(void) const
{
  return pi == NULL;
}

QUrl SMediaInfo::filePath(void) const
{
  return pi->filePath;
}

QString SMediaInfo::fileName(void) const
{
  const QString path = pi->filePath.path();
  
  return path.mid(path.lastIndexOf('/') + 1);
}

QString SMediaInfo::baseName(void) const
{
  const QString name = fileName();
  const int lastdot = name.lastIndexOf('.');

  return lastdot >= 0 ? name.left(lastdot) : name;
}

QUrl SMediaInfo::path(void) const
{
  QUrl result = pi->filePath;

  QString path = result.path();
  path = path.left(path.lastIndexOf('/') + 1);
  result.setPath(path);
  
  return result;
}

SMediaFilesystem SMediaInfo::directory(void) const
{
  return SMediaFilesystem(path());
}

bool SMediaInfo::isReadable(void) const
{
  if (pi->isReadable && !pi->isFileInfoRead)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->isReadable;
}

qint64 SMediaInfo::size(void) const
{
  if (pi->isReadable && !pi->isFileInfoRead)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->fileInfo.size;
}

bool SMediaInfo::isDir(void) const
{
  if (pi->isReadable && !pi->isFileInfoRead)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->fileInfo.isDir;
}

QDateTime SMediaInfo::lastModified(void) const
{
  if (pi->isReadable && !pi->isFileInfoRead)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->fileInfo.lastModified;
}

QString SMediaInfo::format(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->format.format;
}

SMediaInfo::ProbeInfo::FileType SMediaInfo::fileType(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->format.fileType;
}

QString SMediaInfo::fileTypeName(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->format.fileTypeName;
}

QVariant SMediaInfo::metadata(const QString &key) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  QMap<QString, QVariant>::ConstIterator i = pi->content.metadata.find(key);
  if (i != pi->content.metadata.end())
    return i.value();

  return QVariant();
}

const QList<SMediaInfo::ProbeInfo::Title> & SMediaInfo::titles(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->content.titles;
}

bool SMediaInfo::isFileInfoRead(void) const
{
  return pi->isFileInfoRead;
}

bool SMediaInfo::isFormatProbed(void) const
{
  return pi->isFormatProbed;
}

bool SMediaInfo::isContentProbed(void) const
{
  return pi->isContentProbed;
}

void SMediaInfo::readFileInfo(void)
{
  const SMediaFilesystem mediaDir(path());

  const SMediaFilesystem::Info info = mediaDir.readInfo(fileName());
  pi->fileInfo.isDir = info.isDir;
  pi->fileInfo.size = info.size;
  pi->fileInfo.lastModified = info.lastModified;
  pi->isReadable = info.isReadable;

  pi->isFileInfoRead = true;
}

void SMediaInfo::probeFormat(void)
{
  if (pi->isReadable && !pi->isFileInfoRead)
    readFileInfo();

  if (pi->isReadable)
  {
    if (!pi->fileInfo.isDir)
    {
      const SMediaFilesystem mediaDir(path());

      QIODevice * const ioDevice = mediaDir.openFile(fileName());
      if (ioDevice)
      {
        const QByteArray buffer =
            ioDevice->read(SInterfaces::FormatProber::defaultProbeSize);

        foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
        {
          if (!pi->isFormatProbed)
            prober->readFormat(*pi, buffer);

          delete prober;
        }

        pi->isFormatProbed = true;
        delete ioDevice;
      }
    }
    else
    {
      foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
      {
        if (!pi->isFormatProbed)
          prober->readFormat(*pi, QByteArray());

        delete prober;
      }

      pi->isFormatProbed = true;
    }
  }
}

void SMediaInfo::probeContent(void)
{
  if (pi->isReadable && !pi->isFormatProbed)
    probeFormat();

  if (pi->isReadable)
  {
    if (!pi->fileInfo.isDir)
    {
      const SMediaFilesystem mediaDir(path());

      QIODevice * const ioDevice = mediaDir.openFile(fileName());
      if (ioDevice)
      {
        foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
        {
          if (ioDevice->seek(0))
          if (!pi->isContentProbed)
            prober->readContent(*pi, ioDevice);

          delete prober;
        }

        probeDataStreams();

        pi->isContentProbed = true;
        delete ioDevice;
      }
    }
    else
    {
      foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
      {
        if (!pi->isContentProbed)
          prober->readContent(*pi, NULL);

        delete prober;
      }

      pi->isContentProbed = true;
    }
  }
}

SVideoBuffer SMediaInfo::readThumbnail(const QSize &size)
{
  if (pi->isReadable && !pi->isFormatProbed)
    probeFormat();

  SVideoBuffer result;

  if (pi->isReadable)
  {
    const SMediaFilesystem mediaDir(path());

    QIODevice * const ioDevice = mediaDir.openFile(fileName());
    if (ioDevice)
    {
      foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
      {
        if (result.isNull())
          result = prober->readThumbnail(*pi, ioDevice, size);

        delete prober;
      }

      delete ioDevice;
    }
  }

  return result;
}

void SMediaInfo::serialize(QXmlStreamWriter &writer) const
{
  struct T  { static const char * trueFalse(bool b) { return b ? "true" : "false"; } };

  writer.writeStartElement("mediainfo");

  writer.writeAttribute("filepath", pi->filePath.toString());
  writer.writeAttribute("isfileinforead", T::trueFalse(pi->isFileInfoRead));
  writer.writeAttribute("isformatprobed", T::trueFalse(pi->isFormatProbed));
  writer.writeAttribute("iscontentprobed", T::trueFalse(pi->isContentProbed));

  if (pi->isFileInfoRead)
  {
    writer.writeStartElement("fileinfo");

    writer.writeAttribute("isdir", T::trueFalse(pi->fileInfo.isDir));
    writer.writeAttribute("size", QString::number(pi->fileInfo.size));
    writer.writeAttribute("lastmodified", pi->fileInfo.lastModified.toString(Qt::ISODate));

    writer.writeEndElement();
  }

  if (pi->isFormatProbed)
  {
    writer.writeStartElement("format");

    writer.writeAttribute("format", SStringParser::removeControl(pi->format.format));
    writer.writeAttribute("filetype", QString::number(pi->format.fileType));
    writer.writeAttribute("filetypename", SStringParser::removeControl(pi->format.fileTypeName));

    writer.writeEndElement();
  }

  if (pi->isContentProbed)
  {
    writer.writeStartElement("content");

    writer.writeStartElement("metadata");

    for (QMap<QString, QVariant>::ConstIterator i = pi->content.metadata.begin();
         i != pi->content.metadata.end();
         i++)
    {
      writer.writeTextElement(i.key(), SStringParser::removeControl(i.value().toString()));
    }

    writer.writeEndElement();

    foreach (const ProbeInfo::Title &title, pi->content.titles)
    {
      writer.writeStartElement("title");

      if (title.duration.isValid())
        writer.writeAttribute("duration", QString::number(title.duration.toMSec()));

      foreach (const Chapter &chapter, title.chapters)
      {
        writer.writeStartElement("chapter");

        writer.writeAttribute("title", chapter.title);
        writer.writeAttribute("begin", QString::number(chapter.begin.toMSec()));
        writer.writeAttribute("end", QString::number(chapter.end.toMSec()));

        writer.writeEndElement();
      }

      foreach (const AudioStreamInfo &audioStream, title.audioStreams)
      {
        writer.writeStartElement("audiostream");

        writer.writeAttribute("id", audioStream.toString());
        writer.writeAttribute("language", audioStream.language);
        writer.writeAttribute("title", audioStream.title);
        audioStream.codec.serialize(writer);

        writer.writeEndElement();
      }

      foreach (const VideoStreamInfo &videoStream, title.videoStreams)
      {
        writer.writeStartElement("videostream");

        writer.writeAttribute("id", videoStream.toString());
        writer.writeAttribute("language", videoStream.language);
        writer.writeAttribute("title", videoStream.title);
        videoStream.codec.serialize(writer);

        writer.writeEndElement();
      }

      foreach (const DataStreamInfo &dataStream, title.dataStreams)
      {
        writer.writeStartElement("datastream");

        writer.writeAttribute("id", dataStream.toString());
        writer.writeAttribute("language", dataStream.language);
        writer.writeAttribute("title", dataStream.title);
        writer.writeAttribute("file", dataStream.file.toString());
        dataStream.codec.serialize(writer);

        writer.writeEndElement();
      }

      if (!title.imageCodec.isNull())
      {
        writer.writeStartElement("imagecodec");

        title.imageCodec.serialize(writer);

        writer.writeEndElement();
      }

      writer.writeEndElement();
    }

    writer.writeEndElement();
  }

  writer.writeEndElement();
}

bool SMediaInfo::deserialize(QXmlStreamReader &reader)
{
  struct T { static bool trueFalse(const QStringRef &s) { return s == "true"; }  };

  pi = new ProbeInfo();
  pi->isReadable = false;

  if (reader.name() == "mediainfo")
  {
    pi->filePath = reader.attributes().value("filepath").toString();
    pi->isFileInfoRead = T::trueFalse(reader.attributes().value("isfileinforead"));
    pi->isFormatProbed = T::trueFalse(reader.attributes().value("isformatprobed"));
    pi->isContentProbed = T::trueFalse(reader.attributes().value("iscontentprobed"));

    while (reader.readNextStartElement())
    {
      if (reader.name() == "fileinfo")
      {
        pi->fileInfo.isDir = T::trueFalse(reader.attributes().value("isdir"));
        pi->fileInfo.size = reader.attributes().value("size").toString().toLongLong();
        pi->fileInfo.lastModified = QDateTime::fromString(reader.attributes().value("lastmodified").toString(), Qt::ISODate);

        reader.skipCurrentElement();
      }
      else if (reader.name() == "format")
      {
        pi->format.format = reader.attributes().value("format").toString().toAscii();
        pi->format.fileType = ProbeInfo::FileType(reader.attributes().value("filetype").toString().toInt());
        pi->format.fileTypeName = reader.attributes().value("filetypename").toString();

        reader.skipCurrentElement();
      }
      else if (reader.name() == "content")
      {
        while (reader.readNextStartElement())
        {
          if (reader.name() == "metadata")
          {
            while (reader.readNextStartElement())
              pi->content.metadata.insert(reader.name().toString(), reader.readElementText());
          }
          else if (reader.name() == "title")
          {
            pi->content.titles += ProbeInfo::Title();
            ProbeInfo::Title &title = pi->content.titles.last();

            if (reader.attributes().hasAttribute("duration"))
              title.duration = STime::fromMSec(reader.attributes().value("duration").toString().toLongLong());

            while (reader.readNextStartElement())
            {
              if (reader.name() == "chapter")
              {
                title.chapters += Chapter();
                Chapter &chapter = title.chapters.last();

                chapter.title = reader.attributes().value("title").toString();
                chapter.begin = STime::fromMSec(reader.attributes().value("begin").toString().toLongLong());
                chapter.end = STime::fromMSec(reader.attributes().value("end").toString().toLongLong());
              }
              else if (reader.name() == "audiostream")
              {
                const StreamId id = StreamId::fromString(reader.attributes().value("id").toString());
                const QString language = reader.attributes().value("language").toString();
                const QString xtitle = reader.attributes().value("title").toString();

                reader.readNextStartElement();
                SAudioCodec codec;
                if (codec.deserialize(reader))
                  title.audioStreams += AudioStreamInfo(id, language, xtitle, codec);
              }
              else if (reader.name() == "videostream")
              {
                const StreamId id = StreamId::fromString(reader.attributes().value("id").toString());
                const QString language = reader.attributes().value("language").toString();
                const QString xtitle = reader.attributes().value("title").toString();

                reader.readNextStartElement();
                SVideoCodec codec;
                if (codec.deserialize(reader))
                  title.videoStreams += VideoStreamInfo(id, language, xtitle, codec);
              }
              else if (reader.name() == "datastream")
              {
                const StreamId id = StreamId::fromString(reader.attributes().value("id").toString());
                const QString language = reader.attributes().value("language").toString();
                const QString xtitle = reader.attributes().value("title").toString();
                const QString file = reader.attributes().value("file").toString();

                reader.readNextStartElement();
                SDataCodec codec;
                if (codec.deserialize(reader))
                  title.dataStreams += DataStreamInfo(id, language, xtitle, codec, file);
              }
              else if (reader.name() == "imagecodec")
              {
                reader.readNextStartElement();
                SVideoCodec codec;
                if (codec.deserialize(reader))
                  title.imageCodec = codec;
              }

              reader.skipCurrentElement();
            }
          }
          else
            reader.skipCurrentElement();
        }
      }
      else
        reader.skipCurrentElement();
    }

    return true;
  }

  reader.raiseError("Not a mediainfo element.");
  return false;
}

void SMediaInfo::probeDataStreams(void)
{
  if (pi->content.titles.count() == 1)
  {
    ProbeInfo::Title &title = pi->content.titles.first();

    if (!title.videoStreams.isEmpty())
    {
      // Remove subtitles.
      QList<DataStreamInfo> subs;
      for (QList<DataStreamInfo>::Iterator i=title.dataStreams.begin();
           i != title.dataStreams.end(); )
      if (!i->file.isEmpty())
      {
        subs += *i;
        i = title.dataStreams.erase(i);
      }
      else
        i++;

      // Add subtitles with new ID (To ensure IDs match SFileInputNode).
      quint16 nextStreamId = 0xF000;
      foreach (const QUrl &filePath, SSubtitleFile::findSubtitleFiles(pi->filePath))
      {
        bool found = false;
        for (QList<DataStreamInfo>::Iterator i=subs.begin(); i!=subs.end(); i++)
        if (i->file == filePath)
        {
          i->type = DataStreamInfo::Type_Subtitle;
          i->id = nextStreamId++;
          title.dataStreams += *i;

          found = true;
          break;
        }

        if (!found)
        {
          SSubtitleFile file(filePath);
          if (file.open())
          {
            title.dataStreams += DataStreamInfo(
                StreamId(DataStreamInfo::Type_Subtitle, nextStreamId++),
                file.language(),
                QString::null,
                file.codec(),
                filePath);
          }
        }
      }
    }
  }
}

} // End of namespace
