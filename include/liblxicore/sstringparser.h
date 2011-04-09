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
  __pure static bool            isUtf8(const QByteArray &);

  __pure static QString         removeControl(const QString &);
  __pure static QStringList     removeControl(const QStringList &);
  __pure static QString         toBasicLatin(const QString &);
  __pure static QString         toBasicLatin(QChar);
  __pure static QStringList     toBasicLatin(const QStringList &);
  __pure static QString         toCleanName(const QString &);
  __pure static QStringList     toCleanName(const QStringList &);
  __pure static QString         toRawName(const QString &);
  __pure static QStringList     toRawName(const QStringList &);
  __pure static QString         toRawPath(const QString &);

  __pure static QString         findMatch(const QString &, const QString &);
  __pure static qreal           computeMatch(const QString &, const QString &);
  __pure static qreal           computeMatch(const QString &, const QStringList &);
  __pure static qreal           computeBidirMatch(const QString &, const QString &);

  __pure static unsigned        numWords(const QString &);

  __pure static const char    * languageOf(const QString &);
  __pure static QString         iso639Language(const char *);
};

} // End of namespaces

#endif
