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

#ifndef __BUFFERREADER_H
#define __BUFFERREADER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {


class BufferReader : public SInterfaces::BufferReader
{
Q_OBJECT
private:
  struct StreamContext
  {
    SAudioCodec                 audioCodec;
    SVideoCodec                 videoCodec;
    SDataCodec                  dataCodec;
    SInterval                   timeBase;
    bool                        selected;
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
  explicit                      BufferReader(const QString &, QObject *);
  virtual                       ~BufferReader();

  inline const ::AVFormatContext * context(void) const                          { return formatContext; }

public: // From SInterfaces::BufferReader
  virtual bool                  openFormat(const QString &);

  virtual bool                  start(Callback *, bool streamed);
  virtual void                  stop(void);
  virtual bool                  process(void);

  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;

  virtual QList<AudioStreamInfo> audioStreams(void) const;
  virtual QList<VideoStreamInfo> videoStreams(void) const;
  virtual QList<DataStreamInfo>  dataStreams(void) const;
  virtual void                  selectStreams(const QList<quint16> &);

private: // DTS framing
  static bool                   isDTS(const uint8_t *, int);
  static int                    findDTSFrame(const uint8_t *, int);
  static unsigned               findDTSFrameType(const uint8_t *);
  static SEncodedAudioBufferList parseDTSFrames(StreamContext *, const uint8_t *, int);

private:
  QPair<STime, STime>           correctTimeStamp(const AVPacket &);
  QPair<STime, STime>           correctTimeStampToVideo(const AVPacket &);
  static int                    read(void *opaque, uint8_t *buf, int buf_size);
  static int64_t                seek(void *opaque, int64_t offset, int whence);

private:
  static const STime            maxJumpTime;

  Callback                    * callback;
  ::AVInputFormat             * format;
  ::AVFormatContext           * formatContext;
  ::ByteIOContext             * ioContext;

  StreamContext               * streamContext[MAX_STREAMS];
  bool                          running;
  static const unsigned         maxBufferCount = StreamContext::measurementSize * 8;
  QList< QPair<StreamContext *, SEncodedAudioBuffer> > audioBuffers;
  QList< QPair<StreamContext *, SEncodedVideoBuffer> > videoBuffers;
  QList< QPair<StreamContext *, SEncodedDataBuffer> >  dataBuffers;
};


} } // End of namespaces

#endif
