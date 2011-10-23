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

#ifndef LXSTREAMCOMMON_DEINTERLACE_H
#define LXSTREAMCOMMON_DEINTERLACE_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {

class DeinterlaceBlend : public SInterfaces::VideoDeinterlacer
{
Q_OBJECT
public:
  inline explicit               DeinterlaceBlend(const QString &, QObject *parent) : SInterfaces::VideoDeinterlacer(parent) { }

public: // From SInterfaces::VideoDeinterlacer
  virtual SVideoBufferList      processBuffer(const SVideoBuffer &);

private:
  void                          copyLines(SVideoBuffer &destBuffer, const SVideoBuffer &videoBuffer, int plane);
};


class DeinterlaceBob : public SInterfaces::VideoDeinterlacer
{
Q_OBJECT
public:
  inline explicit               DeinterlaceBob(const QString &, QObject *parent) : SInterfaces::VideoDeinterlacer(parent) { }

public: // From SInterfaces::VideoDeinterlacer
  virtual SVideoBufferList      processBuffer(const SVideoBuffer &);

private:
  void                          copyLines(SVideoBuffer &destBuffer, const SVideoBuffer &videoBuffer, int offset, int plane);

private:
  STime                         avgFrameTime;
  STime                         lastTimeStamp;
};

} } // End of namespaces

#endif
