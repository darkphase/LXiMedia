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

#ifndef __IMAGEENCODER_H
#define __IMAGEENCODER_H

#include <QtCore>
#include <LXiStream>
#include <LXiStreamGui>

namespace LXiStream {
namespace GuiBackend {


class ImageEncoder : public SInterfaces::VideoEncoder
{
Q_OBJECT
public:
                                ImageEncoder(const QString &, QObject *);
  virtual                       ~ImageEncoder();

public: // From SBufferEncoder
  virtual bool                  openCodec(const SVideoCodec &, Flags = Flag_None);
  virtual SVideoCodec           codec(void) const;
  virtual SEncodedVideoBufferList encodeBuffer(const SVideoBuffer &);

private:
  SVideoCodec                   outCodec;
  int                           quality;
};


} } // End of namespaces

#endif
