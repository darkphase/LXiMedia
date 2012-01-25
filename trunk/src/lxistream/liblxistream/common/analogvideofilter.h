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

#ifndef LXSTREAMCOMMON_ANALOGVIDEOFILTER_H
#define LXSTREAMCOMMON_ANALOGVIDEOFILTER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {


class AnalogVideoFilter : public SNodes::Video::AnalogVideoFilter
{
Q_OBJECT
public:
                                AnalogVideoFilter(QObject *);

public: // From SNode
  virtual bool                  prepare(const SCodecList &);
  virtual bool                  unprepare(void);
  virtual Result                processBuffer(const SBuffer &, SBufferList &);

private:
  SVideoBuffer                  tempBuffer;
  quint8                        minVal, maxVal;
  int                           count;
};


} } // End of namespaces

#endif
