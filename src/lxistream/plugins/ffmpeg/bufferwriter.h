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

#ifndef __BUFFERWRITER_H
#define __BUFFERWRITER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {


class BufferWriter : public SInterfaces::BufferWriter
{
Q_OBJECT
public:
  explicit                      BufferWriter(const QString &, QObject *);
  virtual                       ~BufferWriter();

public: // From SInterfaces::BufferWriter
  virtual bool                  openFormat(const QString &);
  virtual bool                  createStreams(const QList<SAudioCodec> &, const QList<SVideoCodec> &, STime);

  virtual bool                  start(WriteCallback *);
  virtual void                  stop(void);
  virtual void                  process(const SEncodedAudioBuffer &);
  virtual void                  process(const SEncodedVideoBuffer &);
  virtual void                  process(const SEncodedDataBuffer &);

private:
  static int                    write(void *opaque, uint8_t *buf, int buf_size);

private:
  static const quint32          audioStreamId = 0x00010000;
  static const quint32          videoStreamId = 0x00020000;

  WriteCallback               * callback;
  ::AVOutputFormat            * format;
  ::AVFormatContext           * formatContext;
  ::ByteIOContext             * ioContext;
  QMap<quint32, ::AVStream *>   streams;

  bool                          hasAudio, hasVideo;
};


} } // End of namespaces

#endif
