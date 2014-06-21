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

#include "dvbinput.h"

// AleVT functions
extern "C"
{
  // Hamming
  int hamm8(const quint8 *p, int &err);
  int hamm16(const quint8 *p, int &err);
  int hamm24(const quint8 *p, int &err);
  int chk_parity(char *p, int n);

  // Language parsing
  void conv2latin(char *p, int n, int lang);
}

namespace LXiStream {
namespace LinuxDvbBackend {


QStringList DVBInput::schemes(void)
{
  return QStringList("__linuxdvb_input");
}

DVBInput::DVBInput(DVBDevice *parent, unsigned streamID, quint16 serviceID)
         :SNode(Behavior_Blocking, 0, SCodecList(), parent),
          parent(parent),
          streamID(streamID),
          serviceID(serviceID),
          program(),
          audioDesc(0),
          videoDesc(0),
          teletextDesc(0),
          audioPES(DVBDevice::getPESType(DMX_PES_AUDIO, streamID)),
          videoPES(DVBDevice::getPESType(DMX_PES_VIDEO, streamID)),
          teletextPES(DVBDevice::getPESType(DMX_PES_TELETEXT, streamID)),
          audioCodec("MP2"),
          videoCodec("MPEG2"),
          buffers(250),
          pesAudioPacketStream(this, (Common::MPEG::PESPacketStream::PESCallback)&DVBInput::audioPacketReceived,
                                     (Common::MPEG::PESPacketStream::ADTCallback)&DVBInput::audioAFieldReceived),
          pesVideoPacketStream(this, (Common::MPEG::PESPacketStream::PESCallback)&DVBInput::videoPacketReceived,
                                     (Common::MPEG::PESPacketStream::ADTCallback)&DVBInput::videoAFieldReceived),
          pesTeletextPacketStream(this, (Common::MPEG::PESPacketStream::PESCallback)&DVBInput::teletextPacketReceived,
                                        (Common::MPEG::PESPacketStream::ADTCallback)&DVBInput::teletextAFieldReceived),
          videoKeyFrame(false)
{
  findProgram();
}

DVBInput::~DVBInput()
{
  SDebug::MutexLocker l(&parent->mutex, __FILE__, __LINE__);

  parent->inputDev[streamID] = NULL;
  parent->pesPacketStream[program.audioPID] = NULL;
  parent->pesPacketStream[program.videoPID] = NULL;
  parent->pesPacketStream[program.dataPID] = NULL;

  if (audioDesc > 0)    close(audioDesc);
  if (videoDesc > 0)    close(videoDesc);
  if (teletextDesc > 0) close(teletextDesc);
}

bool DVBInput::prepare(const SCodecList &)
{
  for (unsigned i=0; i<8; i++)
    teletextMagazines[i] = SBuffer();

  timer.setTimeStamp(STime::fromMSec(0));

  return true;
}

bool DVBInput::unprepare(void)
{
  return true;
}

SNode::Result DVBInput::processBuffer(const SBuffer &, SBufferList &output)
{
  if (__builtin_expect((program.audioPID == 0) && (program.videoPID == 0) && (program.dataPID == 0), false))
    findProgram();

  const SBuffer buffer = buffers.wait(SGraph::nonBlockingTaskTimeMs);
  if (!buffer.isNull())
  {
    output << buffer;
    return Result_Active;
  }
  else
    return Result_Blocked;
}

void DVBInput::findProgram(void)
{
  QMap<quint16, DVBDevice::Channel>::ConstIterator c = parent->channels.find(serviceID);

  if (c != parent->channels.end())
  {
    program.audioPID = c->audioPIDs.isEmpty() ? 0 : c->audioPIDs.first();
    program.videoPID = c->videoPID;
    program.dataPID = c->teletextPID;

    const QString demuxDev = "/dev/dvb/adapter" + QString::number(parent->adapterID) + "/demux0";

    if (parent->outputAudio && (audioPES != DMX_PES_OTHER))
    if ((audioDesc = open(demuxDev.toLatin1(), O_RDWR | O_NONBLOCK)) > 0)
    if (ioctl(audioDesc, DMX_SET_BUFFER_SIZE, DVBDevice::bufferSize) >= 0)
    if ((program.audioPID > 0) && (program.audioPID <= Common::MPEG::maxPID))
    {
      dmx_pes_filter_params pes;
      pes.pid = program.audioPID;
      pes.input = DMX_IN_FRONTEND;
      pes.output = DMX_OUT_TS_TAP;
      pes.pes_type = audioPES;
      pes.flags = DMX_IMMEDIATE_START;

      if (ioctl(audioDesc, DMX_SET_PES_FILTER, &pes) >= 0)
      {
        SDebug::MutexLocker l(&parent->mutex, __FILE__, __LINE__);

        parent->pesPacketStream[program.audioPID] = &pesAudioPacketStream;
      }
    }

    if (parent->outputVideo && (videoPES != DMX_PES_OTHER))
    if ((videoDesc = open(demuxDev.toLatin1(), O_RDWR | O_NONBLOCK)) > 0)
    if (ioctl(videoDesc, DMX_SET_BUFFER_SIZE, DVBDevice::bufferSize) >= 0)
    if ((program.videoPID > 0) && (program.videoPID <= Common::MPEG::maxPID))
    {
      dmx_pes_filter_params pes;
      pes.pid = program.videoPID;
      pes.input = DMX_IN_FRONTEND;
      pes.output = DMX_OUT_TS_TAP;
      pes.pes_type = videoPES;
      pes.flags = DMX_IMMEDIATE_START;

      if (ioctl(videoDesc, DMX_SET_PES_FILTER, &pes) >= 0)
      {
        SDebug::MutexLocker l(&parent->mutex, __FILE__, __LINE__);

        parent->pesPacketStream[program.videoPID] = &pesVideoPacketStream;
      }
    }

    if (teletextPES != DMX_PES_OTHER)
    if ((teletextDesc = open(demuxDev.toLatin1(), O_RDWR | O_NONBLOCK)) > 0)
    if (ioctl(teletextDesc, DMX_SET_BUFFER_SIZE, DVBDevice::bufferSize) >= 0)
    if ((program.dataPID > 0) && (program.dataPID <= Common::MPEG::maxPID))
    {
      dmx_pes_filter_params pes;
      pes.pid = program.dataPID;
      pes.input = DMX_IN_FRONTEND;
      pes.output = DMX_OUT_TS_TAP;
      pes.pes_type = teletextPES;
      pes.flags = DMX_IMMEDIATE_START;

      if (ioctl(teletextDesc, DMX_SET_PES_FILTER, &pes) >= 0)
      {
        SDebug::MutexLocker l(&parent->mutex, __FILE__, __LINE__);

        parent->pesPacketStream[program.dataPID] = &pesTeletextPacketStream;
      }
    }
  }
}

void DVBInput::audioPacketReceived(const Common::MPEG::PESPacket *pesPacket, size_t size)
{
  if (pesPacket)
  if (pesPacket->isValid() && (pesPacket->isAudioStream() || (pesPacket->getStreamID() == Common::MPEG::PESHeader::StreamId_Private1))) // Private1 for AC3
  {
    const size_t payloadSize = pesPacket->calculatePayloadSize(size);
    SAudioBuffer buffer(payloadSize);

    memcpy(buffer.bits(), pesPacket->getPayload(), payloadSize);
    buffer.setNumBytes(payloadSize);
    buffer.setTimeStamp(timer.correctTimeStamp(STime::fromClock(pesPacket->getDTS(), 90000), STime::fromSec(1)));
    buffer.setDecodingTimeStamp(STime::fromClock(pesPacket->getDTS(), 90000));
    buffer.setPresentationTimeStamp(STime::fromClock(pesPacket->getPTS(), 90000));
    buffer.setCodec(audioCodec);

    buffers.enqueue(buffer);
  }
}

void DVBInput::audioAFieldReceived(const Common::MPEG::TSPacket::AdaptationField *)
{
}

void DVBInput::videoPacketReceived(const Common::MPEG::PESPacket *pesPacket, size_t size)
{
  if (pesPacket)
  if (pesPacket->isValid() && pesPacket->isVideoStream())
  {
    const size_t payloadSize = pesPacket->calculatePayloadSize(size);
    SVideoBuffer buffer(payloadSize);

    memcpy(buffer.bits(), pesPacket->getPayload(), payloadSize);
    buffer.setNumBytes(payloadSize);

    buffer.setTimeStamp(timer.correctTimeStamp(STime::fromClock(pesPacket->getDTS(), 90000), STime::fromSec(1)));
    buffer.setDecodingTimeStamp(STime::fromClock(pesPacket->getDTS(), 90000));
    buffer.setPresentationTimeStamp(STime::fromClock(pesPacket->getPTS(), 90000));
    buffer.setCodec(videoCodec);
    buffer.setKeyFrame(videoKeyFrame);
    videoKeyFrame = false;

    buffers.enqueue(buffer);
  }
}

void DVBInput::videoAFieldReceived(const Common::MPEG::TSPacket::AdaptationField *aField)
{
  if (aField)
  if (aField->hasRandomAccessIndicator())
    videoKeyFrame = true;
}

void DVBInput::teletextPacketReceived(const Common::MPEG::PESPacket *pesPacket, size_t size)
{
  if (pesPacket)
  if (pesPacket->isValid() && (pesPacket->getStreamID() == Common::MPEG::PESHeader::StreamId_Private1))
  {
    const size_t payloadSize = pesPacket->calculatePayloadSize(size);
    const quint8 * const payload = pesPacket->getPayload();
    const quint8 dataIdentifier = payload[0];

    if ((dataIdentifier >= 0x10) && (dataIdentifier <= 0x1F)) // EBU Data
    for (size_t i=1; (i+2)<payloadSize; i+=2)
    {
      const quint8 unitID = payload[i];
      const quint8 unitLen = payload[i+1];

      if ((unitID == 0x02) || (unitID == 0x03))
      {
        const quint8 * const unitData = payload + i + 2;

        // Reverse all bits
        quint8 reversedData[44];
        for (unsigned j=1; j<44; j++)
          reversedData[j] = DVBDevice::reversedBits[unitData[j]];

        int errors = 0;
        //const quint8 line = unitData[0] & 0x1F;
        const quint8 framingCode = reversedData[1];
        const quint8 address = hamm16(reversedData + 2, errors);
        const quint8 magazine = address & 0x07;
        const quint8 pagerow = address >> 3;

        //std::cout << "TT: framingCode = " << (void *)framingCode << "  address = " << (void *)address << "  magazine = " << (int)magazine << "  pagerow = " << (int)pagerow << std::endl;

        if (framingCode == 0x27)
        {
          SDataBuffer * const buffer = static_cast<SDataBuffer *>(teletextMagazines + magazine);
          if (pagerow == 0) // New page
          {
            // Submit the previous page, if present.
            if (!buffer->isNull())
              buffers.enqueue(*buffer);

            // Get a new buffer
            *buffer = SDataBuffer(sizeof(SDataBuffer::TeletextPage));

            memset(buffer->bits(), 0, sizeof(SDataBuffer::TeletextPage));
            buffer->setNumBytes(sizeof(SDataBuffer::TeletextPage));
            buffer->setTimeStamp(timer.correctTimeStamp(STime::fromClock(pesPacket->getDTS(), 90000), STime::fromSec(1)));
            buffer->setDecodingTimeStamp(STime::fromClock(pesPacket->getDTS(), 90000));
            buffer->setPresentationTimeStamp(STime::fromClock(pesPacket->getPTS(), 90000));
            buffer->setCodec(SDataCodec::Format_TeletextPage);

            SDataBuffer::TeletextPage * const page = reinterpret_cast<SDataBuffer::TeletextPage *>(buffer->bits());

            // Copy the line
            memcpy(page->data[0], reversedData + 4, 40);

            // Decode the Hamming codes
            const int h1  = hamm16(reversedData + 4    , errors); // page number
            const int h2  = hamm16(reversedData + 4 + 2, errors); // subpage number + flags
            const int h3  = hamm16(reversedData + 4 + 4, errors); // subpage number + flags
            const int h4  = hamm16(reversedData + 4 + 6, errors); // language code + more flags

            page->errors  = (errors >> 8) + chk_parity(page->data[0] + 8, 40 - 8); // First eight bytes are Hamming codes.
            page->pgno    = (int((magazine == 0) ? 8 : magazine) << 8) | h1;
            page->subno   = (h2 | (h3 << 8)) & 0x3F7F;
            page->lang    = "\0\4\2\6\1\5\3\7"[h4 >> 5];// + (latin1 ? 0 : 8);
            page->flags   = h4 & 0x1f;
            page->flags  |= h3 & 0xc0;
            page->flags  |= (h2 & 0x80) >> 2;
            page->lines   = 0x00000001; // Line 0 present
            page->flof    = 0;

            // Convert language specific characters
            conv2latin(page->data[0] + 8, 40 - 8, page->lang); // First eight bytes are Hamming codes.
          }
          else if (!buffer->isNull() && (pagerow < 24))
          {
            SDataBuffer::TeletextPage * const page = reinterpret_cast<SDataBuffer::TeletextPage *>(buffer->bits());

            // Copy the line
            memcpy(page->data[pagerow], reversedData + 4, 40);
            page->errors += chk_parity(page->data[pagerow], 40);
            page->lines  |= (1 << pagerow);

            // Convert language specific characters
            conv2latin(page->data[pagerow], 40, page->lang);
          }
        }
      }

      i += unitLen;
    }
  }
}

void DVBInput::teletextAFieldReceived(const Common::MPEG::TSPacket::AdaptationField *)
{
}



} } // End of namespaces
