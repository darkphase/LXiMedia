/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef __BUFFERREADERBASE_H
#define __BUFFERREADERBASE_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {

class BufferReaderBase : public virtual SInterfaces::AbstractBufferReader
{
private:
  struct StreamContext
  {
    SAudioCodec                 audioCodec;
    SVideoCodec                 videoCodec;
    SDataCodec                  dataCodec;
    SInterval                   timeBase;
    bool                        dtsChecked;
    bool                        needsDTSFraming;

    STime                       firstTimeStamp;
    STime                       lastTimeStamp;
    STime                       timeStampGap;

    // For measuring the framerate.
    static const int            measurementSize = 48; // Needs to be a multiple of 6.
    QVector<STime>              measurement;

    // For DTS framing.
    static const size_t         dtsBufferSize = 19600 + AVCODEC_MAX_AUDIO_FRAME_SIZE;
    quint8                    * dtsBuffer;
    size_t                      dtsBufferUsed;
  };

public:
                                BufferReaderBase(void);
  virtual                       ~BufferReaderBase();

  inline const ::AVFormatContext * context(void) const                          { return formatContext; }
  bool                          setPosition(STime, bool fast);

  bool                          start(ProduceCallback *, ::AVFormatContext *);
  void                          stop(void);
  bool                          process(bool fast = false);

public: // From SInterfaces::AbstractBufferReader
  virtual STime                 duration(void) const;
  inline virtual bool           setPosition(STime pos)                          { return setPosition(pos, false); }
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual QList<AudioStreamInfo> audioStreams(void) const;
  virtual QList<VideoStreamInfo> videoStreams(void) const;
  virtual QList<DataStreamInfo> dataStreams(void) const;
  virtual void                  selectStreams(const QList<StreamId> &);

private: // DTS framing
  static bool                   isDTS(const uint8_t *, int);
  static int                    findDTSFrame(const uint8_t *, int);
  static unsigned               findDTSFrameType(const uint8_t *);
  static SEncodedAudioBufferList parseDTSFrames(StreamContext *, const uint8_t *, int);

private:
  static StreamContext        * initStreamContext(const ::AVStream *);
  static QString                readMetadata(::AVMetadata *, const char *tagName);
  QPair<STime, STime>           correctTimeStamp(const ::AVPacket &);
  QPair<STime, STime>           correctTimeStampToVideo(const AVPacket &);

private:
  static const STime            maxJumpTime;

  ProduceCallback             * produceCallback;
  ::AVFormatContext           * formatContext;

  StreamContext               * streamContext[MAX_STREAMS];
  QSet<StreamId>                selectedStreams;
  bool                          running;
  
  static const unsigned         maxBufferCount = StreamContext::measurementSize * 8;
  QList< QPair<int, SEncodedAudioBuffer> > audioBuffers;
  QList< QPair<int, SEncodedVideoBuffer> > videoBuffers;
  QList< QPair<int, SEncodedDataBuffer> >  dataBuffers;
};

} } // End of namespaces

#endif
