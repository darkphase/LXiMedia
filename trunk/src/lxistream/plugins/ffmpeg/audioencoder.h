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

public: // From SBufferEncoder
  virtual bool                  openCodec(const SAudioCodec &, Flags = Flag_None);
  virtual SAudioCodec           codec(void) const;
  virtual SEncodedAudioBufferList encodeBuffer(const SAudioBuffer &);

private:
  inline int                    numSamples(size_t n)                            { return int((n / contextHandle->channels) / inSampleSize); }

private:
  SAudioCodec                   outCodec;
  AVCodec                     * codecHandle;
  AVCodecContext              * contextHandle;
  bool                          passThrough;

  unsigned                      inSampleSize;
  size_t                        inFrameSize;
  char                        * inFrameBufferRaw;
  char                        * inFrameBuffer;
  size_t                        inFrameBufferSize;

  SAudioFormatConvertNode       formatConvert;

  unsigned                      showErrors;
};


} } // End of namespaces

#endif
