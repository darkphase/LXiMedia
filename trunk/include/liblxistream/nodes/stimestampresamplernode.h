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

#ifndef LXISTREAM_STIMESTAMPRESAMPLERNODE_H
#define LXISTREAM_STIMESTAMPRESAMPLERNODE_H

#include <QtCore>
#include <LXiCore>
#include "../saudiobuffer.h"
#include "../sinterfaces.h"
#include "../spixels.h"
#include "../ssubpicturebuffer.h"
#include "../ssubtitlebuffer.h"
#include "../svideobuffer.h"
#include "../export.h"

namespace LXiStream {

class SAudioBuffer;
class SDataBuffer;
class SVideoBuffer;

/*! This filter can be used to resample the timestamps in a stream.
 */
class LXISTREAM_PUBLIC STimeStampResamplerNode : public SInterfaces::Node
{
Q_OBJECT
public:
  explicit                      STimeStampResamplerNode(SGraph *);
  virtual                       ~STimeStampResamplerNode();

  void                          setFrameRate(SInterval frameRate, double maxRatio = 0.08);

  static const QVector<SInterval> & standardFrameRates(void);
  static SInterval              roundFrameRate(SInterval, const QVector<SInterval> &);

public: // From SInterfaces::Node
  virtual bool                  start(void);
  virtual void                  stop(void);

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
  STime                         correctOnly(const STime &) const;

private:
  static const int              numChannels;

  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
