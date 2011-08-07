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
  virtual bool                  openCodec(const SVideoCodec &, Flags = Flag_None);
  virtual SVideoBufferList      decodeBuffer(const SEncodedVideoBuffer &);

private:
  SVideoCodec                   inCodec;
  AVCodec                     * codecHandle;
  AVCodecContext              * contextHandle;
  AVFrame                     * pictureHandle;

  SVideoFormat                  outFormat;
  int                           outNumLines[3];

  quint32                       lastSubStreamId;
  STime                         timeStamp;
  QMap<STime, SVideoBuffer>     reorderBuffer;
};


} } // End of namespaces

#endif
