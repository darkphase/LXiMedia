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

#include "mpeg.h"

namespace LXiStream {
namespace Common {
namespace MPEG {


TSPacket::AdaptationField * TSPacket::getAdaptationField(void)
{
  if (hasAdaptationField() && !isCorrupt())
    return reinterpret_cast<TSPacket::AdaptationField *>(data + 4);
  else
    return NULL;
}

const TSPacket::AdaptationField * TSPacket::getAdaptationField(void) const
{
  if (hasAdaptationField() && !isCorrupt())
    return reinterpret_cast<const TSPacket::AdaptationField *>(data + 4);
  else
    return NULL;
}

quint8 * TSPacket::getPayload(void)
{
  const size_t payloadSize = getPayloadSize();

  if (payloadSize > 0)
    return data + tsPacketSize - payloadSize;
  else
    return NULL;
}

const quint8 * TSPacket::getPayload(void) const
{
  const size_t payloadSize = getPayloadSize();

  if (payloadSize > 0)
    return data + tsPacketSize - payloadSize;
  else
    return NULL;
}

size_t TSPacket::getPayloadSize(void) const
{
  if (hasPayload() && !isCorrupt())
  {
    if (hasAdaptationField())
    {
      const TSPacket::AdaptationField * const aField = getAdaptationField();

      if (aField)
      {
        const size_t afieldSize  = aField->getLength();

        if (afieldSize < tsMaxPayloadSize)
          return tsMaxPayloadSize - afieldSize;
      }
    }
    else
      return tsMaxPayloadSize;
  }

  return 0;
}

size_t TSPacket::setPayload(const quint8 *payload, size_t payloadSize)
{
  size_t maxPayloadSize = tsMaxPayloadSize;

  if (hasAdaptationField())
  {
    const TSPacket::AdaptationField * const aField = getAdaptationField();

    if (aField)
    {
      const size_t afieldSize  = aField->getLength();

      if (afieldSize < tsMaxPayloadSize)
        maxPayloadSize -= afieldSize;
      else
        maxPayloadSize = 0;
    }
  }

  payloadSize = qMin(payloadSize, maxPayloadSize);
  memcpy(data + 4, payload, payloadSize);
  data[3] |= 0x10;

  return payloadSize;
}

quint64 TSPacket::AdaptationField::getPCR(void) const
{
  if (hasPCR())
  {
     return (quint64(data[2]) << 25) |
            (quint64(data[3]) << 17) |
            (quint64(data[4]) << 9) |
            (quint64(data[5]) << 1) |
            (quint64(data[6]) >> 7);
  }
  else
    return 0;
}

quint64 TSPacket::AdaptationField::getOPCR(void) const
{
  if (hasOPCR())
  {
    if (!hasPCR())
    {
      return (quint64(data[2]) << 25) |
              (quint64(data[3]) << 17) |
              (quint64(data[4]) << 9) |
              (quint64(data[5]) << 1) |
              (quint64(data[6]) >> 7);
    }
    else
    {
      return (quint64(data[7] & 0x3F) << 27) |
              (quint64(data[8]) << 19) |
              (quint64(data[9]) << 11) |
              (quint64(data[10]) << 3) |
              (quint64(data[11]) >> 5);
    }
  }
  else
    return 0;
}

qint8 TSPacket::AdaptationField::getSpliceCountdown(void) const
{
  if (hasSplicingField())
  {
    if (!hasPCR() && !hasOPCR())
      return qint8(data[2]);
    else if ((hasPCR() && !hasOPCR()) || (!hasPCR() && hasOPCR()))
      return qint8(quint8(data[7] << 2) | quint8(data[8] >> 6));
    else
      return qint8(quint8(data[12] << 4) | quint8(data[13] >> 4));
  }
  else
    return 0;
}

void PESPacket::initialize(quint8 streamID)
{
  memset(data, 0, 32);
  data[2] = 0x01;
  data[3] = streamID;

  if (isAudioStream() || isVideoStream() || (getStreamID() == StreamId_Private1))
  {
    _data[0] = 0x80;
    _data[1] = 0x00;
    _data[2] = 0x00;
  }
}

quint64 PESPacket::getPTS(void) const
{
  if (hasPTS() && (getHeaderLength() >= 14))
    return (quint64(_data[3] & 0x0E) << 29) |  // Bits 30..32
           (quint64(_data[4])        << 22) |  // Bits 22..29
           (quint64(_data[5] & 0xFE) << 14) |  // Bits 15..21
           (quint64(_data[6])        << 7 ) |  // Bits 14..6
           (quint64(_data[7] & 0xFE) >> 1 );   // Bits  6..0
  else
    return 0;
}

void PESPacket::setPTS(quint64 pts)
{
  _data[ 1] |= 0x80; // Set PTS flag
  _data[ 3]  = 0x21 | quint8((pts >> 29) & 0x0E);
  _data[ 4]  =        quint8((pts >> 22) & 0xFF);
  _data[ 5]  = 0x01 | quint8((pts >> 14) & 0xFE);
  _data[ 6]  =        quint8((pts >> 7 ) & 0xFF);
  _data[ 7]  = 0x01 | quint8((pts << 1 ) & 0xFE);

  if (getHeaderLength() < 14)
    setHeaderLength(14);
}

quint64 PESPacket::getDTS(void) const
{
  if (hasDTS() && (getHeaderLength() >= 19))
    return (quint64(_data[ 8] & 0x0E) << 29) |  // Bits 30..32
           (quint64(_data[ 9])        << 22) |  // Bits 22..29
           (quint64(_data[10] & 0xFE) << 14) |  // Bits 15..21
           (quint64(_data[11])        << 7 ) |  // Bits 14..6
           (quint64(_data[12] & 0xFE) >> 1 );   // Bits  6..0
  else
    return 0;
}

void PESPacket::setDTS(quint64 dts)
{
  _data[ 1] |= 0x40; // Set DTS flag
  _data[ 3] |= 0x31;
  _data[ 8]  = 0x01 | quint8((dts >> 29) & 0x0E);
  _data[ 9]  =        quint8((dts >> 22) & 0xFF);
  _data[10]  = 0x01 | quint8((dts >> 14) & 0xFE);
  _data[11]  =        quint8((dts >> 7 ) & 0xFF);
  _data[12]  = 0x01 | quint8((dts << 1 ) & 0xFE);

  if (getHeaderLength() < 19)
    setHeaderLength(19);
}

size_t PESPacket::calculatePayloadSize(size_t size) const
{
  if (hasOptionalHeader())
  {
    const size_t length = getLength();

    return ((length > 0) ? (length + sizeof(PESHeader)) : size) - getHeaderLength();
  }
  else
    return size - sizeof(PESHeader);
}

const quint8 * PESPacket::getPayload(void) const
{
  if (hasOptionalHeader())
    return data + getHeaderLength();
  else
    return data + sizeof(PESHeader);
}

quint8 * PESPacket::getPayload(void)
{
  if (hasOptionalHeader())
    return data + getHeaderLength();
  else
    return data + sizeof(PESHeader);
}

void PSPackHeader::initialize(void)
{
  memset(data, 0, sizeof(data));
  data[2] = 0x01; // Start code
  data[3] = PESHeader::StreamId_PackHeader; // Start code
  setSCR(STime::null);
  setMuxRate(20000);
  data[13] = 0xF8; // reserved + pack_stuffing_length
}

void PSPackHeader::setSCR(STime timeStamp)
{
  const quint64 base = timeStamp.toClock(90000);
  const quint16 ext = 0;

  data[4]  = quint8(base >> 27) & 0x38;   // Base bits 32..30
  data[4] |= quint8(base >> 28) & 0x03;   // Base bits 29..28
  data[4] |= 0x44;                        // Marker bits
  data[5]  = quint8(base >> 20);          // Base bits 27..20
  data[6]  = quint8(base >> 12) & 0xF8;   // Base bits 19..15
  data[6] |= quint8(base >> 13) & 0x03;   // Base bits 14..13
  data[6] |= 0x04;                        // Marker bit
  data[7]  = quint8(base >> 5);           // Base bits 12..5
  data[8]  = quint8(base << 3);           // Base bits 4..0
  data[8] |= quint8(ext >> 7) & 0x03;     // Ext bits 8..7
  data[8] |= 0x04;                        // Marker bit
  data[9]  = quint8(ext << 1);            // Ext bits 6..0
  data[9] |= 0x01;                        // Marker bit
}

void PSPackHeader::setMuxRate(quint32 muxRate)
{
  data[10]  = quint8(muxRate >> 14);      // Bits 21..14
  data[11]  = quint8(muxRate >> 6);       // Bits 13..6
  data[12]  = quint8(muxRate << 2);       // Bits 5..0
  data[12] |= 0x03; // Marker bits
}

size_t PSPackHeader::getStuffing(quint8 *stuffing, size_t maxSize) const
{
  const size_t size = data[13] & 0x07;

  if (stuffing && (size > 0) && (maxSize > 0))
    memcpy(stuffing, data + 14, qMin(size, maxSize));

  return size;
}

void PSPackHeader::setStuffing(const quint8 *stuffing, size_t size)
{
  size &= 0x07;

  data[13] &= 0xF8;
  data[13] |= quint8(size);
  memcpy(data + 14, stuffing, size);
}


void PSSystemHeader::initialize(void)
{
  static const quint32 rate_bound = 20000;
  static const quint8  audio_bound = 1;
  static const quint8  video_bound = 1;

  memset(data, 0, sizeof(data));
  memset(_data, 0, sizeof(_data));
  data[2] = 0x01; // Start code
  data[3] = 0xBB; // Start code
  setLength(7);

  _data[0] = 0x80; // Marker bit
  _data[1] = quint8(rate_bound >> 15);
  _data[2] = quint8(rate_bound >> 7);
  _data[3] = quint8(rate_bound << 1) | 0x01; // Marker bit
  _data[4] = quint8(audio_bound << 2);
  _data[5] = video_bound | 0x20; // Marker bit
  _data[6] = 0xFF; // packet_rate_restriction_flag + reserved
}

void PSMap::initialize(void)
{
  memset(data, 0, 32);
  data[2] = 0x01;
  data[3] = 0xBC;
  _data[0] = 0x80;
  _data[1] = 0x01;

  setLength(16 - sizeof(PESHeader));
}

bool PSMap::hasStream(quint8 streamID) const
{
  quint16 mapLen = getMapLength();
  const quint16 ofs = getInfoLength();

  for (unsigned i=0; i<mapLen; i+=4)
  {
    const Stream * const stream = reinterpret_cast<const Stream *>(data + ofs + 12 + i);
    if (stream->getStreamID() == streamID)
      return true;

    i += stream->getDescLength();
  }

  return false;
}

void PSMap::addStream(const Stream &stream)
{
  quint16 mapLen = getMapLength();
  const quint16 ofs = getInfoLength();
  const quint16 sstart = ofs + mapLen + 12;

  memcpy(data + sstart, stream.data, sizeof(stream.data));
  mapLen += stream.getDescLength() + sizeof(stream.data);

  _data[4 + ofs] = mapLen >> 8;
  _data[5 + ofs] = mapLen & 0xFF;

  setLength((ofs + mapLen + 16) - sizeof(PESHeader));
}


/*! This method retrieves (part of) a TS packet from the specified block of
    data. The TSCallback function is invoked for each TS packet found.
 */
void TSPacketStream::processData(const quint8 *data, size_t size)
{
  off_t pos = 0;

  // Process temporary buffer
  if ((bufferUse > 0) && (size >= (sizeof(buffer) - bufferUse)))
  {
    memcpy(buffer + bufferUse, data, sizeof(buffer) - bufferUse);
    data += bufferUse;
    size -= bufferUse;

    (stream->*tsCallback)(reinterpret_cast<const TSPacket *>(buffer));
  }

  bufferUse = 0;

  // Process buffer
  while (pos + tsPacketSize <= size)
  if (data[pos] == tsSyncByte)
  {
    (stream->*tsCallback)(reinterpret_cast<const TSPacket *>(data + pos));
    pos += tsPacketSize;
  }
  else
    pos += syncStream(data + pos, size - pos);

  // Store remaining data in buffer
  if ((pos >= 0) && (pos < off_t(size)))
  {
    bufferUse = qMin(size_t(size - pos), sizeof(buffer));
    memcpy(buffer, data + pos, bufferUse);
  }
}

/*! Looks for two TS sync bytes at the appropriate positions. Returns the offset
    where the bytes are found. If none are found, size is returned.
 */
off_t TSPacketStream::syncStream(const quint8 *data, size_t size)
{
  for (off_t i=0; i + tsPacketSize < size; i++)
  if ((data[i] == tsSyncByte) && (data[i + tsPacketSize] == tsSyncByte))
    return i;

  return size;
}

/*! Resets the packet stream, flushes any intermediate results.
 */
void PESPacketStream::reset(void)
{
  bufferUse = 0;
  bufferPESLength = 0;
}

/*! This method retrieves (part of) a PES packet from the specified TS packet
    and invokes the PESCallback function if it is complete. The ADTCallback
    function is invoked if the tsPacket has an adaptation field.
 */
void PESPacketStream::processPacket(const TSPacket *tsPacket)
{
  if (tsPacket)
  if (tsPacket->isValid())
  {
    if (tsPacket->isPayloadUnitStart() && (bufferUse > 0))
    { // New PES, emit previous one
      (stream->*pesCallback)(reinterpret_cast<const PESPacket *>(buffer), bufferUse);
      bufferUse = 0;
      bufferPESLength = 0;
    }

    if (tsPacket->hasAdaptationField())
      (stream->*adtCallback)(tsPacket->getAdaptationField());

    const quint8 * const payload = tsPacket->getPayload();
    const size_t payloadSize = tsPacket->getPayloadSize();

    if (bufferUse + payloadSize <= sizeof(buffer))
    {
      memcpy(buffer + bufferUse, payload, payloadSize);
      bufferUse += payloadSize;
    }

    // Determine PES packet size (so we can get the last PES packet properly
    if ((bufferPESLength == 0) && (bufferUse > sizeof(PESHeader)))
      bufferPESLength = getPacketLen(buffer, bufferUse);

    // If packet is complete; emit it.
    if ((bufferPESLength > 0) && (bufferUse >= bufferPESLength))
    {
      (stream->*pesCallback)(reinterpret_cast<const PESPacket *>(buffer), bufferUse);
      bufferUse = 0;
      bufferPESLength = 0;
    }
  }
}

/*! This method retrieves (part of) a PES packet from the specified block of
    data. The PESCallback function is invoked for each PES packet found.

    \note The ADTCallback function is not invoked from this method.
 */
void PESPacketStream::processData(const quint8 *data, size_t size)
{
  if ((bufferUse + size) < sizeof(buffer)) // Append data
  {
    memcpy(buffer + bufferUse, data, size);
    bufferUse += size;
  }
  else // Overflow, clear buffer
    bufferUse = 0;

  // Try to get complete packets
  size_t off = 0;

  // Go to the next header
  for (off = 0; (off + sizeof(PESHeader)) < bufferUse; off++)
  if (reinterpret_cast<const PESHeader *>(buffer + off)->isValid())
    break;

  // Decode packets
  while ((off < bufferUse) && ((bufferUse - off) >= sizeof(PESHeader)))
  {
    const size_t packetLen = getPacketLen(buffer + off, bufferUse - off);

    if ((packetLen > sizeof(PESHeader)) && ((bufferUse - off) >= packetLen))
    {
      (stream->*pesCallback)(reinterpret_cast<const PESPacket *>(buffer + off), packetLen);
      off += packetLen;
    }
    else
      break;
  }

  // Move remaining buffer
  if (off > 0)
  {
    memmove(buffer, buffer + off, bufferUse - off);
    bufferUse -= off;
  }
}

size_t PESPacketStream::getPacketLen(const quint8 *data, size_t size)
{
  if (size >= sizeof(PESHeader))
  {
    const PESPacket * const pesPacket = reinterpret_cast<const PESPacket *>(data);

    if (pesPacket->isValid())
    {
      if (pesPacket->getStreamID() == PESPacket::StreamId_PackHeader)
      {
        if (size >= PSPackHeader::baseLength)
          return reinterpret_cast<const PSPackHeader *>(pesPacket)->getTotalLength();
      }
      else if (pesPacket->getLength() > 0)
        return pesPacket->getTotalLength();
    }
  }

  return 0;
}


} } } // End of namespaces
