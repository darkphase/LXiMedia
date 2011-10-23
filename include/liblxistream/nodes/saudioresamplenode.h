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

#ifndef LXSTREAM_SAUDIORESAMPLENODE_H
#define LXSTREAM_SAUDIORESAMPLENODE_H

#include <QtCore>
#include <LXiCore>
#include "../saudiobuffer.h"
#include "../sinterfaces.h"
#include "../export.h"

namespace LXiStream {


class LXISTREAM_PUBLIC SAudioResampleNode : public SInterfaces::Node
{
Q_OBJECT
public:
  explicit                      SAudioResampleNode(SGraph *, const QString &algo = QString::null);
  virtual                       ~SAudioResampleNode();

  static QStringList            algorithms(void);

  unsigned                      sampleRate(void) const;
  void                          setSampleRate(unsigned);

public: // From SInterfaces::Node
  virtual bool                  start(void);
  virtual void                  stop(void);

public slots:
  void                          input(const SAudioBuffer &);
  void                          compensate(float);

signals:
  void                          output(const SAudioBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
