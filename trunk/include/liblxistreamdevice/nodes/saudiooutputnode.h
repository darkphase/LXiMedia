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

#ifndef LXISTREAMDEVICE_SAUDIOOUTPUTNODE_H
#define LXISTREAMDEVICE_SAUDIOOUTPUTNODE_H

#include <QtCore>
#include <LXiCore>
#include <LXiStream>
#include "../export.h"

namespace LXiStreamDevice {

/*! This is a generic decoder node wich decodes audio and video buffers.
 */
class LXISTREAMDEVICE_PUBLIC SAudioOutputNode : public ::LXiStream::SInterfaces::SinkNode
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
