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

#ifndef LXISTREAM_SDISCINFO_H
#define LXISTREAM_SDISCINFO_H

#include <QtCore>
#include <QtXml>
#include "sinterfaces.h"
#include "smediainfo.h"

namespace LXiStream {

class SDiscInfo : public SSerializable
{
public:
  inline                        SDiscInfo(void) : path(), pi()                  { }
  inline explicit               SDiscInfo(const QString &path) : path(path), pi() { probe(); }
  inline                        SDiscInfo(const SDiscInfo &c) : path(c.path), pi(c.pi) { }
  inline explicit               SDiscInfo(const SInterfaces::DiscFormatProber::ProbeInfo &pi) : path(), pi(pi) { }

  virtual QDomNode              toXml(QDomDocument &) const;
  virtual void                  fromXml(const QDomNode &);

  inline QString                format(void) const                              { return pi.format; }
  SMediaInfoList                titles(void) const;

private:
  void                          probe(void);

private:
  QString                       path;
  SInterfaces::DiscFormatProber::ProbeInfo pi;
};


} // End of namespace

#endif
