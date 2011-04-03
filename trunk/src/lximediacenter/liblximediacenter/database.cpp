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

#include "database.h"
#include "globalsettings.h"

namespace LXiMediaCenter {

void Database::initialize(void)
{
  SScheduler::DependencyLocker l(dependency());

  database() = QSqlDatabase::addDatabase("QSQLITE", "LXiMediaCenter");
  database().setDatabaseName(GlobalSettings::databaseFile());
  if (!database().open())
  {
    qCritical() << "Failed to open SQLite database" << GlobalSettings::databaseFile();

    const QString corruptFile = GlobalSettings::databaseFile() + ".corrupt";
    QFile::remove(corruptFile);
    if (!QFile::rename(GlobalSettings::databaseFile(), corruptFile))
      return;

    qDebug() << "Creating new database, renamed corrupt file to" << corruptFile;
    database() = QSqlDatabase::addDatabase("QSQLITE", "LXiMediaCenter");
    database().setDatabaseName(GlobalSettings::databaseFile());
    if (!database().open())
      return;
  }

  ::QSqlQuery query(database());
  if (!query.exec("PRAGMA foreign_keys = ON"))
  {
    qWarning() << "Failed to enable foreign key constraints on database"
               << (GlobalSettings::databaseFile());
  }
}

void Database::shutdown(void)
{
  SScheduler::DependencyLocker l(dependency());

  database().close();
  database() = QSqlDatabase();
  QSqlDatabase::removeDatabase("LXiMediaCenter");
}

SScheduler::Dependency * Database::dependency(void)
{
  static SScheduler::Dependency d(sApp);

  return &d;
}

void Database::transaction(void)
{
  Q_ASSERT(!dependency()->tryLock());

  database().transaction();
}

void Database::commit(void)
{
  Q_ASSERT(!dependency()->tryLock());

  database().commit();
}

QSqlDatabase & Database::database(void)
{
  static QSqlDatabase d;

  return d;
}

void Database::handleError(const ::QSqlQuery &query, const QString &q)
{
  const QString error = query.lastError().text();
  if (error.contains("database disk image is malformed", Qt::CaseInsensitive) ||
      error.contains("unsupported file format", Qt::CaseInsensitive))
  { // Big trouble here ...
    qCritical() << "Fatal database error" << error << "attempting to recover.";

    database().close();
    database() = QSqlDatabase();
    QSqlDatabase::removeDatabase("LXiMediaCenter");

    const QString corruptFile = GlobalSettings::databaseFile() + ".corrupt";
    QFile::remove(corruptFile);
    if (QFile::rename(GlobalSettings::databaseFile(), corruptFile))
      qFatal("SQL error: %s. Renamed corrupt database file to %s, database will be recreated at restart.", error.toAscii().data(), corruptFile.toAscii().data());
  }

  if (q.isEmpty())
    qFatal("SQL error: %s", error.toAscii().data());
  else
    qFatal("SQL error on exec of \"%s\": %s", q.toAscii().data(), error.toAscii().data());
}


Database::Query::Query(void)
  : QSqlQuery(database())
{
  Q_ASSERT(!dependency()->tryLock());
}

Database::Query::Query(QSqlResult *r)
  : QSqlQuery(r)
{
  Q_ASSERT(!dependency()->tryLock());
}

Database::Query::Query(const QString &query)
  : QSqlQuery(query, database())
{
  Q_ASSERT(!dependency()->tryLock());
}

Database::Query::Query(const QSqlQuery &other)
  : QSqlQuery(other)
{
  Q_ASSERT(!dependency()->tryLock());
}

Database::Query & Database::Query::operator=(const ::QSqlQuery &other)
{
  Q_ASSERT(!dependency()->tryLock());

  QSqlQuery::operator=(other);

  return *this;
}

Database::Query::~Query()
{
  Q_ASSERT(!dependency()->tryLock());
}

void Database::Query::exec(const QString &q)
{
  Q_ASSERT(!dependency()->tryLock());

  if (!QSqlQuery::exec(q))
    Database::handleError(*this, q);
}

void Database::Query::exec(void)
{
  Q_ASSERT(!dependency()->tryLock());

  if (!QSqlQuery::exec())
    Database::handleError(*this);
}

void Database::Query::execBatch(BatchExecutionMode mode)
{
  Q_ASSERT(!dependency()->tryLock());

  if (!QSqlQuery::execBatch(mode))
    Database::handleError(*this);
}

void Database::Query::prepare(const QString &q)
{
  Q_ASSERT(!dependency()->tryLock());

  if (!QSqlQuery::prepare(q))
    Database::handleError(*this, q);
}

} // End of namespace
