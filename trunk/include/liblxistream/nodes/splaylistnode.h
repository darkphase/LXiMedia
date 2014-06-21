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
  explicit                      SPlaylistNode(SGraph *, const QList<QUrl> &files, SMediaInfo::ProbeInfo::FileType);
  virtual                       ~SPlaylistNode();

  void                          setFiles(const QList<QUrl> &files, SMediaInfo::ProbeInfo::FileType);

  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual QList<AudioStreamInfo> audioStreams(int title) const;
  virtual QList<VideoStreamInfo> videoStreams(int title) const;
  virtual QList<DataStreamInfo>  dataStreams(int title) const;
  virtual void                  selectStreams(int title, const QVector<StreamId> &);

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);

signals:
  void                          opened(const QUrl &);
  void                          closed(const QUrl &);

protected:
  virtual void                  endReached(void);

private:
  bool                          openNext(void);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
