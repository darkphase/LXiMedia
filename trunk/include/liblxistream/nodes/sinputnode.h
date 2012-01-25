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

#ifndef LXISTREAM_SINPUTNODE_H
#define LXISTREAM_SINPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {

class SAudioDecoderNode;
class SVideoDecoderNode;
class SDataDecoderNode;

/*! This is a generic input node, reading from a QIODevice.
 */
class LXISTREAM_PUBLIC SInputNode : public SInterfaces::SourceNode,
                                    protected SInterfaces::AbstractBufferReader::ProduceCallback
{
Q_OBJECT
friend class SAudioDecoderNode;
friend class SVideoDecoderNode;
friend class SDataDecoderNode;
public:
  typedef SInterfaces::AbstractBufferReader::StreamId        StreamId;
  typedef SInterfaces::AbstractBufferReader::AudioStreamInfo AudioStreamInfo;
  typedef SInterfaces::AbstractBufferReader::VideoStreamInfo VideoStreamInfo;
  typedef SInterfaces::AbstractBufferReader::DataStreamInfo  DataStreamInfo;
  typedef SInterfaces::AbstractBufferReader::Chapter         Chapter;

public:
  explicit                      SInputNode(SGraph *, SInterfaces::AbstractBufferReader * = NULL);
  virtual                       ~SInputNode();

  void                          setBufferReader(SInterfaces::AbstractBufferReader *);
  const SInterfaces::AbstractBufferReader * bufferReader(void) const;
  SInterfaces::AbstractBufferReader * bufferReader(void);

  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual int                   numTitles(void) const;
  virtual QList<AudioStreamInfo> audioStreams(int title) const;
  virtual QList<VideoStreamInfo> videoStreams(int title) const;
  virtual QList<DataStreamInfo>  dataStreams(int title) const;
  virtual void                  selectStreams(int title, const QVector<StreamId> &);

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

signals:
  void                          output(const SEncodedAudioBuffer &);
  void                          output(const SEncodedVideoBuffer &);
  void                          output(const SEncodedDataBuffer &);
  void                          closeDecoder(void);

protected: // From SInterfaces::AbstractBufferReader::ProduceCallback
  virtual void                  produce(const SEncodedAudioBuffer &);
  virtual void                  produce(const SEncodedVideoBuffer &);
  virtual void                  produce(const SEncodedDataBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
