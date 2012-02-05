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

#ifndef __FORMATPROBER_H
#define __FORMATPROBER_H

#include <QtCore>
#include <LXiStream>
#include "ffmpegcommon.h"

namespace LXiStream {
namespace FFMpegBackend {

class BufferReader;

class FormatProber : public SInterfaces::FormatProber,
                     private SInterfaces::BufferReader::ProduceCallback
{
Q_OBJECT
public:
                                FormatProber(const QString &, QObject *);
  virtual                       ~FormatProber();

public: // From SInterfaces::FormatProber
  virtual void                  readFormat(ProbeInfo &, const QByteArray &);
  virtual void                  readContent(ProbeInfo &, QIODevice *);
  virtual SVideoBuffer          readThumbnail(const ProbeInfo &, QIODevice *, const QSize &);

private: // SInterfaces::BufferReader::ProduceCallback
  virtual void                  produce(const SEncodedAudioBuffer &);
  virtual void                  produce(const SEncodedVideoBuffer &);
  virtual void                  produce(const SEncodedDataBuffer &);

private:
  SVideoBuffer                  readThumbnail(const ProbeInfo &, BufferReader *, const QSize &);

  BufferReader                * createBufferReader(const QString &, QIODevice *, const QUrl &);

  static void                   setMetadata(ProbeInfo &, const BufferReader *);
  static void                   setMetadata(ProbeInfo &, const char *, const QString &);

private:
  static const int              minBufferCount = 25;
  static const int              maxBufferCount = 50;

  BufferReader                * bufferReader;
  QIODevice                   * lastIoDevice;

  SEncodedVideoBufferList       videoBuffers;
  bool                          waitForKeyFrame;
};

} } // End of namespaces

#endif
