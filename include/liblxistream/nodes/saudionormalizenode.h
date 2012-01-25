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

#ifndef LXISTREAM_SAUDIONORMALIZENODE_H
#define LXISTREAM_SAUDIONORMALIZENODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {

/*! This audio filter can be used to normalize an audio stream.
 */
class LXISTREAM_PUBLIC SAudioNormalizeNode : public SInterfaces::Node
{
Q_OBJECT
public:
public:
                                SAudioNormalizeNode(SGraph *);
  virtual                       ~SAudioNormalizeNode();

public: // From SInterfaces::Node
  virtual bool                  start(void);
  virtual void                  stop(void);

public slots:
  void                          input(const SAudioBuffer &);

signals:
  void                          output(const SAudioBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
