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
#include "../sgraph.h"
#include "../sinterfaces.h"

namespace LXiStream {

/*! This is a generic output node, writing to a QIODevice.
 */
class SIOOutputNode : public QObject,
                      public SGraph::SinkNode,
                      protected SInterfaces::BufferWriter::WriteCallback
{
Q_OBJECT
public:
  explicit                      SIOOutputNode(SGraph *, QIODevice * = NULL);
  virtual                       ~SIOOutputNode();

  void                          setIODevice(QIODevice *);
  bool                          openFormat(const QString &, const SAudioCodec &, STime);
  bool                          openFormat(const QString &, const SAudioCodec &, const SVideoCodec &, STime);
  bool                          openFormat(const QString &, const QList<SAudioCodec> &, const QList<SVideoCodec> &, STime);

  virtual bool                  start(STimer *);
  virtual void                  stop(void);

public slots:
  void                          input(const SEncodedAudioBuffer &);
  void                          input(const SEncodedVideoBuffer &);
  void                          input(const SEncodedDataBuffer &);

protected: // From SInterfaces::BufferReader::WriteCallback
  virtual void                  write(const uchar *, qint64);

public:
  static const int              outBufferSize;
  static const int              outBufferDelay;

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif