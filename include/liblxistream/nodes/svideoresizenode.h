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

#ifndef LXSTREAM_SVIDEORESIZENODE_H
#define LXSTREAM_SVIDEORESIZENODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {

class LXISTREAM_PUBLIC SVideoResizeNode : public SInterfaces::Node
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

public: // From SInterfaces::Node
  virtual bool                  start(void);
  virtual void                  stop(void);

public slots:
  void                          input(const SVideoBuffer &);

signals:
  void                          output(const SVideoBuffer &);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
