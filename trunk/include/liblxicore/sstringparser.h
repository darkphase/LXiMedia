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

class LXICORE_PUBLIC SStringParser
{
public:
  pure static bool              isUtf8(const QByteArray &);

  pure static QString           removeControl(const QString &);
  pure static QStringList       removeControl(const QStringList &);
  pure static QString           toBasicLatin(const QString &);
  pure static QString           toBasicLatin(QChar);
  pure static QStringList       toBasicLatin(const QStringList &);
  pure static QString           toCleanName(const QString &);
  pure static QStringList       toCleanName(const QStringList &);
  pure static QString           toRawName(const QString &);
  pure static QStringList       toRawName(const QStringList &);
  pure static QString           toRawPath(const QString &);

  pure static QString           findMatch(const QString &, const QString &);
  pure static qreal             computeMatch(const QString &, const QString &);
  pure static qreal             computeMatch(const QString &, const QStringList &);
  pure static qreal             computeBidirMatch(const QString &, const QString &);

  pure static unsigned          numWords(const QString &);

  pure static const char      * languageOf(const QString &);
  pure static QString           iso639Language(const char *);
};

} // End of namespaces

#endif
