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

#include "filenode.h"
#include <QtXml>

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

QByteArray FileNode::toByteArray(int indent) const
{
  struct T
  {
    static const char * trueFalse(bool b)
    {
      return b ? "true" : "false";
    }
  };

  QDomDocument doc;
  QDomElement mediaInfoElm = doc.createElement("mediainfo");

  const ProbeInfo &pi = probeInfo();

  mediaInfoElm.setAttribute("filePath", pi.filePath);
  mediaInfoElm.setAttribute("size", pi.size);
  mediaInfoElm.setAttribute("lastModified", pi.lastModified.toString(Qt::ISODate));

  mediaInfoElm.setAttribute("format", SStringParser::removeControl(pi.format));

  if (pi.isProbed)         mediaInfoElm.setAttribute("probed", T::trueFalse(pi.isProbed));
  if (pi.isReadable)       mediaInfoElm.setAttribute("readable", T::trueFalse(pi.isReadable));

  mediaInfoElm.setAttribute("type", SStringParser::removeControl(pi.fileTypeName));

  foreach (const Program &program, pi.programs)
  {
    QDomElement programElm = doc.createElement("program");
    programElm.setAttribute("id", QString::number(program.programId));

    if (!program.title.isEmpty())
      programElm.setAttribute("title", SStringParser::removeControl(program.title));

    if (program.duration.isValid())
      programElm.setAttribute("duration", QString::number(program.duration.toMSec()));

    foreach (const Chapter &chapter, program.chapters)
    {
      QDomElement elm = doc.createElement("chapter");
      elm.setAttribute("title", chapter.title);
      elm.setAttribute("begin", chapter.begin.toUSec());
      elm.setAttribute("end", chapter.end.toUSec());
      programElm.appendChild(elm);
    }

    foreach (const AudioStreamInfo &audioStream, program.audioStreams)
    {
      QDomElement elm = doc.createElement("audiostream");
      elm.setAttribute("id", audioStream.toString());
      elm.setAttribute("language", audioStream.language);
      elm.setAttribute("title", audioStream.title);
      elm.setAttribute("codec", audioStream.codec.toString(false));
      programElm.appendChild(elm);
    }

    foreach (const VideoStreamInfo &videoStream, program.videoStreams)
    {
      QDomElement elm = doc.createElement("videostream");
      elm.setAttribute("id", videoStream.toString());
      elm.setAttribute("language", videoStream.language);
      elm.setAttribute("title", videoStream.title);
      elm.setAttribute("codec", videoStream.codec.toString(false));
      programElm.appendChild(elm);
    }

    foreach (const DataStreamInfo &dataStream, program.dataStreams)
    {
      QDomElement elm = doc.createElement("datastream");
      elm.setAttribute("id", dataStream.toString());
      elm.setAttribute("language", dataStream.language);
      elm.setAttribute("title", dataStream.title);
      elm.setAttribute("file", dataStream.file);
      elm.setAttribute("codec", dataStream.codec.toString(false));
      programElm.appendChild(elm);
    }

    if (!program.imageCodec.isNull())
    {
      QDomElement elm = doc.createElement("imagecodec");
      elm.setAttribute("codec", program.imageCodec.toString(false));
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

  doc.appendChild(mediaInfoElm);

  return doc.toByteArray(indent);
}

FileNode FileNode::fromByteArray(const QByteArray &str)
{
  struct T
  {
    static bool trueFalse(const QString &s)
    {
      return s == "true";
    }
  };

  QDomDocument doc;
  if (doc.setContent(str))
  {
    QDomElement mediaInfoElm = doc.documentElement();

    ProbeInfo * const pi = new ProbeInfo();
    pi->filePath = mediaInfoElm.attribute("filePath");
    pi->size = mediaInfoElm.attribute("size").toLongLong();
    pi->lastModified = QDateTime::fromString(mediaInfoElm.attribute("lastModified"), Qt::ISODate);

    pi->format = mediaInfoElm.attribute("format");

    pi->isProbed = T::trueFalse(mediaInfoElm.attribute("probed"));
    pi->isReadable = T::trueFalse(mediaInfoElm.attribute("readable"));

    pi->fileTypeName = mediaInfoElm.attribute("type");

    for (QDomElement programElm = mediaInfoElm.firstChildElement("program");
         !programElm.isNull();
         programElm = programElm.nextSiblingElement("program"))
    {
      Program program(programElm.attribute("id").toUShort());
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
        program.audioStreams.append(
            AudioStreamInfo(
                StreamId::fromString(elm.attribute("id")),
                elm.attribute("language").toAscii(),
                elm.attribute("title"),
                SAudioCodec::fromString(elm.attribute("codec"))));
      }

      for (QDomElement elm = programElm.firstChildElement("videostream");
           !elm.isNull();
           elm = elm.nextSiblingElement("videostream"))
      {
        program.videoStreams.append(
            VideoStreamInfo(
                StreamId::fromString(elm.attribute("id")),
                elm.attribute("language").toAscii(),
                elm.attribute("title"),
                SVideoCodec::fromString(elm.attribute("codec"))));
      }

      for (QDomElement elm = programElm.firstChildElement("datastream");
           !elm.isNull();
           elm = elm.nextSiblingElement("datastream"))
      {
        DataStreamInfo stream(
            StreamId::fromString(elm.attribute("id")),
            elm.attribute("language").toAscii(),
            elm.attribute("title"),
            SDataCodec::fromString(elm.attribute("codec")));

        stream.file = elm.attribute("file");
        program.dataStreams.append(stream);
      }

      QDomElement imageCodecElm = programElm.firstChildElement("imagecodec");
      if (!imageCodecElm.isNull())
        program.imageCodec = SVideoCodec::fromString(imageCodecElm.attribute("codec"));

      QDomElement thumbElm = programElm.firstChildElement("thumbnail");
      if (!thumbElm.isNull())
        program.thumbnail = QByteArray::fromBase64(thumbElm.text().toAscii());

      pi->programs.append(program);
    }

    pi->title = mediaInfoElm.attribute("title");
    pi->author = mediaInfoElm.attribute("author");
    pi->copyright = mediaInfoElm.attribute("copyright");
    pi->comment = mediaInfoElm.attribute("comment");
    pi->album = mediaInfoElm.attribute("album");
    pi->genre = mediaInfoElm.attribute("genre");
    pi->year = mediaInfoElm.attribute("year").toUInt();
    pi->track = mediaInfoElm.attribute("track").toUInt();

    return FileNode(QSharedDataPointer<ProbeInfo>(pi));
  }

  return FileNode();
}

} } // End of namespaces
