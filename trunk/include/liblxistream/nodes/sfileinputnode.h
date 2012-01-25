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

#ifndef LXSTREAM_SFILEINPUTNODE_H
#define LXSTREAM_SFILEINPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "sioinputnode.h"
#include "../export.h"

namespace LXiStream {

class SEncodedVideoBuffer;

class LXISTREAM_PUBLIC SFileInputNode : public SIOInputNode
{
Q_OBJECT
public:
  explicit                      SFileInputNode(SGraph *, const QUrl &filePath = QUrl());
  virtual                       ~SFileInputNode();

  void                          setFilePath(const QUrl &filePath);
  QUrl                          filePath(void) const;

  virtual bool                  setPosition(STime);

  virtual QList<DataStreamInfo> dataStreams(int title) const;
  virtual void                  selectStreams(int title, const QVector<StreamId> &);

public: // From SInterfaces::SourceNode
  virtual void                  stop(void);

private slots:
  void                          parseSubtitle(const SEncodedVideoBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
