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
class LXISTREAM_PUBLIC SIOOutputNode : public SInterfaces::SinkNode
{
Q_OBJECT
friend class SAudioEncoderNode;
friend class SVideoEncoderNode;
public:
  explicit                      SIOOutputNode(SGraph *, QIODevice * = NULL);
  virtual                       ~SIOOutputNode();

  static QStringList            formats(void);

  void                          setIODevice(QIODevice *);
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

private:
  SInterfaces::BufferWriter   * bufferWriter(void);
  void                          blockUntil(STime);

private slots:
  void                          close(void);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
