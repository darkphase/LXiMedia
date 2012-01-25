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

#ifndef __AUDIODECODER_H
#define __AUDIODECODER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {


/*! This audio filter can be used to decode an audio stream. If the output
    filterscan directly deal with the encoded input format, this filter will
    simply passtrough the buffers. This usually occurs when the output is an AC3
    or DCA capable SPDIF and the input is AC3 or DCA.
 */
class AudioDecoder : public SInterfaces::AudioDecoder
{
Q_OBJECT
private:
  typedef void (* PostFilterFunc)(qint16 *, const qint16 *, size_t, unsigned);

public:
                                AudioDecoder(const QString &, QObject *);
  virtual                       ~AudioDecoder();

public: // From SBufferDecoder
  virtual bool                  openCodec(const SAudioCodec &, SInterfaces::AbstractBufferReader *, Flags = Flag_None);
  virtual SAudioBufferList      decodeBuffer(const SEncodedAudioBuffer &);

private:
  SAudioCodec                   inCodec;
  AVCodec                     * codecHandle;
  AVCodecContext              * contextHandle;
  bool                          contextHandleOwner;
  PostFilterFunc                postFilter;
  bool                          passThrough;

  static const int              defaultBufferLen = 40; // msec
  STime                         timeStamp;
};


} } // End of namespaces

#endif
