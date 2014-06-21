/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include "psbufferreader.h"
#include "psbufferwriter.h"

namespace LXiStream {
namespace Common {

/*
PsBufferReader::PsBufferReader(const QString &, QObject *parent)
  : SInterfaces::BufferReader(parent),
    callback(NULL),
    pesPacketStream(this, (MPEG::PESPacketStream::PESCallback)&PsBufferReader::processPESPacket, NULL),
    buffer(NULL),
    audioBuffer(),
    videoBuffer(SBuffer()),
    dataBuffer(SBuffer()),
    expectedSize(0),
    readOffset(0)
{
}

PsBufferReader::~PsBufferReader()
{
}

bool PsBufferReader::openFormat(const QString &)
{
  return true;
}

bool PsBufferReader::start(Callback *c, bool)
{
  callback = c;

  expectedSize = 0;

  return true;
}

void PsBufferReader::stop(void)
{
  callback = NULL;
  buffer = NULL;
  audioBuffer = SBuffer();
  videoBuffer = SBuffer();
  dataBuffer = SBuffer();
}

void PsBufferReader::process(void)
{
  uchar buffer[pesPacketStream.bufferSize / 4];

  const qint64 size = callback->read(buffer, sizeof(buffer));
  if (size > 0)
    pesPacketStream.processData(buffer, size);
}

STime PsBufferReader::duration(void) const
{
  return STime();
}

bool PsBufferReader::setPosition(STime)
{
  return false;
}

QList<PsBufferReader::AudioStreamInfo> PsBufferReader::audioStreams(void) const
{
  QList<AudioStreamInfo> streams;

  for (QMap<SBuffer::StreamId, SCodec>::ConstIterator i=codecs.begin(); i!=codecs.end(); i++)
  if ((i.key() >> 16) == (SAudioBuffer::baseTypeId & SBuffer::dataTypeIdMask))
    streams += AudioStreamInfo(i.key() & 0xFFFF, *i);

  return streams;
}

QList<PsBufferReader::VideoStreamInfo> PsBufferReader::videoStreams(void) const
{
  QList<VideoStreamInfo> streams;

  for (QMap<SBuffer::StreamId, SCodec>::ConstIterator i=codecs.begin(); i!=codecs.end(); i++)
  if ((i.key() >> 16) == (SVideoBuffer::baseTypeId & SBuffer::dataTypeIdMask))
    streams += VideoStreamInfo(i.key() & 0xFFFF, *i);

  return streams;
}

QList<PsBufferReader::DataStreamInfo> PsBufferReader::dataStreams(void) const
{
  QList<DataStreamInfo> streams;

  for (QMap<SBuffer::StreamId, SCodec>::ConstIterator i=codecs.begin(); i!=codecs.end(); i++)
  if ((i.key() >> 16) == (SDataBuffer::baseTypeId & SBuffer::dataTypeIdMask))
    streams += DataStreamInfo(i.key() & 0xFFFF, *i);

  return streams;
}

void PsBufferReader::selectStreams(const QVector<StreamId> &streamIds)
{
#warning todo
}

void PsBufferReader::processPESPacket(const MPEG::PESPacket *pesPacket, size_t size)
{
  if (pesPacket->getStreamID() == MPEG::PESHeader::StreamId_PackHeader)
  {
    buffer = NULL;
    audioBuffer = SBuffer();
    videoBuffer = SBuffer();
    dataBuffer = SBuffer();

    expectedSize = 0;
    readOffset = 0;
  }
  else if (pesPacket->isValid())
  {
    if ((buffer == NULL) && (pesPacket->getStreamID() == MPEG::PESHeader::StreamId_Padding))
    { // We're starting a new buffer
      const PsBufferWriter::BufferHeader * const header =
          reinterpret_cast<const PsBufferWriter::BufferHeader *>(pesPacket->getPayload());

      if ((header->syncWord == PsBufferWriter::BufferHeader::SyncWord) &&
          (pesPacket->calculatePayloadSize(size) >= sizeof(PsBufferWriter::BufferHeader)))
      {
        expectedSize = size_t(header->dataSize);

        if (header->typeId == SAudioBuffer::baseTypeId)
          buffer = &(audioBuffer = SAudioBuffer(expectedSize));
        else if (header->typeId == SVideoBuffer::baseTypeId)
          buffer = &(videoBuffer = SVideoBuffer(expectedSize));
        else if (header->typeId == SDataBuffer::baseTypeId)
          buffer = &(dataBuffer = SDataBuffer(expectedSize));

        if (buffer)
        {
          readOffset = 0;

          buffer->setSubStreamId(header->subStreamId);
          buffer->setNumBytes(header->dataSize);
          buffer->setTimeStamp(header->timeStamp);
          buffer->setDecodingTimeStamp(header->decodingTimeStamp);
          buffer->setPresentationTimeStamp(header->presentationTimeStamp);

          QMap<SBuffer::StreamId, SCodec>::ConstIterator codec = codecs.find(buffer->streamId());
          if (codec != codecs.end())
            buffer->setCodec(*codec);

          if (header->typeId == SVideoBuffer::baseTypeId)
          {
            const PsBufferWriter::VideoBufferHeader * const header =
                reinterpret_cast<const PsBufferWriter::VideoBufferHeader *>(pesPacket->getPayload());

            videoBuffer.setKeyFrame(header->keyFrame);
            videoBuffer.setNumChannels(header->numChannels);

            for (unsigned i=0; i<SVideoBuffer::maxChannels; i++)
            {
              videoBuffer.setLineSize(i, header->lineSize[i]);
              videoBuffer.setOffset(i, header->offset[i]);
            }
          }
        }
      }
      else if (header->syncWord == PsBufferWriter::CodecMapSyncWord)
      { // A new codec map is received.
        const QByteArray codecMap((const char *)pesPacket->getPayload() + sizeof(PsBufferWriter::CodecMapSyncWord),
                                  pesPacket->calculatePayloadSize(size) - sizeof(PsBufferWriter::CodecMapSyncWord));

        foreach (const QByteArray &c, codecMap.split('\n'))
        if (c.length() > 8)
        {
          const quint32 streamID = c.left(8).toUInt(NULL, 16);
          SCodec codec;
          codec.fromByteArray(c.mid(8));

          codecs[streamID] = codec;
        }
      }
    }
    else if (buffer)
    { // Append to existing buffer
      const quint8 * data = pesPacket->getPayload();
      size_t dataSize = pesPacket->calculatePayloadSize(size);

      if ((pesPacket->getStreamID() == MPEG::PESHeader::StreamId_Private1) &&
          ((buffer->codec() == "ac3") || (buffer->codec() == "dts")) &&
          (dataSize >= 4))
      {
        data += 4; // Skip AC3/DTS Audio Substream Header
        dataSize -= 4;
      }

      if (readOffset + dataSize <= buffer->capacity())
      {
        memcpy((quint8 *)(buffer->bits()) + readOffset, data, dataSize);
        readOffset += dataSize;

        // Check if the buffer is complete
        if (readOffset >= expectedSize)
        {
          if (!audioBuffer.isNull())
            callback->produce(audioBuffer);
          else if (!videoBuffer.isNull())
            callback->produce(videoBuffer);
          else if (!dataBuffer.isNull())
            callback->produce(dataBuffer);

          buffer = NULL;
          audioBuffer = SBuffer();
          videoBuffer = SBuffer();
          dataBuffer = SBuffer();
        }
      }
    }
  }
}
*/

} } // End of namespaces
