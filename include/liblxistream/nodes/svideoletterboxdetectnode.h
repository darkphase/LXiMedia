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

#ifndef LXSTREAM_SVIDEOLETTERBOXDETECTNODE_H
#define LXSTREAM_SVIDEOLETTERBOXDETECTNODE_H

#include <QtCore>
#include "../sinterfaces.h"

namespace LXiStream {

class SVideoBuffer;

class SVideoLetterboxDetectNode : public QObject,
                                  public SInterfaces::Node
{
Q_OBJECT
Q_PROPERTY(unsigned delayFrames READ delayFrames WRITE setDelayFrames)
public:
  explicit                      SVideoLetterboxDetectNode(SGraph *);
  virtual                       ~SVideoLetterboxDetectNode();

  unsigned                      delayFrames(void) const;
  void                          setDelayFrames(unsigned);

public slots:
  void                          input(const SVideoBuffer &);

signals:
  void                          output(const SVideoBuffer &);

private:
  qreal                         determineAspectRatio(const SVideoBuffer &) const;

private:
  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
