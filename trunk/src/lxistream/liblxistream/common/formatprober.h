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

#ifndef LXSTREAMCOMMON_FORMATPROBER_H
#define LXSTREAMCOMMON_FORMATPROBER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {


class FormatProber : public SInterfaces::FormatProber
{
Q_OBJECT
public:
                                FormatProber(const QString &, QObject *);
  virtual                       ~FormatProber();

public: // From SInterfaces::FormatProber
  virtual QList<Format>         probeFileFormat(const QByteArray &, const QString &);
  virtual QList<Format>         probeDiscFormat(const QString &);
  virtual void                  probeFile(ProbeInfo &, ReadCallback *);
  virtual void                  probeDisc(ProbeInfo &, const QString &);

public:
  static void                   splitFileName(QString, QString &, QString &, QString &, unsigned &);
  static QString                toGenre(const QString &);

  static QString                audioDescription(const QString &suffix);
  static QString                videoDescription(const QString &suffix);
  static QString                imageDescription(const QString &suffix);

  static const QSet<QString>  & audioSuffixes(void);
  static const QSet<QString>  & videoSuffixes(void);
  static const QSet<QString>  & imageSuffixes(void);
  static const QSet<QString>  & rawImageSuffixes(void);
};


} } // End of namespaces

#endif
