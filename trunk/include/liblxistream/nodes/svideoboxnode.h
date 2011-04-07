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

#ifndef LXISTREAM_SVIDEOBOXNODE_H
#define LXISTREAM_SVIDEOBOXNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sgraph.h"
#include "../ssize.h"
#include "../svideobuffer.h"

namespace LXiStream {

class SVideoBuffer;

class S_DSO_PUBLIC SVideoBoxNode : public QObject,
                                   public SGraph::Node
{
Q_OBJECT
Q_PROPERTY(QSize size READ __internal_size WRITE __internal_setSize)
Q_PROPERTY(float aspectRatio READ __internal_aspectRatio WRITE __internal_setAspectRatio)
public:
  explicit                      SVideoBoxNode(SGraph *);
  virtual                       ~SVideoBoxNode();

  SSize                         size(void) const;
  void                          setSize(const SSize &s);

public slots:
  void                          input(const SVideoBuffer &);

signals:
  void                          output(const SVideoBuffer &);

private:
  inline QSize                  __internal_size(void) const                     { return size().size(); }
  inline void                   __internal_setSize(const QSize &s)              { setSize(s); }
  inline float                  __internal_aspectRatio(void) const              { return size().aspectRatio(); }
  inline void                   __internal_setAspectRatio(float a)              { SSize t = size(); t.setAspectRatio(a); setSize(t); }

private:
  void                          processTask(const SVideoBuffer &);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
