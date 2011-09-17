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

#ifndef __PCMAUDIODECODER_H
#define __PCMAUDIODECODER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {


/*! This audio filter can be used to decode an audio stream. If the output
    filterscan directly deal with the encoded input format, this filter will
    simply passtrough the buffers. This usually occurs when the output is an AC3
    or DCA capable SPDIF and the input is AC3 or DCA.
 */
class PcmAudioDecoder : public SInterfaces::AudioDecoder
{
Q_OBJECT
public:
                                PcmAudioDecoder(const QString &, QObject *);
  virtual                       ~PcmAudioDecoder();

public: // From SBufferDecoder
  virtual bool                  openCodec(const SAudioCodec &, Flags = Flag_None);
  virtual SAudioBufferList      decodeBuffer(const SEncodedAudioBuffer &);

private:
  SAudioFormat::Format          format;
  SAudioFormatConvertNode       formatConvert;
  STime                         nextTimeStamp;
};


} } // End of namespaces

#endif
