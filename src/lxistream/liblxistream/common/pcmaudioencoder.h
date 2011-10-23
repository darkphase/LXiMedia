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

#ifndef __PCMAUDIOENCODER_H
#define __PCMAUDIOENCODER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {


/*! This audio filter can be used to encode an audio stream. Currently it only
    supports the AC3 codec (Dolby Digital surrond) and is intended to be used in
    front of a digital audio output to encode surround channels into one SPDIF
    stream. If the input stream is already either AC3 or DCA (Digital Theatre
    Surround), this filter will simply passtrough the buffers.
 */
class PcmAudioEncoder : public SInterfaces::AudioEncoder
{
Q_OBJECT
public:
                                PcmAudioEncoder(const QString &, QObject *);
  virtual                       ~PcmAudioEncoder();

public: // From SInterfaces::AudioEncoder
  virtual bool                  openCodec(const SAudioCodec &, SInterfaces::BufferWriter *, Flags);
  virtual SAudioCodec           codec(void) const;
  virtual SEncodedAudioBufferList encodeBuffer(const SAudioBuffer &);

private:
  static SEncodedAudioBuffer    copyBuffer(const SAudioBuffer &, const QString &);
  static SEncodedAudioBuffer    swapBufferS16(const SAudioBuffer &, const QString &);
  static SEncodedAudioBuffer    encodeBufferS16U16(const SAudioBuffer &, const QString &);
  static SEncodedAudioBuffer    encodeSwapBufferS16U16(const SAudioBuffer &, const QString &);

private:
  SAudioCodec                   outCodec;
  SAudioFormatConvertNode       formatConvert;
};


} } // End of namespaces

#endif
