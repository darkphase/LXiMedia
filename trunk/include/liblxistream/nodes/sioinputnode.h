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

#ifndef LXISTREAM_SIOINPUTNODE_H
#define LXISTREAM_SIOINPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {

/*! This is a generic input node, reading to a QIODevice.
 */
class LXISTREAM_PUBLIC SIOInputNode : public SInterfaces::SourceNode,
                                      public SInterfaces::AbstractBufferReader,
                                      protected SInterfaces::BufferReader::ReadCallback,
                                      protected SInterfaces::BufferReader::ProduceCallback
{
Q_OBJECT
public:
  explicit                      SIOInputNode(SGraph *, QIODevice * = NULL, const QString &path = QString::null);
  virtual                       ~SIOInputNode();

  void                          setIODevice(QIODevice *);
  virtual bool                  open(quint16 programId = 0);

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual void                  process(void);

public: // From SInterfaces::BufferReaderNode
  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual QList<AudioStreamInfo> audioStreams(void) const;
  virtual QList<VideoStreamInfo> videoStreams(void) const;
  virtual QList<DataStreamInfo>  dataStreams(void) const;
  virtual void                  selectStreams(const QList<StreamId> &);

signals:
  void                          output(const SEncodedAudioBuffer &);
  void                          output(const SEncodedVideoBuffer &);
  void                          output(const SEncodedDataBuffer &);
  void                          finished(void);

protected: // From SInterfaces::BufferReader::ReadCallback
  virtual qint64                read(uchar *, qint64);
  virtual qint64                seek(qint64, int);

protected: // From SInterfaces::BufferReader::ProduceCallback
  virtual void                  produce(const SEncodedAudioBuffer &);
  virtual void                  produce(const SEncodedVideoBuffer &);
  virtual void                  produce(const SEncodedDataBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
