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

#ifndef SITEDATABASE_H
#define SITEDATABASE_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>

namespace LXiMediaCenter {
namespace InternetBackend {

class SiteDatabase : public QObject
{
Q_OBJECT
public:
  enum Category
  {
    Category_None             = 0,
    Category_Radio            = 1,
    Category_Television       = 2,
    Category_Sentinel
  };

public:
  static SiteDatabase         * createInstance(void);
  static void                   destroyInstance(void);

private:
  explicit                      SiteDatabase(QObject *parent = NULL);
  virtual                       ~SiteDatabase();

public:
  void                          setSite(const QString &hostname, const QString &countries, Category, const QString &script);
  bool                          getSite(const QString &hostname, QString &countries, Category &, QString &script);

  QStringList                   allCountries(void);
  int                           countSites(const QString &country);
  QStringList                   getSites(const QString &country, unsigned start = 0, unsigned count = 0);
  int                           countSites(Category);
  QStringList                   getSites(Category, unsigned start = 0, unsigned count = 0);

private:
  static SiteDatabase         * self;
};

} } // End of namespaces

#endif
