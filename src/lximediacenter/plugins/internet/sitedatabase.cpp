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

#include "sitedatabase.h"
#include "module.h"

namespace LXiMediaCenter {
namespace InternetBackend {

SiteDatabase * SiteDatabase::self = NULL;

SiteDatabase * SiteDatabase::createInstance(void)
{
  if (self)
    return self;
  else
    return self = new SiteDatabase();
}

void SiteDatabase::destroyInstance(void)
{
  delete self;
}

SiteDatabase::SiteDatabase(QObject *parent)
  : QObject(parent)
{
  PluginSettings settings(Module::pluginName);

  Database::Query query;

  // Drop all tables if the database version is outdated.
  static const int databaseVersion = 1;
  if (settings.value("DatabaseVersion", 0).toInt() != databaseVersion)
  {
    qDebug() << "Internet site database layout changed, recreating tables.";

    QStringList tables;
    query.exec("SELECT name FROM sqlite_master WHERE type='table'");
    while (query.next())
    {
      const QString name = query.value(0).toString();
      if (name.startsWith("Internet"))
        tables += name;
    }

    QStringList indices;
    query.exec("SELECT name FROM sqlite_master WHERE type='index'");
    while (query.next())
    {
      const QString name = query.value(0).toString();
      if (name.startsWith("Internet"))
        indices += name;
    }

    query.exec("PRAGMA foreign_keys = OFF");

    Database::transaction();

    foreach (const QString &name, indices)
      query.exec("DROP INDEX IF EXISTS " + name);

    foreach (const QString &name, tables)
      query.exec("DROP TABLE IF EXISTS " + name);

    Database::commit();

    query.exec("PRAGMA foreign_keys = ON");

    query.exec("VACUUM");
  }

  // Create tables that don't exist
  query.exec("CREATE TABLE IF NOT EXISTS InternetSites ("
             "name           TEXT NOT NULL,"
             "countries      TEXT NOT NULL,"
             "category       INTEGER NOT NULL,"
             "script         TEXT)");

  query.exec("CREATE INDEX IF NOT EXISTS InternetSites_countries "
             "ON InternetSites(countries)");

  query.exec("CREATE INDEX IF NOT EXISTS InternetSites_category "
             "ON InternetSites(category)");

  settings.setValue("DatabaseVersion", databaseVersion);
}

SiteDatabase::~SiteDatabase()
{
  self = NULL;
}

QStringList SiteDatabase::allCountries(Category category)
{
  QSet<QString> result;

  Database::Query query;
  query.prepare("SELECT countries FROM InternetSites "
                "WHERE category = :category");
  query.bindValue(0, category);
  query.exec();
  while (query.next())
  foreach (const QString &country, query.value(0).toString().simplified().split(' '))
    result.insert(country.toUpper());

  return result.toList();
}

} } // End of namespaces
