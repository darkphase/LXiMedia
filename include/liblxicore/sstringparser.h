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

#ifndef LXICORE_SSTRINGPARSER_H
#define LXICORE_SSTRINGPARSER_H

#include <QtCore>
#include "splatform.h"
#include "export.h"

namespace LXiCore {

/*! This class provides several basig string operations.
 */
class LXICORE_PUBLIC SStringParser
{
public:
  static bool                   isUtf8(const QByteArray &);

  static QString                removeControl(const QString &);
  static QStringList            removeControl(const QStringList &);
  static QString                toBasicLatin(const QString &);
  static QString                toBasicLatin(QChar);
  static QStringList            toBasicLatin(const QStringList &);
  static QString                toCleanName(const QString &);
  static QStringList            toCleanName(const QStringList &);
  static QString                toRawName(const QString &);
  static QStringList            toRawName(const QStringList &);
  static QString                toRawPath(const QString &);

  static QString                findMatch(const QString &, const QString &);
  static qreal                  computeMatch(const QString &, const QString &);
  static qreal                  computeMatch(const QString &, const QStringList &);
  static qreal                  computeBidirMatch(const QString &, const QString &);

  static unsigned               numWords(const QString &);

  static const char           * languageOf(const QString &);
  static QString                iso639Language(const char *);
};

} // End of namespaces

#endif
