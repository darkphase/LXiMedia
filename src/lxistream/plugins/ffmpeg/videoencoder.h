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

public: // From SBufferEncoder
  virtual bool                  openCodec(const SVideoCodec &, Flags = Flag_None);
  virtual SVideoCodec           codec(void) const;
  virtual SEncodedVideoBufferList encodeBuffer(const SVideoBuffer &);

private:
  static const int              bufferSize;

  SVideoCodec                   outCodec;
  AVCodec                     * codecHandle;
  AVCodecContext              * contextHandle;
  AVFrame                     * pictureHandle;
  SVideoBuffer                  pictureBuffer;

  SVideoFormatConvertNode       formatConvert;
  QList<STime>                  inputTimeStamps;
  quint32                       lastSubStreamId;

  bool                          fastEncode;
#ifdef OPT_RESEND_LAST_FRAME
  int                           lastInBufferId;
  SEncodedVideoBuffer           lastEncodedBuffer;
#endif

  SBuffer                       outBuffer;
};


} } // End of namespaces

#endif
