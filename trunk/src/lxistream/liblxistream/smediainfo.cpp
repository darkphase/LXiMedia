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

QDomNode SMediaInfo::toXml(QDomDocument &doc) const
{
  probe();

  QDomElement mediainfo = createElement(doc, "mediainfo");
  if (d.pi.isProbed)         mediainfo.setAttribute("probed", trueFalse(d.pi.isProbed));
  if (d.pi.isReadable)       mediainfo.setAttribute("readable", trueFalse(d.pi.isReadable));

  mediainfo.setAttribute("type", SStringParser::removeControl(d.pi.fileTypeName));

  if (d.pi.duration.isValid()) mediainfo.setAttribute("duration", QString::number(d.pi.duration.toMSec()));

  foreach (const AudioStreamInfo &audioStream, d.pi.audioStreams)
  {
    QDomElement elm = createElement(doc, "audiostream");
    elm.setAttribute("id", audioStream.streamId);
    elm.setAttribute("language", audioStream.language);
    elm.appendChild(audioStream.codec.toXml(doc));
    mediainfo.appendChild(elm);
  }

  foreach (const VideoStreamInfo &videoStream, d.pi.videoStreams)
  {
    QDomElement elm = createElement(doc, "videostream");
    elm.setAttribute("id", videoStream.streamId);
    elm.setAttribute("language", videoStream.language);
    elm.appendChild(videoStream.codec.toXml(doc));
    mediainfo.appendChild(elm);
  }

  foreach (const DataStreamInfo &dataStream, d.pi.dataStreams)
  {
    QDomElement elm = createElement(doc, "datastream");
    elm.setAttribute("id", dataStream.streamId);
    elm.setAttribute("language", dataStream.language);
    elm.setAttribute("file", dataStream.file);
    elm.appendChild(dataStream.codec.toXml(doc));
    mediainfo.appendChild(elm);
  }

  if (!d.pi.imageCodec.isNull())
  {
    QDomElement elm = createElement(doc, "imagecodec");
    elm.appendChild(d.pi.imageCodec.toXml(doc));
    mediainfo.appendChild(elm);
  }

  foreach (const Chapter &chapter, d.pi.chapters)
  {
    QDomElement elm = createElement(doc, "chapter");
    elm.setAttribute("title", chapter.title);
    elm.setAttribute("begin", chapter.begin.toUSec());
    elm.setAttribute("end", chapter.end.toUSec());
    mediainfo.appendChild(elm);
  }

  if (!d.pi.title.isEmpty())     mediainfo.setAttribute("title", SStringParser::removeControl(d.pi.title));
  if (!d.pi.author.isEmpty())    mediainfo.setAttribute("author", SStringParser::removeControl(d.pi.author));
  if (!d.pi.copyright.isEmpty()) mediainfo.setAttribute("copyright", SStringParser::removeControl(d.pi.copyright));
  if (!d.pi.comment.isEmpty())   mediainfo.setAttribute("comment", SStringParser::removeControl(d.pi.comment));
  if (!d.pi.album.isEmpty())     mediainfo.setAttribute("album", SStringParser::removeControl(d.pi.album));
  if (!d.pi.genre.isEmpty())     mediainfo.setAttribute("genre", SStringParser::removeControl(d.pi.genre));
  if (d.pi.year != 0)            mediainfo.setAttribute("year", d.pi.year);
  if (d.pi.track != 0)           mediainfo.setAttribute("track", d.pi.track);

  foreach (const QByteArray &thumbnail, d.pi.thumbnails)
  {
    QDomElement elm = doc.createElement("thumbnail");
    elm.appendChild(doc.createTextNode(thumbnail.toBase64()));
    mediainfo.appendChild(elm);
  }

  return mediainfo;
}

void SMediaInfo::fromXml(const QDomNode &elm)
{
  QDomElement mediainfo = findElement(elm, "mediainfo");

  d.pi.isProbed = trueFalse(mediainfo.attribute("probed"));
  d.pi.isReadable = trueFalse(mediainfo.attribute("readable"));
  d.pi.fileTypeName = mediainfo.attribute("type");

  const long duration = mediainfo.attribute("duration", "-1").toLong();
  d.pi.duration = duration >= 0 ? STime::fromMSec(duration) : STime();

  for (QDomElement elm = mediainfo.firstChildElement("audiostream");
       !elm.isNull();
       elm = elm.nextSiblingElement("audiostream"))
  {
    SAudioCodec codec;
    codec.fromXml(elm);

    d.pi.audioStreams += AudioStreamInfo(elm.attribute("id").toUShort(), elm.attribute("language").toAscii(), codec);
  }

  for (QDomElement elm = mediainfo.firstChildElement("videostream");
       !elm.isNull();
       elm = elm.nextSiblingElement("videostream"))
  {
    SVideoCodec codec;
    codec.fromXml(elm);

    d.pi.videoStreams += VideoStreamInfo(elm.attribute("id").toUShort(), elm.attribute("language").toAscii(), codec);
  }

  for (QDomElement elm = mediainfo.firstChildElement("datastream");
       !elm.isNull();
       elm = elm.nextSiblingElement("datastream"))
  {
    SDataCodec codec;
    codec.fromXml(elm);

    DataStreamInfo stream(elm.attribute("id").toUShort(), elm.attribute("language").toAscii(), codec);
    stream.file = elm.attribute("file");
    d.pi.dataStreams += stream;
  }

  QDomElement imgElm = mediainfo.firstChildElement("imagecodec");
  if (!imgElm.isNull())
    d.pi.imageCodec.fromXml(imgElm);

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

  d.pi.chapters = chapters.values();

  d.pi.title = mediainfo.attribute("title");
  d.pi.author = mediainfo.attribute("author");
  d.pi.copyright = mediainfo.attribute("copyright");
  d.pi.comment = mediainfo.attribute("comment");
  d.pi.album = mediainfo.attribute("album");
  d.pi.genre = mediainfo.attribute("genre");
  d.pi.year = mediainfo.attribute("year").toUInt();
  d.pi.track = mediainfo.attribute("track").toUInt();

  for (QDomElement elm = mediainfo.firstChildElement("thumbnail");
       !elm.isNull();
       elm = elm.nextSiblingElement("thumbnail"))
  {
    d.pi.thumbnails += QByteArray::fromBase64(elm.text().toAscii());
  }
}

void SMediaInfo::probe(void) const
{
  if (!d.pi.isProbed)
  {
    QFile file(d.file);
    if (d.deepProbe)
      file.open(QFile::ReadOnly);

    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
    {
      prober->probeName(d.pi, d.file);
      if (file.isOpen() && file.seek(0))
        prober->probeFile(d.pi, &file);

      delete prober;
    }

    d.pi.isProbed = true;
    probeDataStreams();
  }
}

// Subtitles in separate files need to be verified every time
void SMediaInfo::probeDataStreams(void) const
{
  probe();

  // Remove subtitles.
  QList<SMediaInfo::DataStreamInfo> subs;
  for (QList<SMediaInfo::DataStreamInfo>::Iterator i=d.pi.dataStreams.begin(); i!=d.pi.dataStreams.end(); )
  if (!i->file.isEmpty())
  {
    subs += *i;
    i = d.pi.dataStreams.erase(i);
  }
  else
    i++;

  // Add subtitles with new ID (To ensure IDs match SFileInputNode).
  quint16 nextStreamId = 0xF000;
  foreach (const QString &fileName, SSubtitleFile::findSubtitleFiles(d.file))
  {
    bool found = false;
    for (QList<SMediaInfo::DataStreamInfo>::Iterator i=subs.begin(); i!=subs.end(); i++)
    if (i->file == fileName)
    {
      i->streamId = nextStreamId++;
      d.pi.dataStreams += *i;

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
        d.pi.dataStreams += stream;
      }
    }
  }
}


} // End of namespace
