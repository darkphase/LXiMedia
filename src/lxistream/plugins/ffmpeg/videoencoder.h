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

#ifndef __VIDEOENCODER_H
#define __VIDEOENCODER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {


class VideoEncoder : public SInterfaces::VideoEncoder
{
Q_OBJECT
private:
  typedef void (* PKtoPLfunc)(const void *, unsigned, size_t, quint8 *, quint8 *, quint8 *);

public:
                                VideoEncoder(const QString &, QObject *);
  virtual                       ~VideoEncoder();

  const ::AVCodecContext      * avCodecContext(void) const;

public: // From SBufferEncoder
  virtual bool                  openCodec(const SVideoCodec &, SInterfaces::BufferWriter *, Flags = Flag_None);
  virtual SVideoCodec           codec(void) const;
  virtual SEncodedVideoBufferList encodeBuffer(const SVideoBuffer &);

private:
  void                          encodeBufferTask(const SVideoBuffer &, SEncodedVideoBufferList *, bool);

private:
  SVideoCodec                   outCodec;
  ::AVCodec                   * codecHandle;
  ::AVCodecContext            * contextHandle;
  bool                          contextHandleOwner;

  ::AVFrame                   * pictureHandle;
  SVideoBuffer                  pictureBuffer;
  STime                         frameTime;

  SVideoFormatConvertNode       formatConvert;
  QList<STime>                  inputTimeStamps;
  quint32                       lastSubStreamId;

  QFuture<void>                 encodeFuture;
  SEncodedVideoBufferList       delayedResult;
  static const int              memorySemCount = 64;
  QSemaphore                    memorySem;

  bool                          fastEncode;
  bool                          noDelay;
  mutable bool                  enableWait;

  int                           bufferSize;
};


} } // End of namespaces

#endif
