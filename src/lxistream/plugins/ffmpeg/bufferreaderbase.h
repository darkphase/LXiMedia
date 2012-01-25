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

#ifndef __BUFFERREADERBASE_H
#define __BUFFERREADERBASE_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {

class BufferReaderBase
{
public:
  class Packet : public SBuffer
  {
  public:
                                Packet(void);
                                Packet(const ::AVPacket &from);

  public:
    qint64                      pts, dts;
    qint64                      pos;
    qint64                      convergenceDuration;
    int                         streamIndex;
    int                         flags;
    int                         duration;
  };

private:
  struct StreamContext
  {
    SAudioCodec                 audioCodec;
    SVideoCodec                 videoCodec;
    SDataCodec                  dataCodec;
    SInterval                   timeBase;
    bool                        dtsChecked;
    bool                        needsDTSFraming;

    STime                       lastTimeStamp;
    STime                       timeStampGap;

    // For measuring the framerate.
    static const int            measurementSize; // Needs to be a multiple of 6.
    QVector<STime>              measurement;

    // For DTS framing.
    static const size_t         dtsBufferSize = 19600 + AVCODEC_MAX_AUDIO_FRAME_SIZE;
    quint8                    * dtsBuffer;
    size_t                      dtsBufferUsed;
  };

public:
                                BufferReaderBase(void);
                                ~BufferReaderBase();

  inline const ::AVFormatContext * context(void) const                          { return formatContext; }
  
  ::AVStream                  * getStream(int index) const;

  bool                          start(SInterfaces::AbstractBufferReader::ProduceCallback *, ::AVFormatContext *, bool fast);
  void                          stop(void);

  QString                       formatName(void) const;

  Packet                        read(void);
  STime                         timeStamp(const Packet &) const;
  bool                          demux(const Packet &);

public: // From SInterfaces::AbstractBufferedReader
  bool                          buffer(void);
  STime                         bufferDuration(void) const;

  STime                         duration(void) const;
  bool                          setPosition(STime);
  STime                         position(void) const;
  QList<SInterfaces::AbstractBufferReader::Chapter> chapters(void) const;

  QList<SInterfaces::AbstractBufferReader::AudioStreamInfo> audioStreams(void) const;
  QList<SInterfaces::AbstractBufferReader::VideoStreamInfo> videoStreams(void) const;
  QList<SInterfaces::AbstractBufferReader::DataStreamInfo> dataStreams(void) const;
  void                          selectStreams(const QVector<SInterfaces::AbstractBufferReader::StreamId> &);

private: // DTS framing
  static bool                   isDTS(const SBuffer &);
  static int                    findDTSFrame(const quint8 *, int);
  static unsigned               findDTSFrameType(const quint8 *);
  static SEncodedAudioBufferList parseDTSFrames(StreamContext *, const SBuffer &);

private:
  void                          clear(void);
  static StreamContext        * initStreamContext(const ::AVStream *);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 0, 0)
  static QString                readMetadata(::AVDictionary *, const char *tagName);
#else
  static QString                readMetadata(::AVMetadata *, const char *tagName);
#endif
  bool                          isSelected(const ::AVStream *) const;
  QPair<STime, STime>           correctTimeStamp(const Packet &);
  QPair<STime, STime>           correctTimeStampToVideo(const Packet &);
  QPair<STime, STime>           correctTimeStampToVideoOnly(const Packet &) const;

private:
  static const STime            maxJumpTime;

  SInterfaces::AbstractBufferReader::ProduceCallback * produceCallback;
  ::AVFormatContext           * formatContext;


  QVector<StreamContext *>      streamContext;
  QVector<SInterfaces::AbstractBufferReader::StreamId> selectedStreams;

  static const int              maxBufferCount;
  QList<Packet>                 packetBuffer;
};

} } // End of namespaces

#endif
