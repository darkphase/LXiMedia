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

#ifndef __RAWSUBTITLEDECODER_H
#define __RAWSUBTITLEDECODER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {


class RawSubtitleDecoder : public SInterfaces::DataDecoder
{
Q_OBJECT
public:
                                RawSubtitleDecoder(const QString &, QObject *);
  virtual                       ~RawSubtitleDecoder();

public: // From SBufferDecoder
  virtual bool                  openCodec(const SDataCodec &, SInterfaces::AbstractBufferReader *, Flags = Flag_None);
  virtual SDataBufferList       decodeBuffer(const SEncodedDataBuffer &);

private:
  static SSubtitleBuffer        decodeUtf8(const SEncodedDataBuffer &);
  static SSubtitleBuffer        decodeLocal8Bit(const SEncodedDataBuffer &);

private:
  SSubtitleBuffer               (* decode)(const SEncodedDataBuffer &);
};


} } // End of namespaces

#endif
