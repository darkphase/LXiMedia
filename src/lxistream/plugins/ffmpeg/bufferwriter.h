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

  inline const ::AVOutputFormat * avFormat(void) const                          { return format; }
  ::AVStream                  * createStream(AVCodec *codec);

public: // From SInterfaces::BufferWriter
  virtual bool                  openFormat(const QString &);
  virtual bool                  addStream(const SInterfaces::AudioEncoder *, STime);
  virtual bool                  addStream(const SInterfaces::VideoEncoder *, STime);

  virtual bool                  start(QIODevice *);
  virtual void                  stop(void);
  virtual void                  process(const SEncodedAudioBuffer &);
  virtual void                  process(const SEncodedVideoBuffer &);
  virtual void                  process(const SEncodedDataBuffer &);

private:
  void                          clear(void);

  static int                    write(void *opaque, uint8_t *buf, int buf_size);
  static int64_t                seek(void *opaque, int64_t offset, int whence);

private:
  QIODevice                   * ioDevice;
  ::AVOutputFormat            * format;
  ::AVFormatContext           * formatContext;
  ::AVIOContext               * ioContext;
  QMap<int, ::AVStream *>       streams;

  bool                          hasAudio, hasVideo;
  bool                          mpegClock, mpegTs;
};


} } // End of namespaces

#endif
