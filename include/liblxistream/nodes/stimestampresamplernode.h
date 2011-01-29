/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef LXISTREAM_STIMESTAMPRESAMPLERNODE_H
#define LXISTREAM_STIMESTAMPRESAMPLERNODE_H

#include <QtCore>
#include "../sinterfaces.h"

namespace LXiStream {

class SAudioBuffer;
class SDataBuffer;
class SVideoBuffer;


/*! This filter can be used to resample the timestamps in a stream.
 */
class STimeStampResamplerNode : public QObject,
                                public SInterfaces::Node
{
Q_OBJECT
public:
  explicit                      STimeStampResamplerNode(SGraph *);
  virtual                       ~STimeStampResamplerNode();

  void                          setFrameRate(SInterval frameRate);

public slots:
  void                          input(const SAudioBuffer &);
  void                          input(const SVideoBuffer &);
  void                          input(const SSubtitleBuffer &);

signals:
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);
  void                          output(const SSubtitleBuffer &);

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif