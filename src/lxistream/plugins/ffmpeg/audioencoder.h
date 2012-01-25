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

#ifndef __AUDIOENCODER_H
#define __AUDIOENCODER_H

#include <QtCore>
#include <LXiStream>

#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {


/*! This audio filter can be used to encode an audio stream. Currently it only
    supports the AC3 codec (Dolby Digital surrond) and is intended to be used in
    front of a digital audio output to encode surround channels into one SPDIF
    stream. If the input stream is already either AC3 or DCA (Digital Theatre
    Surround), this filter will simply passtrough the buffers.
 */
class AudioEncoder : public SInterfaces::AudioEncoder
{
Q_OBJECT
public:
                                AudioEncoder(const QString &, QObject *);
  virtual                       ~AudioEncoder();

  const ::AVCodecContext      * avCodecContext(void) const;

public: // From SBufferEncoder
  virtual bool                  openCodec(const SAudioCodec &, SInterfaces::BufferWriter *, Flags = Flag_None);
  virtual SAudioCodec           codec(void) const;
  virtual SEncodedAudioBufferList encodeBuffer(const SAudioBuffer &);

private:
  inline int                    numSamples(size_t n)                            { return int((n / contextHandle->channels) / inSampleSize); }
  void                          encodeBufferTask(const SAudioBuffer &, SEncodedAudioBufferList *, bool);

private:
  SAudioCodec                   outCodec;
  ::AVCodec                   * codecHandle;
  ::AVCodecContext            * contextHandle;
  bool                          contextHandleOwner;
  bool                          passThrough;

  unsigned                      inSampleSize;
  size_t                        inFrameSize;
  STime                         inFrameDuration;
  char                        * inFrameBufferRaw;
  char                        * inFrameBuffer;
  size_t                        inFrameBufferSize;

  SAudioFormatConvertNode       formatConvert;

  QFuture<void>                 encodeFuture;
  SEncodedAudioBufferList       delayedResult;
  static const int              memorySemCount = 64;
  QSemaphore                    memorySem;

  bool                          noDelay;
  mutable bool                  enableWait;

  unsigned                      showErrors;
};


} } // End of namespaces

#endif
