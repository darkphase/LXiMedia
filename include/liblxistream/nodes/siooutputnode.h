/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
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

#ifndef LXSTREAM_SOUTPUTNODE_H
#define LXSTREAM_SOUTPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {

class SAudioEncoderNode;
class SVideoEncoderNode;

/*! This is a generic output node, writing to a QIODevice.
 */
class LXISTREAM_PUBLIC SIOOutputNode : public SInterfaces::SinkNode,
                                       protected SInterfaces::BufferWriter::WriteCallback
{
Q_OBJECT
friend class SAudioEncoderNode;
friend class SVideoEncoderNode;
public:
  explicit                      SIOOutputNode(SGraph *, QIODevice * = NULL);
  virtual                       ~SIOOutputNode();

  static QStringList            formats(void);

  void                          setIODevice(QIODevice *, bool autoClose = false);
  bool                          openFormat(const QString &);
  bool                          hasIODevice(void) const;

  bool                          openFormat(const QString &, const SAudioCodec &, STime);
  bool                          openFormat(const QString &, const SAudioCodec &, const SVideoCodec &, STime);
  bool                          openFormat(const QString &, const QList<SAudioCodec> &, const QList<SVideoCodec> &, STime);

  void                          enablePseudoStreaming(float speed, STime preload = STime::fromSec(10));

  virtual bool                  start(STimer *);
  virtual void                  stop(void);

public slots:
  void                          input(const SEncodedAudioBuffer &);
  void                          input(const SEncodedVideoBuffer &);
  void                          input(const SEncodedDataBuffer &);

signals:
  void                          closed(void);

protected: // From SInterfaces::BufferReader::WriteCallback
  virtual void                  write(const uchar *, qint64);
  virtual qint64                seek(qint64, int);

private:
  _lxi_internal SInterfaces::BufferWriter * bufferWriter(void);
  _lxi_internal void            blockUntil(STime);

private slots:
  _lxi_internal void            close(void);

public:
  static const int              outBufferSize;

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
