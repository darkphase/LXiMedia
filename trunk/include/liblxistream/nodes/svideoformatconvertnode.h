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

#ifndef LXISTREAM_SVIDEOFORMATCONVERTNODE_H
#define LXISTREAM_SVIDEOFORMATCONVERTNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sinterfaces.h"
#include "../svideobuffer.h"
#include "../export.h"

namespace LXiStream {

class SVideoBuffer;

class LXISTREAM_PUBLIC SVideoFormatConvertNode : public SInterfaces::Node
{
Q_OBJECT
public:
  explicit                      SVideoFormatConvertNode(SGraph *);
  virtual                       ~SVideoFormatConvertNode();

  void                          setDestFormat(SVideoFormat::Format);
  SVideoFormat::Format          destFormat(void) const;

  SVideoBuffer                  convert(const SVideoBuffer &);
  static SVideoBuffer           convert(const SVideoBuffer &, SVideoFormat::Format);

public: // From SInterfaces::Node
  virtual bool                  start(void);
  virtual void                  stop(void);

public slots:
  void                          input(const SVideoBuffer &);

signals:
  void                          output(const SVideoBuffer &);

private:
  _lxi_internal void            processTask(const SVideoBuffer &);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
