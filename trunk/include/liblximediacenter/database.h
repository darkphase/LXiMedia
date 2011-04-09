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
#include "export.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC Database
{
public:
  /*! This class is used to reduce error handling code, and tests threading in
      debug mode
   */
  class LXIMEDIACENTER_PUBLIC Query : public QSqlQuery
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

  static void                   transaction(void);
  static void                   commit(void);

private:
  pure internal static QMutex & mutex(void);
  pure internal static QSqlDatabase & database(void);

  internal static void          handleError(const ::QSqlQuery &, const QString & = QString::null);

private:
  internal                      Database(void);
  internal                      ~Database();
};

} // End of namespace

#endif
