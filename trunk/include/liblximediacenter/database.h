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

#ifndef LXMEDIACENTER_DATABASE_H
#define LXMEDIACENTER_DATABASE_H

#include <QtCore>
#include <QtSql>
#include <LXiStream>

namespace LXiMediaCenter {

class Database
{
public:
  /*! This class is used to reduce error handling code, and tests threading in
      debug mode
   */
  class Query : public QSqlQuery
  {
  public:
                                Query(void);
                                Query(QSqlResult *);
                                Query(const QString &);
                                Query(const QSqlQuery &);
                                ~Query();

    Query                     & operator=(const QSqlQuery &other);

    void                        exec(const QString &);
    void                        exec(void);
    void                        execBatch(BatchExecutionMode = ValuesAsRows);
    void                        prepare(const QString &);
  };

public:
  static void                   initialize(void);
  static void                   shutdown(void);

  static SScheduler::Dependency * dependency(void);

  static void                   transaction(void);
  static void                   commit(void);

protected:
  static QSqlDatabase         & database(void) __attribute__((pure));

  static void                   handleError(const ::QSqlQuery &, const QString & = QString::null);

private:
                                Database(void);
                                ~Database();
};
/*
// This wrapper class is used to reduce error handling code.
class QSqlQuery : public ::QSqlQuery
{
public:
  inline                      QSqlQuery(QSqlResult *r) : ::QSqlQuery(r) { }
  inline                      QSqlQuery(const QString& query = QString(), QSqlDatabase db = QSqlDatabase()) : ::QSqlQuery(query, db) { }
  inline explicit             QSqlQuery(const QSqlDatabase &db) : ::QSqlQuery(db) { }
  inline                      QSqlQuery(const ::QSqlQuery& other) : ::QSqlQuery(other) { }
  inline QSqlQuery          & operator=(const ::QSqlQuery& other) { ::QSqlQuery::operator=(other); return *this; }

  void                        exec(const QString &);
  void                        exec(void);
  void                        execBatch(BatchExecutionMode = ValuesAsRows);
  void                        prepare(const QString &);
};*/

} // End of namespace

#endif
