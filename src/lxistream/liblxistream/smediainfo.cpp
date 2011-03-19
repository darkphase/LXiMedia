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

#include "smediainfo.h"
#include "sinterfaces.h"
#include "sstringparser.h"
#include "ssubtitlefile.h"

namespace LXiStream {

const unsigned SMediaInfo::tvShowSeason = 100u;

SMediaInfo::SMediaInfo(void)
  : pi(NULL)
{
}

SMediaInfo::SMediaInfo(const SMediaInfo &from)
  : pi(from.pi)
{
}

SMediaInfo::SMediaInfo(const QString &path)
  : pi(new SInterfaces::FormatProber::ProbeInfo())
{
  probe(path);
}

SMediaInfo::SMediaInfo(const QSharedDataPointer<SInterfaces::FormatProber::ProbeInfo> &pi)
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

QDomNode SMediaInfo::toXml(QDomDocument &doc) const
{
  return toXml(*pi, doc);
}

void SMediaInfo::fromXml(const QDomNode &elm)
{
  pi = new SInterfaces::FormatProber::ProbeInfo();

  fromXml(*pi, elm);
  probeDataStreams();
}

bool SMediaInfo::isNull(void) const
{
  return pi == NULL;
}

QString SMediaInfo::filePath(void) const
{
  return pi->filePath;
}

QString SMediaInfo::fileName(void) const
{
  return QFileInfo(pi->filePath).fileName();
}

QString SMediaInfo::baseName(void) const
{
  return QFileInfo(pi->filePath).completeBaseName();
}

QString SMediaInfo::path(void) const
{
  if (pi->path.isEmpty())
    return QFileInfo(pi->filePath).path();
  else
    return pi->path;
}

qint64 SMediaInfo::size(void) const
{
  return pi->size;
}

QDateTime SMediaInfo::lastModified(void) const
{
  return pi->lastModified;
}

QString SMediaInfo::format(void) const
{
  return pi->format;
}

STime SMediaInfo::totalDuration(void) const
{
  STime result = STime::null;

  foreach (const Program &p, pi->programs)
  if (p.duration.isValid())
    result += p.duration;

  return result.isPositive() ? result : STime();
}

bool SMediaInfo::containsAudio(void) const
{
  foreach (const Program &p, pi->programs)
  if (!p.audioStreams.isEmpty())
    return true;

  return false;
}

bool SMediaInfo::containsVideo(void) const
{
  foreach (const Program &p, pi->programs)
  if (!p.videoStreams.isEmpty())
    return true;

  return false;
}

bool SMediaInfo::containsImage(void) const
{
  foreach (const Program &p, pi->programs)
  if (!p.imageCodec.isNull())
    return true;

  return false;
}

bool SMediaInfo::isProbed(void) const
{
  return pi->isProbed;
}

bool SMediaInfo::isReadable(void) const
{
  return pi->isReadable;
}

QString SMediaInfo::fileTypeName(void) const
{
  return pi->fileTypeName;
}

const QList<SMediaInfo::Program> & SMediaInfo::programs(void) const
{
  return pi->programs;
}

QString SMediaInfo::title(void) const
{
  if (pi->title.isEmpty())
    return SStringParser::toCleanName(QFileInfo(pi->filePath).completeBaseName());
  else
    return pi->title;
}

QString SMediaInfo::author(void) const
{
  return pi->author;
}

QString SMediaInfo::copyright(void) const
{
  return pi->copyright;
}

QString SMediaInfo::comment(void) const
{
  return pi->comment;
}

QString SMediaInfo::album(void) const
{
  return pi->album;
}

QString SMediaInfo::genre(void) const
{
  return pi->genre;
}

unsigned SMediaInfo::year(void) const
{
  return pi->year;
}

unsigned SMediaInfo::track(void) const
{
  return pi->track;
}

QDomNode SMediaInfo::toXml(const SInterfaces::FormatProber::ProbeInfo &pi, QDomDocument &doc)
{
  QDomElement mediaInfoElm = createElement(doc, "mediainfo");

  mediaInfoElm.setAttribute("filePath", pi.filePath);
  mediaInfoElm.setAttribute("size", pi.size);
  mediaInfoElm.setAttribute("lastModified", pi.lastModified.toString(Qt::ISODate));

  mediaInfoElm.setAttribute("format", SStringParser::removeControl(pi.format));

  if (pi.isProbed)         mediaInfoElm.setAttribute("probed", trueFalse(pi.isProbed));
  if (pi.isReadable)       mediaInfoElm.setAttribute("readable", trueFalse(pi.isReadable));

  mediaInfoElm.setAttribute("type", SStringParser::removeControl(pi.fileTypeName));

  foreach (const Program &program, pi.programs)
  {
    QDomElement programElm = createElement(doc, "program");

    if (!program.title.isEmpty())
      programElm.setAttribute("title", SStringParser::removeControl(program.title));

    if (program.duration.isValid())
      programElm.setAttribute("duration", QString::number(program.duration.toMSec()));

    foreach (const Chapter &chapter, program.chapters)
    {
      QDomElement elm = createElement(doc, "chapter");
      elm.setAttribute("title", chapter.title);
      elm.setAttribute("begin", chapter.begin.toUSec());
      elm.setAttribute("end", chapter.end.toUSec());
      programElm.appendChild(elm);
    }

    foreach (const AudioStreamInfo &audioStream, program.audioStreams)
    {
      QDomElement elm = createElement(doc, "audiostream");
      elm.setAttribute("id", audioStream.streamId());
      elm.setAttribute("language", audioStream.language);
      elm.appendChild(audioStream.codec.toXml(doc));
      programElm.appendChild(elm);
    }

    foreach (const VideoStreamInfo &videoStream, program.videoStreams)
    {
      QDomElement elm = createElement(doc, "videostream");
      elm.setAttribute("id", videoStream.streamId());
      elm.setAttribute("language", videoStream.language);
      elm.appendChild(videoStream.codec.toXml(doc));
      programElm.appendChild(elm);
    }

    foreach (const DataStreamInfo &dataStream, program.dataStreams)
    {
      QDomElement elm = createElement(doc, "datastream");
      elm.setAttribute("type", dataStream.streamType());
      elm.setAttribute("id", dataStream.streamId());
      elm.setAttribute("language", dataStream.language);
      elm.setAttribute("file", dataStream.file);
      elm.appendChild(dataStream.codec.toXml(doc));
      programElm.appendChild(elm);
    }

    if (!program.imageCodec.isNull())
    {
      QDomElement elm = createElement(doc, "imagecodec");
      elm.appendChild(program.imageCodec.toXml(doc));
      programElm.appendChild(elm);
    }

    QDomElement thumbElm = doc.createElement("thumbnail");
    thumbElm.appendChild(doc.createTextNode(program.thumbnail.toBase64()));
    programElm.appendChild(thumbElm);

    mediaInfoElm.appendChild(programElm);
  }

  if (!pi.title.isEmpty())     mediaInfoElm.setAttribute("title", SStringParser::removeControl(pi.title));
  if (!pi.author.isEmpty())    mediaInfoElm.setAttribute("author", SStringParser::removeControl(pi.author));
  if (!pi.copyright.isEmpty()) mediaInfoElm.setAttribute("copyright", SStringParser::removeControl(pi.copyright));
  if (!pi.comment.isEmpty())   mediaInfoElm.setAttribute("comment", SStringParser::removeControl(pi.comment));
  if (!pi.album.isEmpty())     mediaInfoElm.setAttribute("album", SStringParser::removeControl(pi.album));
  if (!pi.genre.isEmpty())     mediaInfoElm.setAttribute("genre", SStringParser::removeControl(pi.genre));
  if (pi.year != 0)            mediaInfoElm.setAttribute("year", pi.year);
  if (pi.track != 0)           mediaInfoElm.setAttribute("track", pi.track);

  return mediaInfoElm;
}

void SMediaInfo::fromXml(SInterfaces::FormatProber::ProbeInfo &pi, const QDomNode &elm)
{
  QDomElement mediaInfoElm = findElement(elm, "mediainfo");

  pi.filePath = mediaInfoElm.attribute("filePath");
  pi.size = mediaInfoElm.attribute("size").toLongLong();
  pi.lastModified = QDateTime::fromString(mediaInfoElm.attribute("lastModified"), Qt::ISODate);

  pi.format = mediaInfoElm.attribute("format");

  pi.isProbed = trueFalse(mediaInfoElm.attribute("probed"));
  pi.isReadable = trueFalse(mediaInfoElm.attribute("readable"));

  pi.fileTypeName = mediaInfoElm.attribute("type");

  for (QDomElement programElm = mediaInfoElm.firstChildElement("program");
       !programElm.isNull();
       programElm = programElm.nextSiblingElement("program"))
  {
    Program program;

    program.title = programElm.attribute("title");

    const long duration = programElm.attribute("duration", "-1").toLong();
    program.duration = duration >= 0 ? STime::fromMSec(duration) : STime();

    for (QDomElement elm = programElm.firstChildElement("chapter");
         !elm.isNull();
         elm = elm.nextSiblingElement("chapter"))
    {
      Chapter chapter;
      chapter.title = elm.attribute("title");
      chapter.begin = STime::fromUSec(elm.attribute("begin").toLongLong());
      chapter.end = STime::fromUSec(elm.attribute("end").toLongLong());

      program.chapters.append(chapter);
    }

    for (QDomElement elm = programElm.firstChildElement("audiostream");
         !elm.isNull();
         elm = elm.nextSiblingElement("audiostream"))
    {
      SAudioCodec codec;
      codec.fromXml(elm);

      program.audioStreams.append(
          AudioStreamInfo(
              elm.attribute("id").toUShort(),
              elm.attribute("language").toAscii(), codec));
    }

    for (QDomElement elm = programElm.firstChildElement("videostream");
         !elm.isNull();
         elm = elm.nextSiblingElement("videostream"))
    {
      SVideoCodec codec;
      codec.fromXml(elm);

      program.videoStreams.append(
          VideoStreamInfo(
              elm.attribute("id").toUShort(),
              elm.attribute("language").toAscii(), codec));
    }

    for (QDomElement elm = programElm.firstChildElement("datastream");
         !elm.isNull();
         elm = elm.nextSiblingElement("datastream"))
    {
      SDataCodec codec;
      codec.fromXml(elm);

      DataStreamInfo stream(
          DataStreamInfo::Type(elm.attribute("type").toInt()),
          elm.attribute("id").toUShort(),
          elm.attribute("language").toAscii(), codec);

      stream.file = elm.attribute("file");
      program.dataStreams.append(stream);
    }

    QDomElement imgElm = programElm.firstChildElement("imagecodec");
    if (!imgElm.isNull())
      program.imageCodec.fromXml(imgElm);

    QDomElement thumbElm = programElm.firstChildElement("thumbnail");
    if (!thumbElm.isNull())
      program.thumbnail = QByteArray::fromBase64(thumbElm.text().toAscii());

    pi.programs.append(program);
  }

  pi.title = mediaInfoElm.attribute("title");
  pi.author = mediaInfoElm.attribute("author");
  pi.copyright = mediaInfoElm.attribute("copyright");
  pi.comment = mediaInfoElm.attribute("comment");
  pi.album = mediaInfoElm.attribute("album");
  pi.genre = mediaInfoElm.attribute("genre");
  pi.year = mediaInfoElm.attribute("year").toUInt();
  pi.track = mediaInfoElm.attribute("track").toUInt();
}

void SMediaInfo::probe(const QString &filePath)
{
  struct Callback : SInterfaces::BufferReader::ReadCallback
  {
    Callback(const QString &path) : ReadCallback(path), file(path)
    {
      file.open(QFile::ReadOnly);
    }

    virtual qint64 read(uchar *buffer, qint64 size)
    {
      if (file.isOpen())
        return file.read((char *)buffer, size);
      else
        return -1;
    }

    virtual qint64 seek(qint64 offset, int whence)
    {
      if (file.isOpen())
      {
        if (whence == SEEK_SET)
          return file.seek(offset) ? 0 : -1;
        else if (whence == SEEK_CUR)
          return file.seek(file.pos() + offset) ? 0 : -1;
        else if (whence == SEEK_END)
          return file.seek(file.size() + offset) ? 0 : -1;
        else if (whence == -1) // get size
          return file.size();
      }

      return -1;
    }

    QFile file;
  };

  const QFileInfo fileInfo(filePath);
  if (fileInfo.exists())
  {
    pi->filePath = filePath;
    pi->size = fileInfo.size();
    pi->lastModified = fileInfo.lastModified();

    Callback callback(filePath);
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
    {
      callback.seek(0, SEEK_SET);
      prober->probeMetadata(*pi, &callback);
      delete prober;
    }

    probeDataStreams();

    pi->isProbed = true;
  }
}

// Subtitles in separate files need to be verified every time
void SMediaInfo::probeDataStreams(void)
{
  if (pi->programs.count() == 1)
  {
    Program * const program = &(pi->programs.first());
    if (!program->videoStreams.isEmpty())
    {
      // Remove subtitles.
      QList<DataStreamInfo> subs;
      for (QList<DataStreamInfo>::Iterator i=program->dataStreams.begin();
           i != program->dataStreams.end(); )
      if (!i->file.isEmpty())
      {
        subs += *i;
        i = program->dataStreams.erase(i);
      }
      else
        i++;

      // Add subtitles with new ID (To ensure IDs match SFileInputNode).
      quint16 nextStreamId = 0xF000;
      foreach (const QString &fileName, SSubtitleFile::findSubtitleFiles(pi->filePath))
      {
        bool found = false;
        for (QList<DataStreamInfo>::Iterator i=subs.begin(); i!=subs.end(); i++)
        if (i->file == fileName)
        {
          i->streamSpec = DataStreamInfo::Type_Subtitle | quint32(nextStreamId++);
          program->dataStreams += *i;

          found = true;
          break;
        }

        if (!found)
        {
          SSubtitleFile file(fileName);
          if (file.open())
          {
            DataStreamInfo stream(
                DataStreamInfo::Type_Subtitle,
                nextStreamId++,
                file.language(),
                file.codec());

            stream.file = fileName;
            program->dataStreams += stream;
          }
        }
      }
    }
  }
}


} // End of namespace
