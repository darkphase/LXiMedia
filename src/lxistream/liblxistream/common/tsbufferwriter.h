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

#ifndef LXISTREAMCOMMON_TSBUFFERWRITER_H
#define LXISTREAMCOMMON_TSBUFFERWRITER_H

#include <QtCore>
#include <LXiStream>
#include "mpeg.h"

namespace LXiStream {
namespace Common {

class TsBufferWriter : public SInterfaces::BufferWriter,
                       protected SInterfaces::BufferWriter::WriteCallback
{
Q_OBJECT
public:
  explicit                      TsBufferWriter(const QString &, QObject *);
  virtual                       ~TsBufferWriter();

public: // From SInterfaces::BufferWriter
  virtual bool                  openFormat(const QString &);
  virtual bool                  createStreams(const QList<SAudioCodec> &, const QList<SVideoCodec> &, STime);

  virtual bool                  start(SInterfaces::BufferWriter::WriteCallback *);
  virtual void                  stop(void);
  virtual void                  process(const SEncodedAudioBuffer &);
  virtual void                  process(const SEncodedVideoBuffer &);
  virtual void                  process(const SEncodedDataBuffer &);

protected:
  virtual void                  write(const uchar *, qint64);

private:
  SInterfaces::BufferWriter   * bufferWriter;
  SInterfaces::BufferWriter::WriteCallback * callback;
};


} } // End of namespaces

#endif
