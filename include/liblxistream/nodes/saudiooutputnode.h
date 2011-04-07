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

#ifndef LXISTREAM_SAUDIOOUTPUTNODE_H
#define LXISTREAM_SAUDIOOUTPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "../saudiobuffer.h"
#include "../sgraph.h"

namespace LXiStream {

/*! This is a generic decoder node wich decodes audio and video buffers.
 */
class S_DSO_PUBLIC SAudioOutputNode : public QObject,
                                      public SGraph::SinkNode
{
Q_OBJECT
public:
  explicit                      SAudioOutputNode(SGraph *, const QString &device = QString::null);
  virtual                       ~SAudioOutputNode();

  static QStringList            devices(void);

  void                          setDelay(STime);
  STime                         delay(void) const;

  virtual bool                  start(STimer *);
  virtual void                  stop(void);

public slots:
  void                          input(const SAudioBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
