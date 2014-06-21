/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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
  virtual bool                  openCodec(const SVideoCodec &, SInterfaces::BufferWriter *, Flags);
  virtual SVideoCodec           codec(void) const;
  virtual SEncodedVideoBufferList encodeBuffer(const SVideoBuffer &);

private:
  SVideoCodec                   outCodec;
  int                           quality;
};


} } // End of namespaces

#endif
