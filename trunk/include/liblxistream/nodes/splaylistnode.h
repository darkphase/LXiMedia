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
#include "sfileinputnode.h"
#include "../smediainfo.h"
#include "../export.h"

namespace LXiStream {

class SFileInputNode;

class LXISTREAM_PUBLIC SPlaylistNode : public SFileInputNode
{
Q_OBJECT
public:
  explicit                      SPlaylistNode(SGraph *, const SMediaInfoList &files);
  virtual                       ~SPlaylistNode();

  virtual bool                  open(quint16 programId = 0);

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);

public: // From SInterfaces::AbstractBufferReader
  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual QList<AudioStreamInfo> audioStreams(void) const;
  virtual QList<VideoStreamInfo> videoStreams(void) const;
  virtual QList<DataStreamInfo>  dataStreams(void) const;
  virtual void                  selectStreams(const QVector<StreamId> &);

signals:
  void                          opened(const QString &, quint16);
  void                          closed(const QString &, quint16);

protected:
  virtual void                  endReached(void);

private:
  _lxi_internal bool            openNext(void);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
