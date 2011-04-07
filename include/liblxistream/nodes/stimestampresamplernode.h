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
#include <LXiCore>
#include "../saudiobuffer.h"
#include "../sgraph.h"
#include "../spixels.h"
#include "../ssubpicturebuffer.h"
#include "../ssubtitlebuffer.h"
#include "../svideobuffer.h"

namespace LXiStream {

class SAudioBuffer;
class SDataBuffer;
class SVideoBuffer;

/*! This filter can be used to resample the timestamps in a stream.
 */
class S_DSO_PUBLIC STimeStampResamplerNode : public QObject,
                                             public SGraph::Node
{
Q_OBJECT
public:
  explicit                      STimeStampResamplerNode(SGraph *);
  virtual                       ~STimeStampResamplerNode();

  void                          setFrameRate(SInterval frameRate, double maxRatio = 0.08);

public slots:
  void                          input(const SAudioBuffer &);
  void                          input(const SVideoBuffer &);
  void                          input(const SSubpictureBuffer &);
  void                          input(const SSubtitleBuffer &);

signals:
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);
  void                          output(const SSubpictureBuffer &);
  void                          output(const SSubtitleBuffer &);

private:
  STime                         correct(const STime &);

private:
  static const int              numChannels = 4;

  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
