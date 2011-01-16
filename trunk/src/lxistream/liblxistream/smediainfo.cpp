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

struct SMediaInfo::Data
{
  inline Data() : path(), pi() { }
  inline Data(const QString &path) : path(path), pi() { }
  inline Data(const SInterfaces::FormatProber::ProbeInfo &pi) : path(), pi(pi) { }
  inline Data(const Data &from) : path(from.path), pi(from.pi) { }

  QString                       path;
  SInterfaces::FormatProber::ProbeInfo pi;
};

SMediaInfo::SMediaInfo(void)
  : d(new Data())
{
}

SMediaInfo::SMediaInfo(const SMediaInfo &from)
  : d(new Data(*(from.d)))
{
}

SMediaInfo::SMediaInfo(const QString &path)
  : d(new Data(path))
{
  probe();
}

SMediaInfo::SMediaInfo(const SInterfaces::FormatProber::ProbeInfo &pi)
  : d(new Data(pi))
{
}

SMediaInfo::~SMediaInfo()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

SMediaInfo & SMediaInfo::operator=(const SMediaInfo &from)
{
  delete d;
  *const_cast<Data **>(&d) = new Data(*(from.d));

  return *this;
}

QDomNode SMediaInfo::toXml(QDomDocument &doc) const
{
  return toXml(d->pi, doc);
}

void SMediaInfo::fromXml(const QDomNode &elm)
{
  fromXml(d->pi, elm);
  probeDataStreams();
}

void SMediaInfo::fromXml(const QDomNode &elm, const QString &path)
{
  d->path = path;
  fromXml(d->pi, elm);
  probeDataStreams();
}

void SMediaInfo::fromByteArray(const QByteArray &data, const QString &path)
{
  d->path = path;
  SSerializable::fromByteArray(data);
  probeDataStreams();
}

QString SMediaInfo::format(void) const
{
  return d->pi.format;
}

bool SMediaInfo::isDisc(void) const
{
  return d->pi.isDisc;
}

bool SMediaInfo::containsAudio(void) const
{
  foreach (const SInterfaces::FormatProber::ProbeInfo &p, d->pi.titles)
  if (!p.audioStreams.isEmpty())
    return true;

  return !d->pi.audioStreams.isEmpty();
}

bool SMediaInfo::containsVideo(void) const
{
  foreach (const SInterfaces::FormatProber::ProbeInfo &p, d->pi.titles)
  if (!p.videoStreams.isEmpty())
    return true;

  return !d->pi.videoStreams.isEmpty();
}

bool SMediaInfo::containsImage(void) const
{
  foreach (const SInterfaces::FormatProber::ProbeInfo &p, d->pi.titles)
  if (!p.imageCodec.isNull())
    return true;

  return !d->pi.imageCodec.isNull();
}

bool SMediaInfo::isProbed(void) const
{
  return d->pi.isProbed;
}

bool SMediaInfo::isReadable(void) const
{
  return d->pi.isReadable;
}

QString SMediaInfo::fileTypeName(void) const
{
  return d->pi.fileTypeName;
}

STime SMediaInfo::duration(void) const
{
  STime result = d->pi.duration.isValid() ? d->pi.duration : STime::null;
  foreach (const SInterfaces::FormatProber::ProbeInfo &p, d->pi.titles)
  if (p.duration.isValid())
    result += p.duration;

  return result;
}

QList<SMediaInfo::AudioStreamInfo> SMediaInfo::audioStreams(void) const
{
  return d->pi.audioStreams;
}

QList<SMediaInfo::VideoStreamInfo> SMediaInfo::videoStreams(void) const
{
  return d->pi.videoStreams;
}

QList<SMediaInfo::DataStreamInfo> SMediaInfo::dataStreams(void) const
{
  return d->pi.dataStreams;
}

SVideoCodec SMediaInfo::imageCodec(void) const
{
  return d->pi.imageCodec;
}

QList<SMediaInfo::Chapter> SMediaInfo::chapters(void) const
{
  return d->pi.chapters;
}

SMediaInfoList SMediaInfo::titles(void) const
{
  SMediaInfoList result;
  foreach (const SInterfaces::FormatProber::ProbeInfo &p, d->pi.titles)
    result += SMediaInfo(p);

  return result;
}

QString SMediaInfo::title(void) const
{
  return d->pi.title;
}

QString SMediaInfo::author(void) const
{
  return d->pi.author;
}

QString SMediaInfo::copyright(void) const
{
  return d->pi.copyright;
}

QString SMediaInfo::comment(void) const
{
  return d->pi.comment;
}

QString SMediaInfo::album(void) const
{
  return d->pi.album;
}

QString SMediaInfo::genre(void) const
{
  return d->pi.genre;
}

unsigned SMediaInfo::year(void) const
{
  return d->pi.year;
}

unsigned SMediaInfo::track(void) const
{
  return d->pi.track;
}

QList<QByteArray> SMediaInfo::thumbnails(void) const
{
  QList<QByteArray> result = d->pi.thumbnails;
  foreach (const SInterfaces::FormatProber::ProbeInfo &p, d->pi.titles)
    result += p.thumbnails;

  return result;
}

QDomNode SMediaInfo::toXml(const SInterfaces::FormatProber::ProbeInfo &pi, QDomDocument &doc)
{
  QDomElement mediainfo = createElement(doc, "mediainfo");

  mediainfo.setAttribute("format", SStringParser::removeControl(pi.format));
  if (pi.isDisc)           mediainfo.setAttribute("disc", trueFalse(pi.isDisc));

  if (pi.isProbed)         mediainfo.setAttribute("probed", trueFalse(pi.isProbed));
  if (pi.isReadable)       mediainfo.setAttribute("readable", trueFalse(pi.isReadable));

  mediainfo.setAttribute("type", SStringParser::removeControl(pi.fileTypeName));

  if (pi.duration.isValid()) mediainfo.setAttribute("duration", QString::number(pi.duration.toMSec()));

  foreach (const AudioStreamInfo &audioStream, pi.audioStreams)
  {
    QDomElement elm = createElement(doc, "audiostream");
    elm.setAttribute("id", audioStream.streamId);
    elm.setAttribute("language", audioStream.language);
    elm.appendChild(audioStream.codec.toXml(doc));
    mediainfo.appendChild(elm);
  }

  foreach (const VideoStreamInfo &videoStream, pi.videoStreams)
  {
    QDomElement elm = createElement(doc, "videostream");
    elm.setAttribute("id", videoStream.streamId);
    elm.setAttribute("language", videoStream.language);
    elm.appendChild(videoStream.codec.toXml(doc));
    mediainfo.appendChild(elm);
  }

  foreach (const DataStreamInfo &dataStream, pi.dataStreams)
  {
    QDomElement elm = createElement(doc, "datastream");
    elm.setAttribute("id", dataStream.streamId);
    elm.setAttribute("language", dataStream.language);
    elm.setAttribute("file", dataStream.file);
    elm.appendChild(dataStream.codec.toXml(doc));
    mediainfo.appendChild(elm);
  }

  if (!pi.imageCodec.isNull())
  {
    QDomElement elm = createElement(doc, "imagecodec");
    elm.appendChild(pi.imageCodec.toXml(doc));
    mediainfo.appendChild(elm);
  }

  foreach (const Chapter &chapter, pi.chapters)
  {
    QDomElement elm = createElement(doc, "chapter");
    elm.setAttribute("title", chapter.title);
    elm.setAttribute("begin", chapter.begin.toUSec());
    elm.setAttribute("end", chapter.end.toUSec());
    mediainfo.appendChild(elm);
  }

  unsigned titleId = 0;
  foreach (const SInterfaces::FormatProber::ProbeInfo &title, pi.titles)
  {
    QDomElement elm = createElement(doc, "title");
    elm.setAttribute("id", titleId++);
    elm.appendChild(toXml(title, doc));
    mediainfo.appendChild(elm);
  }

  if (!pi.title.isEmpty())     mediainfo.setAttribute("title", SStringParser::removeControl(pi.title));
  if (!pi.author.isEmpty())    mediainfo.setAttribute("author", SStringParser::removeControl(pi.author));
  if (!pi.copyright.isEmpty()) mediainfo.setAttribute("copyright", SStringParser::removeControl(pi.copyright));
  if (!pi.comment.isEmpty())   mediainfo.setAttribute("comment", SStringParser::removeControl(pi.comment));
  if (!pi.album.isEmpty())     mediainfo.setAttribute("album", SStringParser::removeControl(pi.album));
  if (!pi.genre.isEmpty())     mediainfo.setAttribute("genre", SStringParser::removeControl(pi.genre));
  if (pi.year != 0)            mediainfo.setAttribute("year", pi.year);
  if (pi.track != 0)           mediainfo.setAttribute("track", pi.track);

  foreach (const QByteArray &thumbnail, pi.thumbnails)
  {
    QDomElement elm = doc.createElement("thumbnail");
    elm.appendChild(doc.createTextNode(thumbnail.toBase64()));
    mediainfo.appendChild(elm);
  }

  return mediainfo;
}

void SMediaInfo::fromXml(SInterfaces::FormatProber::ProbeInfo &pi, const QDomNode &elm)
{
  QDomElement mediainfo = findElement(elm, "mediainfo");

  pi.format = mediainfo.attribute("format");
  pi.isDisc = trueFalse(mediainfo.attribute("disc"));

  pi.isProbed = trueFalse(mediainfo.attribute("probed"));
  pi.isReadable = trueFalse(mediainfo.attribute("readable"));

  pi.fileTypeName = mediainfo.attribute("type");

  const long duration = mediainfo.attribute("duration", "-1").toLong();
  pi.duration = duration >= 0 ? STime::fromMSec(duration) : STime();

  for (QDomElement elm = mediainfo.firstChildElement("audiostream");
       !elm.isNull();
       elm = elm.nextSiblingElement("audiostream"))
  {
    SAudioCodec codec;
    codec.fromXml(elm);

    pi.audioStreams += AudioStreamInfo(elm.attribute("id").toUShort(), elm.attribute("language").toAscii(), codec);
  }

  for (QDomElement elm = mediainfo.firstChildElement("videostream");
       !elm.isNull();
       elm = elm.nextSiblingElement("videostream"))
  {
    SVideoCodec codec;
    codec.fromXml(elm);

    pi.videoStreams += VideoStreamInfo(elm.attribute("id").toUShort(), elm.attribute("language").toAscii(), codec);
  }

  for (QDomElement elm = mediainfo.firstChildElement("datastream");
       !elm.isNull();
       elm = elm.nextSiblingElement("datastream"))
  {
    SDataCodec codec;
    codec.fromXml(elm);

    DataStreamInfo stream(elm.attribute("id").toUShort(), elm.attribute("language").toAscii(), codec);
    stream.file = elm.attribute("file");
    pi.dataStreams += stream;
  }

  QDomElement imgElm = mediainfo.firstChildElement("imagecodec");
  if (!imgElm.isNull())
    pi.imageCodec.fromXml(imgElm);

  QMultiMap<STime, Chapter> chapters;
  for (QDomElement elm = mediainfo.firstChildElement("chapter");
       !elm.isNull();
       elm = elm.nextSiblingElement("chapter"))
  {
    Chapter chapter;
    chapter.title = elm.attribute("title");
    chapter.begin = STime::fromUSec(elm.attribute("begin").toLongLong());
    chapter.end = STime::fromUSec(elm.attribute("end").toLongLong());

    chapters.insert(chapter.begin, chapter);
  }

  pi.chapters = chapters.values();

  QMultiMap<unsigned, SInterfaces::FormatProber::ProbeInfo> titles;
  for (QDomElement elm = mediainfo.firstChildElement("title");
       !elm.isNull();
       elm = elm.nextSiblingElement("title"))
  {
    SInterfaces::FormatProber::ProbeInfo title;
    fromXml(title, elm);

    titles.insert(elm.attribute("id").toUInt(), title);
  }

  pi.titles = titles.values();

  pi.title = mediainfo.attribute("title");
  pi.author = mediainfo.attribute("author");
  pi.copyright = mediainfo.attribute("copyright");
  pi.comment = mediainfo.attribute("comment");
  pi.album = mediainfo.attribute("album");
  pi.genre = mediainfo.attribute("genre");
  pi.year = mediainfo.attribute("year").toUInt();
  pi.track = mediainfo.attribute("track").toUInt();

  for (QDomElement elm = mediainfo.firstChildElement("thumbnail");
       !elm.isNull();
       elm = elm.nextSiblingElement("thumbnail"))
  {
    pi.thumbnails += QByteArray::fromBase64(elm.text().toAscii());
  }
}

void SMediaInfo::probe(void)
{
  struct ReadCallback : SInterfaces::BufferReader::ReadCallback
  {
    ReadCallback(QIODevice *file) : file(file)
    {
    }

    virtual qint64 read(uchar *buffer, qint64 size)
    {
      return file->read((char *)buffer, size);
    }

    virtual qint64 seek(qint64 offset, int whence)
    {
      if (whence == SEEK_SET)
        return file->seek(offset) ? 0 : -1;
      else if (whence == SEEK_CUR)
        return file->seek(file->pos() + offset) ? 0 : -1;
      else if (whence == SEEK_END)
        return file->seek(file->size() + offset) ? 0 : -1;
      else if (whence == -1) // get size
        return file->size();
      else
        return -1;
    }

    QIODevice * const file;
  };

  const QList<SInterfaces::FormatProber *> probers = SInterfaces::FormatProber::create(NULL);

  foreach (SInterfaces::FormatProber *prober, probers)
    prober->probeDisc(d->pi, d->path);

  if (!d->pi.isDisc)
  {
    const QString fileName = QFileInfo(d->path).fileName();
    QFile file(d->path);
    if (file.open(QFile::ReadOnly))
    foreach (SInterfaces::FormatProber *prober, probers)
    if (file.seek(0))
    {
      ReadCallback readCallback(&file);
      prober->probeFile(d->pi, &readCallback, fileName);
    }

    probeDataStreams();
  }

  d->pi.isProbed = true;

  foreach (SInterfaces::FormatProber *prober, probers)
    delete prober;
}

// Subtitles in separate files need to be verified every time
void SMediaInfo::probeDataStreams(void)
{
  if (!d->pi.isDisc)
  {
    // Remove subtitles.
    QList<SMediaInfo::DataStreamInfo> subs;
    for (QList<SMediaInfo::DataStreamInfo>::Iterator i=d->pi.dataStreams.begin(); i!=d->pi.dataStreams.end(); )
    if (!i->file.isEmpty())
    {
      subs += *i;
      i = d->pi.dataStreams.erase(i);
    }
    else
      i++;

    // Add subtitles with new ID (To ensure IDs match SFileInputNode).
    quint16 nextStreamId = 0xF000;
    foreach (const QString &fileName, SSubtitleFile::findSubtitleFiles(d->path))
    {
      bool found = false;
      for (QList<SMediaInfo::DataStreamInfo>::Iterator i=subs.begin(); i!=subs.end(); i++)
      if (i->file == fileName)
      {
        i->streamId = nextStreamId++;
        d->pi.dataStreams += *i;

        found = true;
        break;
      }

      if (!found)
      {
        SSubtitleFile file(fileName);
        if (file.open())
        {
          DataStreamInfo stream(nextStreamId++, file.language(), file.codec());
          stream.file = fileName;
          d->pi.dataStreams += stream;
        }
      }
    }
  }
}


} // End of namespace
