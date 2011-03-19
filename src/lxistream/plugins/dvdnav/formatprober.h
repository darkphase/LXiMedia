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
  virtual QList<Format>         probeFormat(const QByteArray &, const QString &);
  virtual void                  probeMetadata(ProbeInfo &, ReadCallback *);
};


} } // End of namespaces

#endif
