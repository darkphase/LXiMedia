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

bool FileNode::isFormatProbed(void) const
{
  return probeInfo().isFormatProbed;
}

bool FileNode::isContentProbed(void) const
{
  return probeInfo().isContentProbed;
}

QByteArray FileNode::probeFormat(int indent)
{
  SMediaInfo::readFileInfo();
  SMediaInfo::probeFormat();

  return toByteArray(indent);
}

QByteArray FileNode::probeContent(int indent)
{
  SMediaInfo::readFileInfo();
  SMediaInfo::probeContent();

  return toByteArray(indent);
}

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

  mediaInfoElm.setAttribute("isFormatProbed", pi.isFormatProbed ? 1 : 0);
  mediaInfoElm.setAttribute("isContentProbed", pi.isContentProbed ? 1 : 0);

  mediaInfoElm.setAttribute("format", SStringParser::removeControl(pi.format));
  mediaInfoElm.setAttribute("type", QString::number(pi.fileType));
  mediaInfoElm.setAttribute("typename", SStringParser::removeControl(pi.fileTypeName));

  mediaInfoElm.setAttribute("fastHash", QString(pi.fastHash.toBase64()));

  if (pi.duration.isValid())
    mediaInfoElm.setAttribute("duration", QString::number(pi.duration.toMSec()));

  foreach (const Chapter &chapter, pi.chapters)
  {
    QDomElement elm = doc.createElement("chapter");
    elm.setAttribute("title", chapter.title);
    elm.setAttribute("begin", chapter.begin.toUSec());
    elm.setAttribute("end", chapter.end.toUSec());
    mediaInfoElm.appendChild(elm);
  }

  foreach (const AudioStreamInfo &audioStream, pi.audioStreams)
  {
    QDomElement elm = doc.createElement("audiostream");
    elm.setAttribute("id", audioStream.toString());
    elm.setAttribute("language", audioStream.language);
    elm.setAttribute("title", audioStream.title);
    elm.setAttribute("codec", audioStream.codec.toString());
    mediaInfoElm.appendChild(elm);
  }

  foreach (const VideoStreamInfo &videoStream, pi.videoStreams)
  {
    QDomElement elm = doc.createElement("videostream");
    elm.setAttribute("id", videoStream.toString());
    elm.setAttribute("language", videoStream.language);
    elm.setAttribute("title", videoStream.title);
    elm.setAttribute("codec", videoStream.codec.toString());
    mediaInfoElm.appendChild(elm);
  }

  foreach (const DataStreamInfo &dataStream, pi.dataStreams)
  {
    QDomElement elm = doc.createElement("datastream");
    elm.setAttribute("id", dataStream.toString());
    elm.setAttribute("language", dataStream.language);
    elm.setAttribute("title", dataStream.title);
    elm.setAttribute("file", dataStream.file);
    elm.setAttribute("codec", dataStream.codec.toString());
    mediaInfoElm.appendChild(elm);
  }

  if (!pi.imageCodec.isNull())
  {
    QDomElement elm = doc.createElement("imagecodec");
    elm.setAttribute("codec", pi.imageCodec.toString());
    mediaInfoElm.appendChild(elm);
  }

  QBuffer b;
  if (SImage(pi.thumbnail).save(&b, "JPEG", 50))
  {
    QDomElement thumbElm = doc.createElement("thumbnail");
    thumbElm.appendChild(doc.createTextNode(b.data().toBase64()));
    mediaInfoElm.appendChild(thumbElm);
  }

  for (QMap<QString, QVariant>::ConstIterator i = pi.metadata.begin();
       i != pi.metadata.end();
       i++)
  {
    QDomElement metadataElm = doc.createElement("metadata");
    metadataElm.setAttribute("key", i.key());
    metadataElm.setAttribute("value", i.value().toString());

    mediaInfoElm.appendChild(metadataElm);
  }

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

    pi->isReadable = false;
    pi->isFormatProbed = mediaInfoElm.attribute("isFormatProbed").toInt() != 0;
    pi->isContentProbed = mediaInfoElm.attribute("isContentProbed").toInt() != 0;

    pi->format = mediaInfoElm.attribute("format");
    pi->fileType = ProbeInfo::FileType(mediaInfoElm.attribute("type").toInt());
    pi->fileTypeName = mediaInfoElm.attribute("typename");

    pi->fastHash = QByteArray::fromBase64(mediaInfoElm.attribute("fastHash").toAscii());

    const qint64 duration = mediaInfoElm.attribute("duration", "-1").toLongLong();
    pi->duration = duration >= 0 ? STime::fromMSec(duration) : STime();

    for (QDomElement elm = mediaInfoElm.firstChildElement("chapter");
         !elm.isNull();
         elm = elm.nextSiblingElement("chapter"))
    {
      Chapter chapter;
      chapter.title = elm.attribute("title");
      chapter.begin = STime::fromUSec(elm.attribute("begin").toLongLong());
      chapter.end = STime::fromUSec(elm.attribute("end").toLongLong());

      pi->chapters.append(chapter);
    }

    for (QDomElement elm = mediaInfoElm.firstChildElement("audiostream");
         !elm.isNull();
         elm = elm.nextSiblingElement("audiostream"))
    {
      pi->audioStreams.append(
          AudioStreamInfo(
              StreamId::fromString(elm.attribute("id")),
              elm.attribute("language").toAscii(),
              elm.attribute("title"),
              SAudioCodec::fromString(elm.attribute("codec"))));
    }

    for (QDomElement elm = mediaInfoElm.firstChildElement("videostream");
         !elm.isNull();
         elm = elm.nextSiblingElement("videostream"))
    {
      pi->videoStreams.append(
          VideoStreamInfo(
              StreamId::fromString(elm.attribute("id")),
              elm.attribute("language").toAscii(),
              elm.attribute("title"),
              SVideoCodec::fromString(elm.attribute("codec"))));
    }

    for (QDomElement elm = mediaInfoElm.firstChildElement("datastream");
         !elm.isNull();
         elm = elm.nextSiblingElement("datastream"))
    {
      DataStreamInfo stream(
          StreamId::fromString(elm.attribute("id")),
          elm.attribute("language").toAscii(),
          elm.attribute("title"),
          SDataCodec::fromString(elm.attribute("codec")));

      stream.file = elm.attribute("file");
      pi->dataStreams.append(stream);
    }

    QDomElement imageCodecElm = mediaInfoElm.firstChildElement("imagecodec");
    if (!imageCodecElm.isNull())
      pi->imageCodec = SVideoCodec::fromString(imageCodecElm.attribute("codec"));

    QDomElement thumbElm = mediaInfoElm.firstChildElement("thumbnail");
    if (!thumbElm.isNull())
      pi->thumbnail = SImage::fromData(QByteArray::fromBase64(thumbElm.text().toAscii())).toVideoBuffer();

    for (QDomElement metadataElm = mediaInfoElm.firstChildElement("metadata");
         !metadataElm.isNull();
         metadataElm = metadataElm.nextSiblingElement("metadata"))
    {
      pi->metadata.insert(metadataElm.attribute("key"), metadataElm.attribute("value"));
    }

    return FileNode(QSharedDataPointer<ProbeInfo>(pi));
  }

  return FileNode();
}

} } // End of namespaces
