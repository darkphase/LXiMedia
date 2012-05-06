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
  virtual void                  readFormat(ProbeInfo &, const QByteArray &);
  virtual void                  readContent(ProbeInfo &, QIODevice *);

public:
  static void                   splitFileName(QString, QString &, QString &, QString &, int &);
  static QString                toGenre(const QString &);

  static QString                audioDescription(const QString &suffix);
  static QString                videoDescription(const QString &suffix);
  static QString                imageDescription(const QString &suffix);
  static QString                subtitleDescription(const QString &suffix);

  static const QSet<QString>  & audioSuffixes(void);
  static const QSet<QString>  & videoSuffixes(void);
  static const QSet<QString>  & imageSuffixes(void);
  static const QSet<QString>  & subtitleSuffixes(void);
};


} } // End of namespaces

#endif
