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
#include "sglobal.h"

namespace LXiCore {

class S_DSO_PUBLIC SStringParser
{
public:
  static bool                   isUtf8(const QByteArray &) __attribute__((pure));

  static QString                removeControl(const QString &) __attribute__((pure));
  static QStringList            removeControl(const QStringList &) __attribute__((pure));
  static QString                toBasicLatin(const QString &) __attribute__((pure));
  static QString                toBasicLatin(QChar) __attribute__((pure));
  static QStringList            toBasicLatin(const QStringList &) __attribute__((pure));
  static QString                toCleanName(const QString &) __attribute__((pure));
  static QStringList            toCleanName(const QStringList &) __attribute__((pure));
  static QString                toRawName(const QString &) __attribute__((pure));
  static QStringList            toRawName(const QStringList &) __attribute__((pure));
  static QString                toRawPath(const QString &) __attribute__((pure));

  static QString                findMatch(const QString &, const QString &) __attribute__((pure));
  static qreal                  computeMatch(const QString &, const QString &) __attribute__((pure));
  static qreal                  computeMatch(const QString &, const QStringList &) __attribute__((pure));
  static qreal                  computeBidirMatch(const QString &, const QString &) __attribute__((pure));

  static unsigned               numWords(const QString &) __attribute__((pure));

  static const char           * languageOf(const QString &) __attribute__((pure));
  static QString                iso639Language(const char *) __attribute__((pure));
};

} // End of namespaces

#endif
