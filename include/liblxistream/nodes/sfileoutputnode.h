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

#ifndef LXSTREAM_SFILEOUTPUTNODE_H
#define LXSTREAM_SFILEOUTPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include "siooutputnode.h"
#include "../export.h"

namespace LXiStream {

class LXISTREAM_PUBLIC SFileOutputNode : public SIOOutputNode
{
Q_OBJECT
public:
  explicit                      SFileOutputNode(SGraph *, const QString &fileName);
  virtual                       ~SFileOutputNode();

  virtual bool                  start(STimer *);
  virtual void                  stop(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
