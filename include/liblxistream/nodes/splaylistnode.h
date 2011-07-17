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

#ifndef LXISTREAM_SPLAYLISTNODE_H
#define LXISTREAM_SPLAYLISTNODE_H

#include <QtCore>
#include <LXiCore>
#include "sioinputnode.h"
#include "../smediainfo.h"
#include "../export.h"

namespace LXiStream {

class SFileInputNode;

class LXISTREAM_PUBLIC SPlaylistNode : public SInterfaces::SourceNode,
                                       public SInterfaces::BufferReaderNode
{
Q_OBJECT
public:
  explicit                      SPlaylistNode(SGraph *, const SMediaInfoList &files);
  virtual                       ~SPlaylistNode();

  virtual bool                  open(void);

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
  void                          opened(const QString &, quint16);
  void                          closed(const QString &, quint16);

private:
  _lxi_internal SFileInputNode * openFile(const QString &, quint16);
  _lxi_internal void            openNext(void);

private slots:
  _lxi_internal void            closeFile(void);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
