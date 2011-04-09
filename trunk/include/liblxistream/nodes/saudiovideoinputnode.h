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

#ifndef LXISTREAM_SAUDIOVIDEOINPUTNODE_H
#define LXISTREAM_SAUDIOVIDEOINPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "../saudiobuffer.h"
#include "../sgraph.h"
#include "../svideobuffer.h"
#include "../export.h"

namespace LXiStream {

/*! This is a generic audio/video input node that can be used to obtain audio
    and video data from a capture device such as a video capture card.
 */
class LXISTREAM_PUBLIC SAudioVideoInputNode : public QObject,
                                              public SGraph::SourceNode
{
Q_OBJECT
public:
  explicit                      SAudioVideoInputNode(SGraph *, const QString &device = QString::null);
  virtual                       ~SAudioVideoInputNode();

  static QStringList            devices(void);

  void                          setFormat(const SVideoFormat &);
  void                          setMaxBuffers(int);

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual void                  process(void);

signals:
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);

private slots:
  internal void                 produced(const SAudioBuffer &);
  internal void                 produced(const SVideoBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
