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

#ifndef LXISTREAM_SNETWORKINPUTNODE_H
#define LXISTREAM_SNETWORKINPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {

class SEncodedVideoBuffer;

class LXISTREAM_PUBLIC SNetworkInputNode : public SInterfaces::SourceNode,
                                           public SInterfaces::AbstractBufferReader,
                                           protected SInterfaces::BufferReader::ProduceCallback
{
Q_OBJECT
public:
  explicit                      SNetworkInputNode(SGraph *, const QUrl &url);
  virtual                       ~SNetworkInputNode();

  void                          setBufferDuration(const STime &);
  STime                         bufferDuration(void) const;

  virtual bool                  open(quint16 programId = 0);

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual void                  process(void);

public: // From SInterfaces::AbstractBufferReader
  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual QList<AudioStreamInfo> audioStreams(void) const;
  virtual QList<VideoStreamInfo> videoStreams(void) const;
  virtual QList<DataStreamInfo>  dataStreams(void) const;
  virtual void                  selectStreams(const QVector<StreamId> &);

public slots:
  void                          fillBuffer(void);

signals:
  void                          bufferState(bool, float);
  void                          output(const SEncodedAudioBuffer &);
  void                          output(const SEncodedVideoBuffer &);
  void                          output(const SEncodedDataBuffer &);
  void                          finished(void);

protected: // From SInterfaces::BufferReader::ProduceCallback
  virtual void                  produce(const SEncodedAudioBuffer &);
  virtual void                  produce(const SEncodedVideoBuffer &);
  virtual void                  produce(const SEncodedDataBuffer &);

private:
  _lxi_internal void            bufferTask(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
