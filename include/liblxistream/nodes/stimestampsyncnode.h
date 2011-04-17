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

#ifndef LXSTREAM_STIMESTAMPSYNCNODE_H
#define LXSTREAM_STIMESTAMPSYNCNODE_H

#include <QtCore>
#include <LXiCore>
#include "../saudiobuffer.h"
#include "../sgraph.h"
#include "../stime.h"
#include "../svideobuffer.h"
#include "../export.h"

namespace LXiStream {

class SAudioBuffer;
class SDataBuffer;
class SVideoBuffer;

class LXISTREAM_PUBLIC STimeStampSyncNode : public QObject,
                                            public SGraph::Node
{
Q_OBJECT
public:
  explicit                      STimeStampSyncNode(SGraph *);
  virtual                       ~STimeStampSyncNode();

  SInterval                     frameRate(void) const;
  void                          setFrameRate(SInterval);

  void                          setHeader(const SAudioBuffer &, const SVideoBufferList &, STime);

public slots:
  void                          input(const SAudioBuffer &);
  void                          input(const SVideoBuffer &);

signals:
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);

private:
  _lxi_internal void            output(void);

private:
  template <class _buffer> struct Queue;
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
