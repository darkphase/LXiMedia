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

#ifndef LXISTREAM_SDISCINPUTNODE_H
#define LXISTREAM_SDISCINPUTNODE_H

#include <QtCore>
#include "../sinterfaces.h"

namespace LXiStream {

/*! This is a generic intput node, reading to a QIODevice.
 */
class SDiscInputNode : public QObject,
                       public SInterfaces::SourceNode,
                       public SInterfaces::BufferReaderNode,
                       protected SInterfaces::BufferReader::ProduceCallback
{
Q_OBJECT
public:
  typedef SInterfaces::BufferReader::StreamId        StreamId;
  typedef SInterfaces::BufferReader::AudioStreamInfo AudioStreamInfo;
  typedef SInterfaces::BufferReader::VideoStreamInfo VideoStreamInfo;
  typedef SInterfaces::BufferReader::DataStreamInfo  DataStreamInfo;

public:
  explicit                      SDiscInputNode(SGraph *, const QString &path);
  virtual                       ~SDiscInputNode();

  unsigned                      numTitles(void) const;
  bool                          playTitle(unsigned title);

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
  virtual QList<DataStreamInfo> dataStreams(void) const;
  virtual void                  selectStreams(const QList<StreamId> &);

signals:
  void                          output(const SEncodedAudioBuffer &);
  void                          output(const SEncodedVideoBuffer &);
  void                          output(const SEncodedDataBuffer &);
  void                          finished(void);

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
