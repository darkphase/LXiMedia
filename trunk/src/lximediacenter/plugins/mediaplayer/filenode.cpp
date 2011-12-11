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

  mediaInfoElm.setAttribute("isFileInfoRead", T::trueFalse(pi.isFileInfoRead));
  mediaInfoElm.setAttribute("isFormatProbed", T::trueFalse(pi.isFormatProbed));
  mediaInfoElm.setAttribute("isContentProbed", T::trueFalse(pi.isContentProbed));

  if (pi.isFileInfoRead)
  {
    QDomElement fileInfoElm = doc.createElement("fileInfo");

    fileInfoElm.setAttribute("isDir", T::trueFalse(pi.fileInfo.isDir));
    fileInfoElm.setAttribute("size", pi.fileInfo.size);
    fileInfoElm.setAttribute("lastModified", pi.fileInfo.lastModified.toString(Qt::ISODate));

    mediaInfoElm.appendChild(fileInfoElm);
  }

  if (pi.isFormatProbed)
  {
    QDomElement formatElm = doc.createElement("format");

    formatElm.setAttribute("format", SStringParser::removeControl(pi.format.format));
    formatElm.setAttribute("fileType", QString::number(pi.format.fileType));
    formatElm.setAttribute("fileTypeName", SStringParser::removeControl(pi.format.fileTypeName));
    formatElm.setAttribute("quickHash", QString(pi.format.quickHash.toBase64()));

    for (QMap<QString, QVariant>::ConstIterator i = pi.format.metadata.begin();
         i != pi.format.metadata.end();
         i++)
    {
      QDomElement metadataElm = doc.createElement("metadata");
      metadataElm.setAttribute("key", i.key());
      metadataElm.setAttribute("value", SStringParser::removeControl(i.value().toString()));

      formatElm.appendChild(metadataElm);
    }

    mediaInfoElm.appendChild(formatElm);
  }

  if (pi.isContentProbed)
  {
    QDomElement contentElm = doc.createElement("content");

    foreach (const ProbeInfo::Title &title, pi.content.titles)
    {
      QDomElement titleElm = doc.createElement("title");

      if (title.duration.isValid())
        titleElm.setAttribute("duration", QString::number(title.duration.toMSec()));

      foreach (const Chapter &chapter, title.chapters)
      {
        QDomElement elm = doc.createElement("chapter");
        elm.setAttribute("title", chapter.title);
        elm.setAttribute("begin", chapter.begin.toUSec());
        elm.setAttribute("end", chapter.end.toUSec());
        titleElm.appendChild(elm);
      }

      foreach (const AudioStreamInfo &audioStream, title.audioStreams)
      {
        QDomElement elm = doc.createElement("audiostream");
        elm.setAttribute("id", audioStream.toString());
        elm.setAttribute("language", audioStream.language);
        elm.setAttribute("title", audioStream.title);
        elm.setAttribute("codec", audioStream.codec.toString());
        titleElm.appendChild(elm);
      }

      foreach (const VideoStreamInfo &videoStream, title.videoStreams)
      {
        QDomElement elm = doc.createElement("videostream");
        elm.setAttribute("id", videoStream.toString());
        elm.setAttribute("language", videoStream.language);
        elm.setAttribute("title", videoStream.title);
        elm.setAttribute("codec", videoStream.codec.toString());
        titleElm.appendChild(elm);
      }

      foreach (const DataStreamInfo &dataStream, title.dataStreams)
      {
        QDomElement elm = doc.createElement("datastream");
        elm.setAttribute("id", dataStream.toString());
        elm.setAttribute("language", dataStream.language);
        elm.setAttribute("title", dataStream.title);
        elm.setAttribute("file", dataStream.file);
        elm.setAttribute("codec", dataStream.codec.toString());
        titleElm.appendChild(elm);
      }

      if (!title.imageCodec.isNull())
      {
        QDomElement elm = doc.createElement("imagecodec");
        elm.setAttribute("codec", title.imageCodec.toString());
        titleElm.appendChild(elm);
      }

      QBuffer b;
      if (SImage(title.thumbnail).save(&b, "JPEG", 50))
      {
        QDomElement thumbElm = doc.createElement("thumbnail");
        thumbElm.appendChild(doc.createTextNode(b.data().toBase64()));
        titleElm.appendChild(thumbElm);
      }

      contentElm.appendChild(titleElm);
    }

    mediaInfoElm.appendChild(contentElm);
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

    pi->isReadable = false;
    pi->isFileInfoRead = T::trueFalse(mediaInfoElm.attribute("isFileInfoRead"));
    pi->isFormatProbed = T::trueFalse(mediaInfoElm.attribute("isFormatProbed"));
    pi->isContentProbed = T::trueFalse(mediaInfoElm.attribute("isContentProbed"));

    QDomElement fileInfoElm = mediaInfoElm.firstChildElement("fileInfo");
    if (!fileInfoElm.isNull())
    {
      pi->fileInfo.isDir = T::trueFalse(fileInfoElm.attribute("isDir"));
      pi->fileInfo.size = fileInfoElm.attribute("size").toLongLong();
      pi->fileInfo.lastModified = QDateTime::fromString(fileInfoElm.attribute("lastModified"), Qt::ISODate);
    }

    QDomElement formatElm = mediaInfoElm.firstChildElement("format");
    if (!formatElm.isNull())
    {
      pi->format.format = formatElm.attribute("format");
      pi->format.fileType = ProbeInfo::FileType(formatElm.attribute("fileType").toInt());
      pi->format.fileTypeName = formatElm.attribute("fileTypeName");
      pi->format.quickHash = QByteArray::fromBase64(formatElm.attribute("quickHash").toAscii());

      for (QDomElement metadataElm = formatElm.firstChildElement("metadata");
           !metadataElm.isNull();
           metadataElm = metadataElm.nextSiblingElement("metadata"))
      {
        pi->format.metadata.insert(metadataElm.attribute("key"), metadataElm.attribute("value"));
      }
    }

    QDomElement contentElm = mediaInfoElm.firstChildElement("content");
    if (!contentElm.isNull())
    {
      for (QDomElement titleElm = contentElm.firstChildElement("title");
           !titleElm.isNull();
           titleElm = titleElm.nextSiblingElement("title"))
      {
        ProbeInfo::Title title;

        const qint64 duration = titleElm.attribute("duration", "-1").toLongLong();
        title.duration = duration >= 0 ? STime::fromMSec(duration) : STime();

        for (QDomElement elm = titleElm.firstChildElement("chapter");
             !elm.isNull();
             elm = elm.nextSiblingElement("chapter"))
        {
          Chapter chapter;
          chapter.title = elm.attribute("title");
          chapter.begin = STime::fromUSec(elm.attribute("begin").toLongLong());
          chapter.end = STime::fromUSec(elm.attribute("end").toLongLong());

          title.chapters.append(chapter);
        }

        for (QDomElement elm = titleElm.firstChildElement("audiostream");
             !elm.isNull();
             elm = elm.nextSiblingElement("audiostream"))
        {
          title.audioStreams.append(
              AudioStreamInfo(
                  StreamId::fromString(elm.attribute("id")),
                  elm.attribute("language").toAscii(),
                  elm.attribute("title"),
                  SAudioCodec::fromString(elm.attribute("codec"))));
        }

        for (QDomElement elm = titleElm.firstChildElement("videostream");
             !elm.isNull();
             elm = elm.nextSiblingElement("videostream"))
        {
          title.videoStreams.append(
              VideoStreamInfo(
                  StreamId::fromString(elm.attribute("id")),
                  elm.attribute("language").toAscii(),
                  elm.attribute("title"),
                  SVideoCodec::fromString(elm.attribute("codec"))));
        }

        for (QDomElement elm = titleElm.firstChildElement("datastream");
             !elm.isNull();
             elm = elm.nextSiblingElement("datastream"))
        {
          DataStreamInfo stream(
              StreamId::fromString(elm.attribute("id")),
              elm.attribute("language").toAscii(),
              elm.attribute("title"),
              SDataCodec::fromString(elm.attribute("codec")));

          stream.file = elm.attribute("file");
          title.dataStreams.append(stream);
        }

        QDomElement imageCodecElm = titleElm.firstChildElement("imagecodec");
        if (!imageCodecElm.isNull())
          title.imageCodec = SVideoCodec::fromString(imageCodecElm.attribute("codec"));

        QDomElement thumbElm = titleElm.firstChildElement("thumbnail");
        if (!thumbElm.isNull())
          title.thumbnail = SImage::fromData(QByteArray::fromBase64(thumbElm.text().toAscii())).toVideoBuffer();

        pi->content.titles += title;
      }
    }

    return FileNode(QSharedDataPointer<ProbeInfo>(pi));
  }

  return FileNode();
}

} } // End of namespaces
