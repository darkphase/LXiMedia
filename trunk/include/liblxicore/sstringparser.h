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
  _lxi_pure static bool         isUtf8(const QByteArray &);

  _lxi_pure static QString      removeControl(const QString &);
  _lxi_pure static QStringList  removeControl(const QStringList &);
  _lxi_pure static QString      toBasicLatin(const QString &);
  _lxi_pure static QString      toBasicLatin(QChar);
  _lxi_pure static QStringList  toBasicLatin(const QStringList &);
  _lxi_pure static QString      toCleanName(const QString &);
  _lxi_pure static QStringList  toCleanName(const QStringList &);
  _lxi_pure static QString      toRawName(const QString &);
  _lxi_pure static QStringList  toRawName(const QStringList &);
  _lxi_pure static QString      toRawPath(const QString &);

  _lxi_pure static QString      findMatch(const QString &, const QString &);
  _lxi_pure static qreal        computeMatch(const QString &, const QString &);
  _lxi_pure static qreal        computeMatch(const QString &, const QStringList &);
  _lxi_pure static qreal        computeBidirMatch(const QString &, const QString &);

  _lxi_pure static unsigned     numWords(const QString &);

  _lxi_pure static const char * languageOf(const QString &);
  _lxi_pure static QString      iso639Language(const char *);
};

} // End of namespaces

#endif
