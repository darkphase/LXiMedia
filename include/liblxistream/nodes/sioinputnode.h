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

#ifndef LXISTREAM_SIOINPUTNODE_H
#define LXISTREAM_SIOINPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "sinputnode.h"
#include "../export.h"

namespace LXiStream {

/*! This is a generic input node, reading from a QIODevice.
 */
class LXISTREAM_PUBLIC SIOInputNode : public SInputNode
{
Q_OBJECT
public:
  explicit                      SIOInputNode(SGraph *, QIODevice * = NULL);
  virtual                       ~SIOInputNode();

  void                          setIODevice(QIODevice *);
  const QIODevice             * ioDevice(void) const;
  QIODevice                   * ioDevice(void);

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

signals:
  void                          finished(void);

protected:
  virtual void                  endReached(void);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
