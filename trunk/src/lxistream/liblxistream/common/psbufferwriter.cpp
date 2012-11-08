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

#include "psbufferwriter.h"

namespace LXiStream {
namespace Common {

/*
const char  * const PsBufferWriter::formatName = "lximedia-mpeg";
const quint16       PsBufferWriter::CodecMapSyncWord = 0x8F22;
const quint16       PsBufferWriter::BufferHeader::SyncWord = 0x8F23;


PsBufferWriter::PsBufferWriter(const QString &, QObject *parent)
  : SInterfaces::BufferWriter(parent),
    callback(NULL),
    lastTypeId(0),
    lastPackHeaderTimeStamp(STime::null),
    lastPSMapTimeStamp(STime::null),
    lastAudioTimeStamp(STime::null),
    sid(0),
    ccounter(0)
{
}

PsBufferWriter::~PsBufferWriter()
{
}

QByteArray PsBufferWriter::buildHeader(void)
{
  QByteArray header;

  MPEG::PSPackHeader psPackHeader;
  header.append(reinterpret_cast<const char *>(psPackHeader.data), MPEG::PSPackHeader::baseLength);

  MPEG::PSSystemHeader psSystemHeader;
  header.append(reinterpret_cast<const char *>(psSystemHeader.data), psSystemHeader.getTotalLength());

  MPEG::PESPacket pesLXiMediaHeader;
  pesLXiMediaHeader.initialize(MPEG::PESHeader::StreamId_Padding);
  qstrcpy(reinterpret_cast<char *>(pesLXiMediaHeader.getPayload()), formatName);
  pesLXiMediaHeader.setPayloadSize(qstrlen(reinterpret_cast<const char *>(pesLXiMediaHeader.getPayload())));
  header.append(reinterpret_cast<const char *>(&pesLXiMediaHeader), pesLXiMediaHeader.getTotalLength());

  return header;
}

bool PsBufferWriter::openFormat(const QString &)
{
  return true;
}

bool PsBufferWriter::start(Callback *c)
{
  callback = c;

  const QByteArray header = buildHeader();
  callback->write(reinterpret_cast<const uchar *>(header.data()), header.size());

  psMap = MPEG::PSMap();
  codecsPacket.initialize(MPEG::PESHeader::StreamId_Padding);
  memcpy(codecsPacket.getPayload(), &CodecMapSyncWord, sizeof(CodecMapSyncWord));
  codecsPacket.setPayloadSize(sizeof(CodecMapSyncWord));
  codecsLen = 0;

  lastTypeId = 0;
  lastPackHeaderTimeStamp = STime::null;
  lastPSMapTimeStamp = STime::null;
  lastAudioTimeStamp = STime::null;
  sid = 0;
  ccounter = 0;

  return true;
}

void PsBufferWriter::stop(void)
{
  callback->write(MPEG::programEndCode, sizeof(MPEG::programEndCode));

  callback = NULL;
}

void PsBufferWriter::process(const SAudioBuffer &buffer)
{
  writeBuffer(buffer);
}

void PsBufferWriter::process(const SVideoBuffer &buffer)
{
  writeBuffer(buffer);
}

void PsBufferWriter::process(const SDataBuffer &buffer)
{
  writeBuffer(buffer);
}

void PsBufferWriter::toPESPackets(QList<MPEG::PESPacket> &packets, const SBuffer &buffer)
{
  const SMemoryBuffer memoryBuffer = buffer;

  if (!memoryBuffer.isNull())
  {
    const qint64 dts = memoryBuffer.decodingTimeStamp().toClock(90000) + 90000;
    const qint64 pts = memoryBuffer.presentationTimeStamp().toClock(90000) + 90000;
    //qDebug() << memoryBuffer.decodedTimeStamp().toMsec() << pts << dts;

    // Put meta and extra data in PES packet (as padding)
    packets.append(MPEG::PESPacket());
    MPEG::PESPacket * const headerPacket = &packets.last();
    headerPacket->initialize(MPEG::PESHeader::StreamId_Padding);
    setTimeStamp(headerPacket, pts, dts);

    BufferHeader * const header = reinterpret_cast<BufferHeader *>(headerPacket->getPayload());
    unsigned headerSize = sizeof(BufferHeader);
    header->syncWord = BufferHeader::SyncWord;
    header->typeId = memoryBuffer.typeId();
    header->subStreamId = quint16(memoryBuffer.subStreamId());
    header->dataSize = memoryBuffer.numBytes();
    header->timeStamp = memoryBuffer.timeStamp();
    header->decodingTimeStamp = memoryBuffer.decodingTimeStamp();
    header->presentationTimeStamp = memoryBuffer.presentationTimeStamp();

    if ((buffer.typeId() & SBuffer::dataTypeIdMask) == (SVideoBuffer::baseTypeId & SBuffer::dataTypeIdMask))
    {
      const SVideoBuffer videoBuffer = buffer;
      VideoBufferHeader * const header = reinterpret_cast<VideoBufferHeader *>(headerPacket->getPayload());
      headerSize = sizeof(VideoBufferHeader);

      header->keyFrame = videoBuffer.isKeyFrame() ? 1 : 0;
      header->__reserved1 = 0;
      header->numChannels = videoBuffer.numChannels();

      for (unsigned i=0; i<SVideoBuffer::maxChannels; i++)
      {
        header->lineSize[i] = videoBuffer.lineSize(i);
        header->offset[i] = videoBuffer.offset(i);
      }
    }

    headerPacket->setPayloadSize(headerSize);

    // Put buffer data in PES packets
    const quint8 streamID = getMPEGStreamID(memoryBuffer);
    const uchar * const data = memoryBuffer.bits();
    const size_t size = memoryBuffer.numBytes();

    for (size_t off=0; off<size; off+=MPEG::PESPacket::maxPayloadSize)
    {
      const size_t packetSize = qMin(size - off, MPEG::PESPacket::maxPayloadSize + 0);

      packets.append(MPEG::PESPacket());
      MPEG::PESPacket * const pesPacket = &packets.last();
      pesPacket->initialize(streamID);
      setTimeStamp(pesPacket, pts, dts);

      if ((streamID == MPEG::PESHeader::StreamId_Private1) && (buffer.codec() == "ac3"))
      { // Prepend AC3 Audio Substream Header
        pesPacket->setPayloadSize(packetSize + 4);
        quint8 * const payload = pesPacket->getPayload();
        payload[0] = 0x80;
        payload[1] = 0x01; // One frame
        payload[2] = 0x00;
        payload[3] = 0x01; // First AC3 packet immediately after this byte
        memcpy(payload + 4, data + off, packetSize);
      }
      else if ((streamID == MPEG::PESHeader::StreamId_Private1) && (buffer.codec() == "dts"))
      { // Prepend DTS Audio Substream Header
        pesPacket->setPayloadSize(packetSize + 4);
        quint8 * const payload = pesPacket->getPayload();
        payload[0] = 0x88;
        payload[1] = 0x01; // One frame
        payload[2] = 0x00;
        payload[3] = 0x01; // First DTS packet immediately after this byte
        memcpy(payload + 4, data + off, packetSize);
      }
      else
      {
        pesPacket->setPayloadSize(packetSize);
        memcpy(pesPacket->getPayload(), data + off, packetSize);
      }
    }
  }
}

void PsBufferWriter::setTimeStamp(MPEG::PESPacket *pesPacket, qint64 pts, qint64 dts)
{
  if ((pts > 0) && (pesPacket->getStreamID() == MPEG::PESHeader::StreamId_Video))
  {
    pesPacket->setPTS(pts);
    if (pts == dts)
      pesPacket->setDTS(pts - 9000);
    else if (pts > dts)
      pesPacket->setDTS(dts - 9000);
  }
  else if (dts > 0)
  {
    pesPacket->setPTS(dts);
    pesPacket->setDTS(dts - 9000);
  }
}

quint8 PsBufferWriter::getMPEGStreamID(const SBuffer &buffer)
{
  if (buffer.typeId() == SAudioBuffer::baseTypeId)
  {
    return ((buffer.codec() == "MP1") || (buffer.codec() == "MP2") || (buffer.codec() == "MP3") || (buffer.codec() == "AAC"))
        ? (MPEG::PESHeader::StreamId_Audio)
        : (MPEG::PESHeader::StreamId_Private1);
  }
  else if (buffer.typeId() == SVideoBuffer::baseTypeId)
  {
    return MPEG::PESHeader::StreamId_Video;
  }

  return MPEG::PESHeader::StreamId_Padding;
}

void PsBufferWriter::fillPSMap(MPEG::PSMap &psMap, const SBuffer &buffer)
{
  if (buffer.typeId() == SAudioBuffer::baseTypeId)
  {
    const quint8 streamID = getMPEGStreamID(buffer);

    MPEG::PSMap::Stream audioStream;
    audioStream.setStreamID(streamID);

    if ((buffer.codec() == "MP1") || (buffer.codec() == "MP2") || (buffer.codec() == "MP3"))
      audioStream.setStreamType(MPEG::PSMap::StreamType_MPEG1Audio);
    //else if (buffer.codec() == "MP2")
    //  audioStream.setStreamType(MPEG::PSMap::StreamType_MPEG2Audio);
    else if (buffer.codec() == "AAC")
      audioStream.setStreamType(MPEG::PSMap::StreamType_AACAudio);
    else if (buffer.codec() == "AC3")
      audioStream.setStreamType(MPEG::PSMap::StreamType_AC3Audio);
    else if (buffer.codec() == "DTS")
      audioStream.setStreamType(MPEG::PSMap::StreamType_DTSAudio);
    else
      audioStream.setStreamType(MPEG::PSMap::StreamType_PrivateData);

    psMap.addStream(audioStream);
  }
  else if (buffer.typeId() == SVideoBuffer::baseTypeId)
  {
    const quint8 streamID = getMPEGStreamID(buffer);

    MPEG::PSMap::Stream videoStream;
    videoStream.setStreamID(streamID);

    if (buffer.codec() == "mpeg1video")
      videoStream.setStreamType(MPEG::PSMap::StreamType_MPEG1Video);
    else if (buffer.codec() == "mpeg2video")
      videoStream.setStreamType(MPEG::PSMap::StreamType_MPEG2Video);
    else if (buffer.codec() == "mpeg4")
      videoStream.setStreamType(MPEG::PSMap::StreamType_MPEG4Video);
    else if (buffer.codec() == "h264")
      videoStream.setStreamType(MPEG::PSMap::StreamType_H264Video);
    else
      videoStream.setStreamType(MPEG::PSMap::StreamType_PrivateData);

    psMap.addStream(videoStream);
  }
}

void PsBufferWriter::writeBuffer(const SMemoryBuffer &buffer)
{
  QList<MPEG::PESPacket> pesPackets;
  toPESPackets(pesPackets, buffer);

  if (pesPackets.count() > 0)
  {
    // Use the audio stream as the clock reference
    if (buffer.typeId() == SAudioBuffer::baseTypeId)
      lastAudioTimeStamp = buffer.presentationTimeStamp();

    // Update the PS map
    bool sendPSMap = false;
    if (buffer.typeId() != SDataBuffer::baseTypeId) // Data buffers are padding, so they don't need to be in the PS map.
    foreach (const MPEG::PESPacket &pesPacket, pesPackets)
    if (pesPacket.isAudioStream() || pesPacket.isVideoStream() || (pesPacket.getStreamID() == MPEG::PESHeader::StreamId_Private1))
    if (!psMap.hasStream(pesPacket.getStreamID()))
    {
      fillPSMap(psMap, buffer);
      psMap.incVersion();

      QByteArray codecs((const char *)codecsPacket.getPayload() + sizeof(CodecMapSyncWord), codecsLen);
      codecs += ("0000000" + QByteArray::number(buffer.streamId(), 16)).right(8) +
                buffer.codec().toByteArray(-1) + "\n";

      if (codecs.count() <= int(codecsPacket.maxPayloadSize - sizeof(CodecMapSyncWord)))
      {
        memcpy(codecsPacket.getPayload() + sizeof(CodecMapSyncWord), codecs.data(), codecs.count());
        codecsPacket.setPayloadSize(codecs.count() + sizeof(CodecMapSyncWord));
        codecsLen = codecs.count();
      }

      sendPSMap = true;

      break;
    }

    if ((lastTypeId != (buffer.typeId() & SBuffer::dataTypeIdMask)) &&
        (buffer.typeId() != SDataBuffer::baseTypeId)) // Data buffers are padding, so they don't need a pack header.
    { // Send the Pack header if the type changes
      MPEG::PSPackHeader psPackHeader;
      psPackHeader.setSCR(lastPackHeaderTimeStamp = lastAudioTimeStamp);
      callback->write(psPackHeader.data, psPackHeader.getTotalLength());

      lastTypeId = buffer.typeId() & SBuffer::dataTypeIdMask;
    }
    else if (lastPackHeaderTimeStamp + STime::fromSec(1) < lastAudioTimeStamp)
    { // Send at least once a second (followed by the last PS map)
      MPEG::PSPackHeader psPackHeader;
      psPackHeader.setSCR(lastAudioTimeStamp);

      callback->write(psPackHeader.data, psPackHeader.getTotalLength());
      callback->write(reinterpret_cast<const uchar *>(&psMap), psMap.getTotalLength());
      callback->write(reinterpret_cast<const uchar *>(&codecsPacket), codecsPacket.getTotalLength());

      sendPSMap = false;
      lastPSMapTimeStamp = lastAudioTimeStamp;
      lastPackHeaderTimeStamp = lastAudioTimeStamp;
    }

    if ((sendPSMap) || (lastPSMapTimeStamp + STime::fromSec(1) < lastAudioTimeStamp))
    { // If the PS map was updated, send it again
      callback->write(reinterpret_cast<const uchar *>(&psMap), psMap.getTotalLength());
      callback->write(reinterpret_cast<const uchar *>(&codecsPacket), codecsPacket.getTotalLength());

      lastPSMapTimeStamp = lastAudioTimeStamp;
    }

    // Finally send the PES packets
    foreach (const MPEG::PESPacket &pesPacket, pesPackets)
      callback->write(reinterpret_cast<const uchar *>(&pesPacket), pesPacket.getTotalLength());
  }
}
*/

} } // End of namespaces
