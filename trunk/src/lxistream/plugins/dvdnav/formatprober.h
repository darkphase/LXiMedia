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

#ifndef __FORMATPROBER_H
#define __FORMATPROBER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace DVDNavBackend {

class FormatProber : public SInterfaces::FormatProber
{
Q_OBJECT
public:
                                FormatProber(const QString &, QObject *);
  virtual                       ~FormatProber();

public: // From SInterfaces::FormatProber
  virtual void                  readFormat(ProbeInfo &probeInfo, const QByteArray &buffer);
  virtual void                  readContent(ProbeInfo &, QIODevice *);
  virtual SVideoBuffer          readThumbnail(const ProbeInfo &, QIODevice *, const QSize &);
};


} } // End of namespaces

#endif
