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

#include "rawsubtitledecoder.h"

namespace LXiStream {
namespace Common {

RawSubtitleDecoder::RawSubtitleDecoder(const QString &, QObject *parent)
  : SInterfaces::DataDecoder(parent),
    decode(NULL)
{
}

RawSubtitleDecoder::~RawSubtitleDecoder()
{
}

bool RawSubtitleDecoder::openCodec(const SDataCodec &c, SInterfaces::AbstractBufferReader *, Flags)
{
  if (c.codec() == "SUB/RAWUTF8")
    decode = &RawSubtitleDecoder::decodeUtf8;
  else if (c.codec() == "SUB/RAWLATIN1")
    decode = &RawSubtitleDecoder::decodeLatin1;

  return decode != NULL;
}

SDataBufferList RawSubtitleDecoder::decodeBuffer(const SEncodedDataBuffer &dataBuffer)
{
  if (!dataBuffer.isNull())
  {
    const SSubtitleBuffer buffer = decode(dataBuffer);
    if (!buffer.isNull())
      return SDataBufferList() << buffer;
  }

  return SDataBufferList();
}

SSubtitleBuffer RawSubtitleDecoder::decodeUtf8(const SEncodedDataBuffer &dataBuffer)
{
  const QList<QByteArray> lines = QByteArray(dataBuffer.data(), dataBuffer.size()).split('\n');

  QStringList text;
  for (int i=0; i<lines.count(); i++)
    text += QString::fromUtf8(lines[i].trimmed());

  if (!text.isEmpty())
  {
    SSubtitleBuffer buffer(text);
    buffer.setDuration(dataBuffer.duration());

    if (dataBuffer.presentationTimeStamp().isValid())
      buffer.setTimeStamp(dataBuffer.presentationTimeStamp());
    else if (dataBuffer.decodingTimeStamp().isValid())
      buffer.setTimeStamp(dataBuffer.decodingTimeStamp());

    return buffer;
  }

  return SSubtitleBuffer();
}

SSubtitleBuffer RawSubtitleDecoder::decodeLatin1(const SEncodedDataBuffer &dataBuffer)
{
  const QList<QByteArray> lines = QByteArray(dataBuffer.data(), dataBuffer.size()).split('\n');

  QStringList text;
  for (int i=0; i<lines.count(); i++)
    text += QString::fromLatin1(lines[i].trimmed());

  if (!text.isEmpty())
  {
    SSubtitleBuffer buffer(text);
    buffer.setDuration(dataBuffer.duration());

    if (dataBuffer.presentationTimeStamp().isValid())
      buffer.setTimeStamp(dataBuffer.presentationTimeStamp());
    else if (dataBuffer.decodingTimeStamp().isValid())
      buffer.setTimeStamp(dataBuffer.decodingTimeStamp());

    return buffer;
  }

  return SSubtitleBuffer();
}

} } // End of namespaces
