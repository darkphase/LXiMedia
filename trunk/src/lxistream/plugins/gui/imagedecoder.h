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

#ifndef __IMAGEDECODER_H
#define __IMAGEDECODER_H

#include <QtCore>
#include <LXiStream>
#include <LXiStreamGui>

namespace LXiStream {
namespace GuiBackend {


class ImageDecoder : public SInterfaces::VideoDecoder
{
Q_OBJECT
public:
                                ImageDecoder(const QString &, QObject *);
  virtual                       ~ImageDecoder();

  static SVideoBuffer           decodeImage(const QByteArray &);

public: // From SBufferDecoder
  virtual bool                  openCodec(const SVideoCodec &, SInterfaces::AbstractBufferReader *, Flags);
  virtual SVideoBufferList      decodeBuffer(const SEncodedVideoBuffer &);
};


} } // End of namespaces

#endif
