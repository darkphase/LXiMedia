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

#ifndef LXSTREAM_SGRAPH_H
#define LXSTREAM_SGRAPH_H

#include <QtCore>
#include <LXiCore>
#include "export.h"

namespace LXiStream {

class STimer;

namespace SInterfaces {
  class Node;
  class SourceNode;
  class SinkNode;
}

class LXISTREAM_PUBLIC SGraph : public QThread
{
Q_OBJECT
public:
  explicit                      SGraph(void);
  virtual                       ~SGraph();

  static bool                   connect(const QObject *, const char *, const QObject *, const char *);
  bool                          connect(const QObject *, const char *, const char *) const;

  bool                          isRunning(void) const;

  void                          addNode(SInterfaces::Node *);
  void                          addNode(SInterfaces::SourceNode *);
  void                          addNode(SInterfaces::SinkNode *);

public slots:
  virtual bool                  start(void);
  virtual void                  stop(void);

protected: // From QThread and QObject
  virtual void                  run(void);
  virtual void                  customEvent(QEvent *);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
