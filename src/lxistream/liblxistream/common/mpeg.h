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

#ifndef LXSTREAMCOMMON_MPEG_H
#define LXSTREAMCOMMON_MPEG_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {


/*! This namespace provides MPEG specific structures and helper classes.

    \author A.J. Admiraal
 */
namespace MPEG {

////////////////////////////////////////////////////////////////////////////////
// Constants
static const size_t             tsPacketSize = 188;
static const size_t             tsMaxPayloadSize = tsPacketSize - 4;
static const quint8             tsSyncByte = 0x47;
static const quint16            maxPID = 0x1FFF;

static const quint8             programEndCode[4] = { 0x00, 0x00, 0x01, 0xB9 };

////////////////////////////////////////////////////////////////////////////////
// Structures

#if defined(__GNUC__)
# define packed __attribute__((packed))
#elif defined(_MSC_VER)
# pragma pack(1)
# define packed
#endif

/*! This structure represents an MPEG-2 Part 1 Transport Stream packet. This is
    a 188 byte packet containing the actual MPEG data. It is the elementary
    packet for DVB transmissions.

    \author A.J. Admiraal
 */
struct packed TSPacket
{
  /*! This structure represents an MPEG-2 Part 1 adaptation field.

    \author A.J. Admiraal
  */
  struct packed AdaptationField
  {
    inline size_t               getLength(void) const                           { return size_t(data[0]) + 1; }     //!< Returns the length of the adaptation field in bytes
    inline bool                 hasDiscontinuity(void) const                    { return (data[1] & 0x80) != 0; }   //!< Returns true if the Discontinuity Indicator is set.
    inline bool                 hasRandomAccessIndicator(void) const            { return (data[1] & 0x40) != 0; }   //!< Returns true if the Random Access Indicator is set.
    inline bool                 isHighPriority(void) const                      { return (data[1] & 0x20) != 0; }   //!< Returns true if the Elementary Stream Priority Indicator is set.
    inline bool                 hasPCR(void) const                              { return (data[1] & 0x10) != 0; }   //!< Returns true if a PCR is present.
    inline bool                 hasOPCR(void) const                             { return (data[1] & 0x08) != 0; }   //!< Returns true if an OPCR is present.
    inline bool                 hasSplicingField(void) const                    { return (data[1] & 0x04) != 0; }   //!< Returns true if a Splicing field is present.
    inline bool                 hasPrivateData(void) const                      { return (data[1] & 0x02) != 0; }   //!< Returns true if private data is present.
    inline bool                 hasExtension(void) const                        { return (data[1] & 0x01) != 0; }   //!< Returns true if an extension is present.

    quint64                     getPCR(void) const;                             //!< Returns the PCR, or 0 if not present.
    quint64                     getOPCR(void) const;                            //!< Returns the OPCR, or 0 if not present.
    qint8                       getSpliceCountdown(void) const;                 //!< Returns the Splice countdown, or 0 if not present.

    quint8                      data[tsPacketSize - 4];                         //!< Holds the raw field.
  };

  inline                        TSPacket(void)                                  { memset(data, 0, sizeof(data)); data[0] = tsSyncByte; }

  inline bool                   isValid(void) const                             { return data[0] == tsSyncByte; }   //!< Returns true if the sync byte is correct.
  inline bool                   isCorrupt(void) const                           { return (data[1] & 0x80) != 0; }   //!< Returns true if the Transport Error Indicator is set.
  inline bool                   isPayloadUnitStart(void) const                  { return (data[1] & 0x40) != 0; }   //!< Returns true if the Payload Unit Start Indicator is set.
  inline bool                   isEncrypted(void) const                         { return (data[3] & 0x80) != 0; }   //!< Returns true if the packet is encrypted.
  inline bool                   usesOddKey(void) const                          { return (data[3] & 0x40) != 0; }   //!< Returns true if the encryption uses the odd key (otherwise it uses the even key).
  inline bool                   hasAdaptationField(void) const                  { return (data[3] & 0x20) != 0; }   //!< Returns true if an adaptation field is present.
  inline bool                   hasPayload(void) const                          { return (data[3] & 0x10) != 0; }   //!< Returns true if a payload is present.
  inline quint16                getPID(void) const                              { return (quint16(data[1] & 0x1F) << 8) | quint16(data[2]); } //!< Returns the PID of the packet.
  inline quint8                 getContinuityCounter(void) const                { return data[3] & 0x0F; }          //!< Returns the 4-bit continuity counter.

  inline void                   setCorrupt(bool c)                              { if (c) data[1] |= 0x80; else data[1] &= ~0x80; }  //!< Sets the Transport Error Indicator.
  inline void                   setPayloadUnitStart(bool p)                     { if (p) data[1] |= 0x40; else data[1] &= ~0x40; }  //!< Sets the Payload Unit Start Indicator.
  inline void                   setEncrypted(bool e)                            { if (e) data[3] |= 0x80; else data[3] &= ~0x80; }  //!< Sets the bit that the packet is encrypted.
  inline void                   setPID(quint16 p)                               { data[1] &= ~0x1F; data[1] |= ((p >> 8) & 0x1F); data[2] = (p & 0xFF); } //!< Sets the PID of the packet.
  inline void                   setContinuityCounter(quint8 c)                  { data[3] &= ~0x0F; data[3] |= (c & 0x0F); }          //!< Sets the 4-bit continuity counter.

  AdaptationField             * getAdaptationField(void);                       //!< Returns a pointer to the adaptation field, or NULL if not present.
  const AdaptationField       * getAdaptationField(void) const;                 //!< Returns a pointer to the adaptation field, or NULL if not present.

  quint8                      * getPayload(void);                               //!< Returns a pointer to the payload, or NULL if not present.
  const quint8                * getPayload(void) const;                         //!< Returns a pointer to the payload, or NULL if not present.
  size_t                        getPayloadSize(void) const;                     //!< Returns the size of the payload in bytes.
  size_t                        setPayload(const quint8 *, size_t);

  quint8                        data[tsPacketSize];                             //!< Holds the raw packet.
};


/*! This structure represents an MPEG-2 Part 1 Packetized Elementary Stream
    header.

    \author A.J. Admiraal
 */
struct packed PESHeader
{
  enum StreamId
  {
    StreamId_Audio            = 0xC0,
    StreamId_Video            = 0xE0,
    StreamId_Private1         = 0xBD,
    StreamId_Padding          = 0xBE,
    StreamId_Private2         = 0xBF,
    StreamId_PackHeader       = 0xBA
  };

  inline                        PESHeader(void)                                 { memset(data, 0, sizeof(data)); data[2] = 0x01; data[3] = 0xBE; }  //!< Creates an empty, valid, header. The stream ID is set to "padding".

  inline bool                   isValid(void) const                             { return (data[0] == 0x00) && (data[1] == 0x00) && (data[2] == 0x01); } //!< Returns true if the start code prefix is valid.
  inline quint8                 isAudioStream(void) const                       { return (getStreamID() >= StreamId_Audio) && (getStreamID() <= (StreamId_Audio + 0x1F)); }  //!< Returns true if the stream ID is an audio stream ID.
  inline quint8                 isVideoStream(void) const                       { return (getStreamID() >= StreamId_Video) && (getStreamID() <= (StreamId_Video + 0x0F)); }  //!< Returns true if the stream ID is a video stream ID.
  inline quint8                 getStreamID(void) const                         { return data[3]; }                 //!< Returns the stream ID.
  inline quint16                getLength(void) const                           { return (quint16(data[4]) << 8) | quint16(data[5]); }  //!< Returns the length of the packet (excluding the header).
  inline void                   setLength(quint16 len)                          { data[4] = len >> 8; data[5] = len & 0xFF; }   //!< Sets the length of the packet (excluding the header).
  inline void                   incLength(int len)                              { setLength(quint16(int(getLength()) + len)); }   //!< Increases the length of the packet by len bytes.
  inline size_t                 getTotalLength(void) const                      { return size_t(getLength()) + sizeof(PESHeader); } //!< Returns the length of the packet (including the header).

  static const size_t           maxPayloadSize = 65506;                         //!< The maximum allowed payload size.
  quint8                        data[6];                                        //!< Holds the raw header, the rest of the packet follows this.
};


/*! This structure represents an MPEG-2 Part 1 Packetized Elementary Stream
    packet.

    \author A.J. Admiraal
 */
struct packed PESPacket : PESHeader
{
  inline                        PESPacket(void)                                 { memset(_data, 0, 32 - sizeof(PESHeader)); }

  void                          initialize(quint8 streamID);                    //!< Initializes the packet and sets the stream ID.

  inline bool                   hasOptionalHeader(void) const                   { return ((_data[0] & 0xC0) == 0x80) && (isAudioStream() || isVideoStream() || (getStreamID() == StreamId_Private1)); } //!< Returns true if the optional header is present.
  inline size_t                 getHeaderLength(void) const                     { return hasOptionalHeader() ? (size_t(_data[2]) + 9) : sizeof(PESHeader); } //!< Returns the total size of the header (including the optional header).
  inline void                   setHeaderLength(size_t len)                     { _data[2] = len - 9; }  //!< Sets the total size of the header (including the optional header).
  inline bool                   hasPTS(void) const                              { return (_data[1] & 0x80) != 0; } //!< Returns true if the optional header contains a PTS.
  inline bool                   hasDTS(void) const                              { return (_data[1] & 0x40) != 0; } //!< Returns true if the optional header contains a DTS.

  quint64                       getPTS(void) const;                             //!< Returns the PTS, or 0 if not present.
  void                          setPTS(quint64);                                //!< Sets the PTS.
  quint64                       getDTS(void) const;                             //!< Returns the DTS, or 0 if not present.
  void                          setDTS(quint64);                                //!< Sets the DTS.

  size_t                        calculatePayloadSize(size_t size) const;        //!< Returns the payload size, the size parameter needs to contain the size of the packet.
  inline void                   setPayloadSize(size_t len)                      { setLength((getHeaderLength() - sizeof(PESHeader)) + len); } //!< Sets the payload size.
  quint8                      * getPayload(void);                               //!< Returns a pointer to the payload.
  const quint8                * getPayload(void) const;                         //!< Returns a pointer to the payload.

  quint8                        _data[65535];                                   //!< Contains the raw data of the packet, excluding the header, use PESHeader::data instead.
};


/*! This structure represents an MPEG-2 Part 1 Program Stream Map.

    \author A.J. Admiraal
 */
struct packed PSMap : PESHeader
{
  enum StreamType
  {
    StreamType_MPEG1Video     = 0x01,
    StreamType_MPEG2Video     = 0x02,
    StreamType_MPEG1Audio     = 0x03,
    StreamType_MPEG2Audio     = 0x04,
    StreamType_PrivateSection = 0x05,
    StreamType_PrivateData    = 0x06,
    StreamType_AACAudio       = 0x0F,
    StreamType_MPEG4Video     = 0x10,
    StreamType_H264Video      = 0x1B,
    StreamType_AC3Audio       = 0x81,
    StreamType_DTSAudio       = 0x8A
  };

  struct packed Stream
  {
    inline                      Stream(void)                                    { memset(data, 0, sizeof(data)); }
    inline StreamType           getStreamType(void) const                       { return StreamType(data[0]); }
    inline void                 setStreamType(StreamType type)                  { data[0] = quint8(type); }
    inline quint8               getStreamID(void) const                         { return data[1]; }
    inline void                 setStreamID(quint8 id)                          { data[1] = id; }
    inline quint8               getDescLength(void) const                       { return (quint16(data[2]) << 8) | quint16(data[3]); }

    quint8                      data[4];
  };

  inline                        PSMap(void)                                     { initialize(); }

  void                          initialize();

  inline bool                   isCurrent(void) const                           { return (_data[0] & 0x80) != 0; }
  inline void                   setCurrent(bool current)                        { if (current) _data[0] |= 0x80; else _data[0] &= 0x7F; }
  inline quint8                 getVersion(void) const                          { return _data[0] & 0x1F; }
  inline void                   incVersion(void)                                { _data[0] = ((getVersion() + 1) & 0x1F) | (_data[0] & 0xE0); }

  inline quint16                getInfoLength(void) const                       { return (quint16(_data[2]) << 8) | quint16(_data[3]); }
  inline quint16                getMapLength(void) const                        { const quint16 ofs = getInfoLength(); return (quint16(_data[4 + ofs]) << 8) | quint16(_data[5 + ofs]); }

  bool                          hasStream(quint8) const;
  void                          addStream(const Stream &);

  quint8                        _data[1018];                                    //!< Contains the raw data of the packet, excluding the header, use PESHeader::data instead.
};


/*! This structure represents an MPEG-2 Part 1 Program Stream Pack Header.

    \author A.J. Admiraal
 */
struct packed PSPackHeader
{
  inline                        PSPackHeader(void)                              { initialize(); }

  void                          initialize();

  inline size_t                 getTotalLength(void) const                      { return baseLength + size_t(data[13] & 0x07); } //!< Returns the length of the packet.

  void                          setSCR(STime);                                  //!< Sets the System Clock Reference.
  void                          setMuxRate(quint32);                            //!< Sets the Multiplexer rate.
  size_t                        getStuffing(quint8 *, size_t) const;            //!< Returns the stuffing
  void                          setStuffing(const quint8 *, size_t);            //!< Sets the stuffing

  static const size_t           baseLength = 14;
  quint8                        data[baseLength + 8];
};


/*! This structure represents an MPEG-2 Part 1 Program Stream System Header.

    \author A.J. Admiraal
 */
struct packed PSSystemHeader : PESHeader
{
  inline                        PSSystemHeader(void)                            { initialize(); }

  void                          initialize();

  quint8                        _data[64];                                      //!< Contains the raw data of the packet, excluding the header, use PESHeader::data instead.
};

#ifdef _MSC_VER
#pragma pack()
#endif
#undef packed

////////////////////////////////////////////////////////////////////////////////
// Support classes
/*! Dummy class for callback functions

    \author A.J. Admiraal
    \sa TSPacketStream, PESPacketStream
 */
class Stream
{
};

/*! This support class decodes transport-stream packets from a raw data stream.
    To use it, invoke the processData() method with a raw data buffer, the
    callback function specified with the constructor is then invoked for each
    TS packet found. The callback function can be directly connected to
    PESPacketStream::processPacket().

    \author A.J. Admiraal
    \sa PESPacketStream
 */
class TSPacketStream : public Stream
{
public:
  typedef void (Stream::* TSCallback)(const TSPacket *);

public:
  inline                        TSPacketStream(Stream *stream, TSCallback tsCallback) : stream(stream),
                                                                                      tsCallback(tsCallback),
                                                                                      bufferUse(0) { }

  void                          processData(const quint8 *, size_t);

protected:
  off_t                         syncStream(const quint8 *, size_t);

private:
  Stream                * const stream;
  const TSCallback              tsCallback;

  quint8                        buffer[tsPacketSize * 256];
  size_t                        bufferUse;
};

/*! This support class decodes elementary-stream packets from either a raw data
    stream or a stream of TS packets. To use it, invoke the processData() method
    with a raw data buffer or the processPacket() method with a TS packet, the
    callback function specified with the constructor is then invoked for each
    PES packet found.

    \author A.J. Admiraal
 */
class PESPacketStream : public Stream
{
public:
  typedef void (Stream::* PESCallback)(const PESPacket *, size_t);
  typedef void (Stream::* ADTCallback)(const TSPacket::AdaptationField *);

public:
  inline                        PESPacketStream(Stream *stream, PESCallback pesCallback, ADTCallback adtCallback) : stream(stream),
                                                                                                                    pesCallback(pesCallback),
                                                                                                                    adtCallback(adtCallback),
                                                                                                                    bufferUse(0),
                                                                                                                    bufferPESLength(0) { }

  void                          reset(void);
  void                          processPacket(const TSPacket *);
  void                          processData(const quint8 *, size_t);

private:
  size_t                        getPacketLen(const quint8 *, size_t);

public:
  static const size_t           bufferSize = 262144;

private:
  Stream                * const stream;
  const PESCallback             pesCallback;
  const ADTCallback             adtCallback;

  quint8                        buffer[bufferSize];
  size_t                        bufferUse;
  size_t                        bufferPESLength;
};


} // End of namespace

} } // End of namespaces

#endif
