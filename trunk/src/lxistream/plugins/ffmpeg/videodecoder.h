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

#ifndef __VIDEODECODER_H
#define __VIDEODECODER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {


class VideoDecoder : public SInterfaces::VideoDecoder
{
Q_OBJECT
private:
  typedef int (VideoDecoder::* ConvertFunc)(SVideoBuffer &);

public:
                                VideoDecoder(const QString &, QObject *);
  virtual                       ~VideoDecoder();

public: // From SBufferDecoder
  virtual bool                  openCodec(const SVideoCodec &, SInterfaces::AbstractBufferReader *, Flags = Flag_None);
  virtual SVideoBufferList      decodeBuffer(const SEncodedVideoBuffer &);

private:
  SVideoCodec                   inCodec;
  ::AVDictionary              * codecDict;
  ::AVCodec                   * codecHandle;
  ::AVCodecContext            * contextHandle;
  bool                          contextHandleOwner;
  ::AVFrame                   * pictureHandle;

  SVideoFormat                  outFormat;
  int                           outNumLines[3];

  quint32                       lastSubStreamId;
  STime                         timeStamp;
  QMap<STime, SVideoBuffer>     reorderBuffer;
};


} } // End of namespaces

#endif
