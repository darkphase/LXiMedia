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

#ifndef LXSTREAM_SVIDEORESIZENODE_H
#define LXSTREAM_SVIDEORESIZENODE_H

#include <QtCore>
#include <LXiCore>
#include "../sgraph.h"
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {

class LXISTREAM_PUBLIC SVideoResizeNode : public QObject,
                                          public SGraph::Node
{
Q_OBJECT
public:
  explicit                      SVideoResizeNode(SGraph *, const QString &algo = QString::null);
  virtual                       ~SVideoResizeNode();

  static QStringList            algorithms(void);

  void                          setHighQuality(bool);
  bool                          highQuality(void) const;
  void                          setSize(const SSize &size);
  SSize                         size(void) const;
  void                          setAspectRatioMode(Qt::AspectRatioMode);
  Qt::AspectRatioMode           aspectRatioMode(void) const;

public slots:
  void                          input(const SVideoBuffer &);

signals:
  void                          output(const SVideoBuffer &);

private:
  _lxi_internal void            processTask(const SVideoBuffer &, SInterfaces::VideoResizer *);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
